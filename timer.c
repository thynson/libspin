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

enum {
    TIMER_STATE_STOP,
    TIMER_STATE_START,
};

static int spin_timer_task_callback (spin_task_t task);

static inline int tick_add_expiration (prioque_weight_t *ret)
{
    struct timespec now;
    int x;
    x = timespec_now (&now);
    if (x != 0)
        return x;
    *ret += timespec_diff_millisecons (&now, &startup_timespec);
    return 0;
}

static int spin_timer_task_callback (spin_task_t task)
{
    int ret;
    spin_timer_t timer = CAST_TASK_TO_TIMER (task);
    ret = timer->callback(timer->context);
    if (timer->interval != 0) {
        timer->loop->refcount++;
        timer->tick += timer->interval;

        if (ret != 0)
            return ret;

        ret = prioque_insert (timer->loop->prioque,
                              &timer->q, timer->tick);
        if (ret != 0)
            return ret;
    }
    return 0;
}

spin_timer_t spin_timer_create (spin_loop_t loop, int (*callback) (void*),
                                void *context)
{
    if (loop == NULL || callback == NULL) {
        errno = EINVAL;
        return NULL;
    }

    spin_timer_t t = (spin_timer_t) malloc (sizeof (*t));

    if (t == NULL)
        return NULL;

    spin_task_init (&t->task, spin_timer_task_callback);
    t->q.__offset = -1;
    t->loop = loop;
    t->callback = callback;
    t->context = context;
    t->interval = 0;
    t->tick = 0;

    return t;
}

int spin_timer_destroy (spin_timer_t timer)
{
    int ret;
    /* FIXME: remove from loop */
    if (timer == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (timer->q.__offset >= 0) {
        ret = prioque_remove (timer->loop->prioque, &timer->q);
        timer->loop->refcount--;
        if (ret != 0)
            return ret;
    } else if (!link_node_is_dettached(&timer->task.l)) {
        link_list_dettach (&timer->loop->currtask, &timer->task.l);
        timer->loop->refcount--;
    }
    free (timer);

    return 0;
}



int spin_timer_ctl (spin_timer_t timer,
                    const struct spin_itimespec *val,
                    struct spin_itimespec *stat)
{
    int ret;

    if (timer == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (stat != NULL) {
        if (timer->q.__offset >= 0) {
            stat->interval = timer->interval;
            prioque_weight_t tick = 0;

            ret = tick_add_expiration (&tick);
            if (ret != 0)
                return ret;

            stat->initial = timer->tick - tick;
        } else if (!link_node_is_dettached (&timer->task.l)) {
            stat->interval = timer->interval;
            stat->initial = 0;
        } else {
            stat->interval = 0;
            stat->initial = 0;
        }
    }

    if (val != NULL) {
        if (val->interval == 0 && val->initial == 0) {
            /* Stop the timer */
            timer->interval = 0;
            timer->tick = 0;
            if (timer->q.__offset >= 0) {
                int ret = prioque_remove (timer->loop->prioque,
                                          &timer->q);
                if (ret != 0)
                    return ret;
                timer->loop->refcount--;
            } else if (!link_node_is_dettached (&timer->task.l)) {
                link_list_dettach (&timer->loop->currtask, &timer->task.l);
                timer->loop->refcount--;
            }
        } else {
            timer->interval = val->interval;
            if (val->initial == 0)
                timer->tick = val->interval;
            else
                timer->tick = val->initial;

            ret = tick_add_expiration (&timer->tick);

            if (ret != 0)
                return ret;

            if (timer->q.__offset >= 0) {
                ret = prioque_update (timer->loop->prioque,
                                      &timer->q, timer->tick);
                if (ret != 0)
                    return ret;
            } else if (!link_node_is_dettached(&timer->task.l)) {
                link_list_dettach (&timer->loop->currtask, &timer->task.l);
                ret = prioque_insert (timer->loop->prioque,
                                      &timer->q, timer->tick);
                if (ret != 0)
                    return ret;
            } else {
                /* Start the timer, increase refcount */
                timer->loop->refcount++;
                ret = prioque_insert (timer->loop->prioque,
                                      &timer->q, timer->tick);
            }
        }
    }
    return 0;
}

