/*
 * Copyright (C) 2013 LAN Xingcan
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include "spin.h"

struct __spin_timer {
    struct __spin_task task;
    int (*callback) (void *);
    void *context;
};

#define CAST_TASK_TO_TIMER(x) \
    SPIN_DEFINE_DOWNCAST(struct __spin_timer, task, x)

static int spin_timer_task_callback (spin_task_t task);
static spin_timer_t spin_timer_alloc (int (*callback)(void *), void *context);
static void spin_timer_free (spin_timer_t task);

static void *spin_poll_thread (void *param);
static void wait_for_task (spin_loop_t xp);

static struct timespec startup_timespec;
static pthread_once_t startup_timespec_once = PTHREAD_ONCE_INIT;

static void startup_timespec_init (void)
{
    clock_gettime (CLOCK_MONOTONIC, &startup_timespec);
}

static inline spin_timer_t spin_timer_alloc (int (*callback)(void *),
                                             void *context)
{
    spin_timer_t ret;

    if (callback == NULL) {
        errno = EINVAL;
        return NULL;
    }

    ret = (spin_timer_t) malloc (sizeof (*ret));

    if (ret == NULL)
        return NULL;

    spin_task_init (&ret->task, spin_timer_task_callback);
    ret->callback = callback;
    ret->context = context;
    return ret;
}

static inline void spin_timer_free (spin_timer_t t)
{
    free (t);
}

static int spin_timer_task_callback (spin_task_t task)
{
    spin_timer_t timer = CAST_TASK_TO_TIMER (task);
    timer->callback(timer->context);
    /* XXX: clean up in else where ?*/
    spin_timer_free (timer);
    return 0;
}

/**
 * Create an spin object
 * return NULL if exited, and then you may want to see errno
 */
spin_loop_t spin_loop_create (void)
{
    spin_loop_t ret;
    struct epoll_event event;
    int tmp;

    pthread_once (&startup_timespec_once, startup_timespec_init);
    pthread_condattr_t attr;
    pthread_condattr_init (&attr);
    pthread_condattr_setclock (&attr, CLOCK_MONOTONIC);

    ret = (spin_loop_t) calloc (1, sizeof(*ret));
    pthread_mutex_init (&ret->lock, NULL);
    pthread_cond_init (&ret->guard, &attr);

    if (ret == NULL)
        return NULL;

    ret->epollfd = epoll_create1 (O_CLOEXEC);

    if (ret->epollfd == -1) {
        free (ret);
        return NULL;
    }

    tmp = pipe2 (ret->dummy_pipe, O_CLOEXEC | O_NONBLOCK);

    if (tmp == -1)
        goto clean_and_exit;

    event.events = EPOLLIN | EPOLLET;
    event.data.ptr = NULL;

    tmp = epoll_ctl (ret->epollfd, EPOLL_CTL_ADD, ret->dummy_pipe[0],
                     &event);
    if (tmp == -1)
        goto clean_and_exit;

    ret->prioque = prioque_create();
    if (ret->prioque == NULL)
        goto clean_and_exit;

    tmp = pthread_create (&ret->poll_thread, NULL, spin_poll_thread, ret);

    if (tmp != 0) {
        close (ret->epollfd);
        free (ret);
        return NULL;
    }

    link_list_init (&ret->currtask);
    link_list_init (&ret->nexttask);
    link_list_init (&ret->bgtask);

    /* since SPIN_LOOP_PREPARE is exactly 0, we don't need to set it */

    return ret;
clean_and_exit:

    if (ret != NULL) {
        if (ret->epollfd > 0)
            close(ret->epollfd);

        if (ret->prioque != NULL)
            prioque_destroy (ret->prioque);

        if (ret->dummy_pipe[0] > 0)
            close (ret->dummy_pipe[0]);

        if (ret->dummy_pipe[1] > 0)
            close (ret->dummy_pipe[1]);
        pthread_cond_destroy (&ret->guard);
        pthread_mutex_destroy (&ret->lock);
        free (ret);
    }

    return NULL;
}

/**
 * Destroy an spin object
 * you should not close an spin object within event loop!
 * the control flow of the program will block on spin_loop_run if the are any
 * event to be fired. you should only close an spin object before or after
 * calling spin_loop_run
 */
void spin_loop_destroy (spin_loop_t loop)
{
    assert (loop != NULL);

    /* Close the dummy_pipe[1] and the poller thread will an EPOLLIN evnet
     * for the dummy_pipe[0], so the poller thread know when to exit */

    close (loop->dummy_pipe[1]);
    pthread_join (loop->poll_thread, NULL);

    pthread_mutex_destroy (&loop->lock);
    pthread_cond_destroy (&loop->guard);

    close (loop->epollfd);
    close (loop->dummy_pipe[0]);
    close (loop->dummy_pipe[1]);
    prioque_destroy (loop->prioque);
    free (loop);
}

