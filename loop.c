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

#include "spin/loop.h"
#include "linklist.h"
#include "prioque.h"
#include "timespec.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>

enum {
    SPIN_LOOP_PREPARE = 0,
    SPIN_LOOP_RUN,
    SPIN_LOOP_EXIT
};

struct __spin_loop {
    pthread_t poll_thread;
    pthread_mutex_t lock;
    pthread_cond_t guard;

    /* task and bgtask may used across thread */
    int state;
    int epollfd;
    link_list_t bgtask;

    /* the following member should only used in running thread */
    link_list_t currtask;
    link_list_t nexttask;
    prioque_t prioque;
};

struct __spin_task {
    union {
        prioque_node_t q;
        link_node_t l;
    } node;
    int (*callback) (void*);
    void *args;
};

#define CAST_PRIOQUE_NODE_TO_TASK(x) \
    ((spin_task_t)((int8_t *)(x) - offsetof(struct __spin_task, node.q)))

#define CAST_LINK_NODE_TO_TASK(x) \
    ((spin_task_t)((int8_t *)(x) - offsetof(struct __spin_task, node.l)))

static spin_task_t spin_task_alloc (int (*callback)(void *), void *args);
static void spin_task_free (spin_task_t task);

static void *spin_poll_thread (void *param);
static int wait_for_tasktimeout (spin_loop_t xp, spin_task_t *task);

static struct timespec startup_timespec;
static pthread_once_t startup_timespec_once = PTHREAD_ONCE_INIT;

static void startup_timespec_init (void)
{
    clock_gettime (CLOCK_MONOTONIC, &startup_timespec);
}

static inline spin_task_t spin_task_alloc (int (*callback)(void *), void *args)
{
    spin_task_t ret;

    if (callback == NULL) {
        errno = EINVAL;
        return NULL;
    }

    ret = (spin_task_t) malloc (sizeof (*ret));

    if (ret == NULL)
        return NULL;

    ret->callback = callback;
    ret->args = args;
}

static inline void spin_task_free (spin_task_t task)
{
    free (task);
}


/**
 * Create an spin object
 * return NULL if exited, and then you may want to see errno
 */
spin_loop_t spin_loop_create (void)
{
    spin_loop_t ret;
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

    ret->prioque = prioque_create();
    if (ret->prioque == NULL)
        goto clean_and_exit;

    pthread_create (&ret->poll_thread, NULL, spin_poll_thread, ret);

    if (tmp != 0) {
        close (ret->epollfd);
        free (ret);
        return NULL;
    }

    /* since SPIN_LOOP_PREPARE is exactly 0, we don't need to set it */

    return ret;
clean_and_exit:

    if (ret != NULL) {
        if (ret->epollfd > 0)
            close(ret->epollfd);

        if (ret->prioque != NULL)
            prioque_destroy (ret->prioque);

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
void spin_loop_destroy (spin_loop_t xp)
{
    assert (xp != NULL);
    pthread_mutex_lock (&xp->lock);
    xp->state = SPIN_LOOP_EXIT;
    pthread_cond_signal (&xp->guard);
    pthread_mutex_unlock (&xp->lock);
    pthread_join (xp->poll_thread, NULL);

    pthread_mutex_destroy (&xp->lock);
    pthread_cond_destroy (&xp->guard);

    close (xp->epollfd);
    prioque_destroy (xp->prioque);
    free (xp);
}

/**
 * Wait for task event, return 0 indicate a task expired. otherwise
 * interrupted by I/O event
 */
int wait_for_tasktimeout (spin_loop_t xp, spin_task_t *task)
{
    int timedout = 0;
    int event_queue_not_empty = 0;

    prioque_node_t *task_node;
    prioque_front (xp->prioque, &task_node);

    if (task_node == NULL) {
        pthread_mutex_lock (&xp->lock);
        pthread_cond_wait (&xp->guard, &xp->lock);
        pthread_mutex_unlock (&xp->lock);
        return -1;
    } else {
        struct timespec ts = startup_timespec;
        int ret;
        prioque_weight_t msecs, secs;
        prioque_get_node_weight (xp->prioque, task_node, &msecs);

        timespec_add_milliseconds (&ts, &msecs);

        pthread_mutex_lock (&xp->lock);
        ret = pthread_cond_timedwait (&xp->guard, &xp->lock, &ts);
        pthread_mutex_unlock (&xp->lock);

        if (ret == ETIMEDOUT) {
            *task = CAST_PRIOQUE_NODE_TO_TASK (task_node);
            prioque_remove (xp->prioque, task_node);
            return 0;
        }
        return -1;
    }
}


int spin_loop_run (spin_loop_t xp)
{
    pthread_mutex_lock (&xp->lock);
    if (xp->state != SPIN_LOOP_RUN) {
        pthread_cond_signal (&xp->guard);
        xp->state = SPIN_LOOP_RUN;
    }
    pthread_mutex_unlock (&xp->lock);

    do {

        /* TODO:
         *
         * bool timedout = false;
         * if have task event
         *     timedout = pthread_cond_timedwait until latest task event
         * else
         *     pthread_cond_wait
         *
         * if timedout
         *     add task event to event list
         * else
         *     peek targets from io event list
         *
         * while true
         *     fire event for items in event list respectively
         *     flip immediate event
         *     fire current imediate event
         *     check timedout task
         *     fire task event
         *     if event list empty
         *         break loop
         * */

        spin_task_t task;
        if (wait_for_tasktimeout(xp, &task) == 0) {
            task->callback(task->args);
            spin_task_destroy(task);
        } else {

        }

    } while (1); /* test if there are event to be fired */

    pthread_mutex_unlock (&xp->lock);
}

void *spin_poll_thread (void *param)
{
    /*
     * do preparation
     *
     * pthread_mutex_lock
     * while state != XPOLL_RUN
     *     pthread_cond_wait
     * pthread_mutex_unlock
     *
     * :sh
     */
}

spin_task_t spin_task_create (spin_loop_t loop, unsigned msecs,
                                int (*callback) (void*), void *args)
{
    /* TODO: Error checking and free memeory in some where */
    struct timespec ts;
    spin_task_t task = spin_task_alloc (callback, args);
    if (task == NULL)
        return NULL;

    prioque_weight_t x = msecs;
    timespec_now (&ts);

    x += timespec_diff_millisecons (&ts, &startup_timespec);

    if (prioque_insert(loop->prioque, &task->node.q, x) != 0) {
        spin_task_free (task);
        return NULL;
    }

    return task;
}

int spin_task_destroy (spin_task_t task)
{
    /* FIXME: remove from loop */
    if (task == NULL) {
        errno = EINVAL;
        return -1;
    }

    spin_task_free (task);
}
