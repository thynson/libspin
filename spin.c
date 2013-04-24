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


struct spin_poller_t spin_poller = {
    0, /* epollfd */
    { 0, 0 }, /* pipe */
    PTHREAD_MUTEX_INITIALIZER /* lock */
    /* Remains will be initialized in spin_init_once */
};

void *spin_poll_thread (void *param)
{
    /* TODO: Mask all signals */
    int looping = 1;

    while (looping) {
        /* We don't need a large array as edge-trigger mode epoll cut down the
         * number of event */
        enum { EVENT_ARRAY_SIZE = 128 };
        int i, ret;
        struct epoll_event event [EVENT_ARRAY_SIZE];

        ret = epoll_wait (spin_poller.epollfd, event, EVENT_ARRAY_SIZE, -1);

        /* XXX: Handle error */
        if (ret <= 0)
            continue;

        pthread_mutex_lock (&spin_poller.lock);
        for (i = 0; i < ret; i++) {
            spin_poll_target_t t = (spin_poll_target_t) event[i].data.ptr;
            if (t != NULL) {
                pthread_spin_lock (&t->lock);
                event[i].events &= ~(t->cached_events | t->notified_events);
                if (event[i].events != 0) {
                    if (t->notified_events != 0)
                        t->notified_events |= event[i].events;
                    else {
                        t->notified_events |= event[i].events;
                        link_list_attach_to_tail (&t->loop->bgtask,
                                                  &t->task.l);
                    }
                }
                pthread_spin_unlock (&t->lock);
            } else {
                char ch;
                int ret = read (spin_poller.pipe[0], &ch, sizeof(ch));
                if (ret == 0)
                    /* The dummy_pipe[1] is closed, indicate that this thread
                     * should go exit */
                    looping = 0;
            }
        }
        pthread_cond_broadcast (&spin_poller.cond);
        pthread_mutex_unlock (&spin_poller.lock);
    }

    return NULL;
}

int spin_init (void)
{
    int ret;
    pthread_condattr_t condattr;
    struct epoll_event event;

    if (spin_poller.epollfd > 0)
        return 1;

    ret = pthread_condattr_init (&condattr);
    if (ret != 0)
        goto error_clean_up;
    ret = pthread_condattr_setclock (&condattr, CLOCK_MONOTONIC);
    if (ret != 0)
        goto error_clean_up;
    ret = pthread_cond_init (&spin_poller.cond, &condattr);
    if (ret != 0)
        goto error_clean_up;
    ret = clock_gettime (CLOCK_MONOTONIC, &spin_poller.basetime);
    if (ret != 0)
        goto error_clean_up;

    ret = epoll_create1 (O_CLOEXEC);
    if (ret == -1)
        goto error_clean_up;
    spin_poller.epollfd = ret;

    ret = pipe2 (spin_poller.pipe, O_CLOEXEC | O_NONBLOCK);
    if (ret == -1)
        goto error_clean_up;

    event.events = EPOLLIN | EPOLLET;
    event.data.ptr = NULL;

    ret = epoll_ctl (spin_poller.epollfd, EPOLL_CTL_ADD,
                     spin_poller.pipe[0], &event);

    if (ret == -1)
        goto error_clean_up;

    ret = pthread_create (&spin_poller.thread, NULL, spin_poll_thread, NULL);
    if (ret == -1)
        goto error_clean_up;
    return 0;
error_clean_up:
    if (spin_poller.epollfd != 0) {
        close (spin_poller.epollfd);
        spin_poller.epollfd = 0;
    }

    if (spin_poller.pipe[0] != 0) {
        close (spin_poller.pipe[0]);
        spin_poller.pipe[0] = 0;
    }

    if (spin_poller.pipe[1] != 0) {
        close (spin_poller.pipe[1]);
        spin_poller.pipe[1] = 0;
    }
    return -1;
}

int spin_uninit (void)
{
    /* Close the write end so epoll will be notified */
    close (spin_poller.pipe[1]);
    pthread_join (spin_poller.thread, NULL);
    pthread_cond_destroy (&spin_poller.cond);
    close (spin_poller.pipe[0]);
    close (spin_poller.epollfd);
    return 0;
}