void wait_for_task (spin_loop_t loop)
{
    int ret = 0;

    prioque_node_t *task_node;
    prioque_front (loop->prioque, &task_node);

    if (task_node == NULL) {
        pthread_mutex_lock (&loop->lock);
        if (link_list_is_empty (&loop->currtask))
            pthread_cond_wait (&loop->guard, &loop->lock);
        link_list_cat (&loop->currtask, &loop->bgtask);
        pthread_mutex_unlock (&loop->lock);
    } else {
        struct timespec ts = startup_timespec;
        struct timespec now;
        prioque_weight_t msecs;
        prioque_get_node_weight (loop->prioque, task_node, &msecs);

        timespec_now (&now);
        timespec_add_milliseconds (&ts, msecs);

        if (link_list_is_empty (&loop->currtask)) {
            pthread_mutex_lock (&loop->lock);
            ret = pthread_cond_timedwait (&loop->guard, &loop->lock, &ts);
            if (ret == 0)
                link_list_cat (&loop->currtask, &loop->bgtask);
            pthread_mutex_unlock (&loop->lock);

            if (ret == ETIMEDOUT) {
                spin_task_t task = CAST_PRIOQUE_NODE_TO_TASK (task_node);
                prioque_remove (loop->prioque, task_node);
                link_list_attach_to_tail (&loop->currtask, &task->node.l);
            }
        } else {
            pthread_mutex_lock (&loop->lock);
            link_list_cat (&loop->currtask, &loop->bgtask);
            pthread_mutex_unlock (&loop->lock);
        }
    }
}


int spin_loop_run (spin_loop_t loop)
{
    if (loop == NULL) {
        errno = EINVAL;
        return -1;
    }
    do {
        link_list_cat (&loop->currtask, &loop->nexttask);
        wait_for_task (loop);

        while (!link_list_is_empty(&loop->currtask)) {
            link_node_t *node = loop->currtask.head;
            link_list_dettach (&loop->currtask, node);
            spin_task_t task = CAST_LINK_NODE_TO_TASK(node);
            task->callback(task);
        }
    } while (1); /* test if there are event to be fired */
    return 0;
}

void *spin_poll_thread (void *param)
{
    /* TODO: Mask all signals */
    spin_loop_t loop = (spin_loop_t) param;
    int looping = 1;
    struct link_list_t event_list;
    link_list_init (&event_list);

    while (looping) {
        struct epoll_event event[256];
        int i;
        int ret = epoll_wait (loop->epollfd, event, 256, -1);

        /* XXX: Handle error */
        if (ret <= 0)
            continue;

        for (i = 0; i < ret; i++) {
            spin_task_t task = (spin_task_t) event[i].data.ptr;
            if (task != NULL) {
                link_list_attach_to_tail (&event_list, &task->node.l);
            } else {
                char ch;
                int ret = read (loop->dummy_pipe[0], &ch, sizeof(ch));
                if (ret == 0)
                    /* The dummy_pipe[1] is closed, indicate that this thread
                     * should go exit */
                    looping = 0;
            }
        }

        pthread_mutex_lock (&loop->lock);
        if (link_list_is_empty (&loop->bgtask))
            pthread_cond_signal (&loop->guard);
        link_list_cat (&loop->bgtask, &event_list);
        pthread_mutex_unlock (&loop->lock);
    }

    return NULL;
}

spin_timer_t spin_timer_create (spin_loop_t loop, unsigned msecs,
                              int (*callback) (void*), void *args)
{
    /* TODO: Error checking and free memeory in some where */
    struct timespec ts;
    spin_timer_t t = spin_timer_alloc (callback, args);
    if (t == NULL)
        return NULL;

    prioque_weight_t x = msecs;

    if (x == 0) {
        link_list_attach_to_tail(&loop->nexttask, &t->task.node.l);
    } else {
        timespec_now (&ts);

        x += timespec_diff_millisecons (&ts, &startup_timespec);

        if (prioque_insert(loop->prioque, &t->task.node.q, x) != 0) {
            spin_timer_free (t);
            return NULL;
        }
    }

    return t;
}

int spin_timer_destroy (spin_timer_t timer)
{
    /* FIXME: remove from loop */
    if (timer == NULL) {
        errno = EINVAL;
        return -1;
    }

    spin_timer_free (timer);
    return 0;
}

int spin_timer_pause (spin_timer_t timer)
{
    /* TODO */
    errno = ENOSYS;
    return -1;
}

int spin_timer_resume (spin_timer_t timer)
{
    /* TODO */
    errno = ENOSYS;
    return -1;
}

