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

static int spin_timer_task_callback (spin_task_t task);
static spin_timer_t spin_timer_alloc (int (*callback)(void *), void *context);
static inline void spin_timer_free (spin_timer_t t);

static int spin_timer_task_callback (spin_task_t task)
{
    spin_timer_t timer = CAST_TASK_TO_TIMER (task);
    timer->callback(timer->context);
    /* XXX: clean up in else where ?*/
    spin_timer_free (timer);
    return 0;
}

static spin_timer_t spin_timer_alloc (int (*callback)(void *), void *context)
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

