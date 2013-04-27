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
#ifndef NDEBUG
#include <stdarg.h>
#include <string.h>
#endif


struct spin_global_t spin_global = {
#ifndef NDEBUG
    NULL,
#endif
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

        ret = epoll_wait (spin_global.epollfd, event, EVENT_ARRAY_SIZE, -1);

        /* XXX: Handle error */
        if (ret <= 0)
            continue;

        pthread_mutex_lock (&spin_global.lock);
        for (i = 0; i < ret; i++) {
            spin_poll_target_t t = (spin_poll_target_t) event[i].data.ptr;
            if (t != NULL) {
                pthread_spin_lock (&t->lock);
                event[i].events &= ~t->notified_events;
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
                int ret = read (spin_global.pipe[0], &ch, sizeof(ch));
                if (ret == 0)
                    /* The dummy_pipe[1] is closed, indicate that this thread
                     * should go exit */
                    looping = 0;
            }
        }
        pthread_cond_broadcast (&spin_global.cond);
        pthread_mutex_unlock (&spin_global.lock);
    }

    return NULL;
}

int spin_init (void)
{
    int ret;
    pthread_condattr_t condattr;
    struct epoll_event event;

    if (spin_global.epollfd > 0)
        return 1;

#ifndef NDEBUG
    spin_global.log = fdopen(dup(fileno(stderr)), "w");
    if (spin_global.log == NULL)
        return -1;
#endif

    ret = pthread_condattr_init (&condattr);
    if (ret != 0)
        goto error_clean_up;
    ret = pthread_condattr_setclock (&condattr, CLOCK_MONOTONIC);
    if (ret != 0)
        goto error_clean_up;
    ret = pthread_cond_init (&spin_global.cond, &condattr);
    if (ret != 0)
        goto error_clean_up;
    ret = clock_gettime (CLOCK_MONOTONIC, &spin_global.basetime);
    if (ret != 0)
        goto error_clean_up;

    ret = epoll_create1 (O_CLOEXEC);
    if (ret == -1)
        goto error_clean_up;
    spin_global.epollfd = ret;

    ret = pipe2 (spin_global.pipe, O_CLOEXEC | O_NONBLOCK);
    if (ret == -1)
        goto error_clean_up;

    event.events = EPOLLIN | EPOLLET;
    event.data.ptr = NULL;

    ret = epoll_ctl (spin_global.epollfd, EPOLL_CTL_ADD,
                     spin_global.pipe[0], &event);

    if (ret == -1)
        goto error_clean_up;

    ret = pthread_create (&spin_global.thread, NULL, spin_poll_thread, NULL);
    if (ret == -1)
        goto error_clean_up;

    spin_debug ("libspin initialized");

    return 0;
error_clean_up:
    if (spin_global.epollfd != 0) {
        close (spin_global.epollfd);
        spin_global.epollfd = 0;
    }

    if (spin_global.pipe[0] != 0) {
        close (spin_global.pipe[0]);
        spin_global.pipe[0] = 0;
    }

    if (spin_global.pipe[1] != 0) {
        close (spin_global.pipe[1]);
        spin_global.pipe[1] = 0;
    }
    return -1;
}

int spin_uninit (void)
{
    /* Close the write end so epoll will be notified */
    close (spin_global.pipe[1]);
    pthread_join (spin_global.thread, NULL);
    pthread_cond_destroy (&spin_global.cond);
    close (spin_global.pipe[0]);
    close (spin_global.epollfd);
    return 0;
}

#ifndef NDEBUG
void spin_debug(const char *fmt, ...)
{
    va_list args;

    assert (fmt != NULL);
    fprintf (spin_global.log, "libspin: ");

    va_start (args, fmt);
    vfprintf (spin_global.log, fmt, args);
    va_end (args);

    if (fmt[strlen(fmt) - 1] != '\n')
        fputc ('\n', spin_global.log);
}
#endif
