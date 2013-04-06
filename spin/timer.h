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

#ifndef __SPIN_TIMER_H__
#define __SPIN_TIMER_H__

#include "config.h"
#include "loop.h"

typedef struct __spin_timer *spin_timer_t;

struct spin_itimespec {
    unsigned initial;   /* Initial expiration */
    unsigned interval;  /* Interval, 0 indicates one-shot timer */
};

/**
 * @brief Create a timer
 * @param loop The event loop object which handle this timer
 * @param callback The callback routine to be called when timer event is fired
 * @param context The context parameter for callback
 */
spin_timer_t __SPIN_EXPORT__
spin_timer_create (spin_loop_t loop, int (*callback) (void *), void *context);

/**
 * @brief Destroy a timer
 * @param timer The timer to be destroyed
 */
int __SPIN_EXPORT__
spin_timer_destroy (spin_timer_t timer);

/**
 * @brief Operate a timer
 * @param timer The timer
 * @param val Specifies the initial expiration and interval for the timer, or
 *            give NULL value if you just want to get the status of timer
 * @param stat Current state of timer, or give NULL if you don't care
 * @note If val->initial is 0, the timer will not arm immediatly but wait for
 *       an interval time and if val->interval is 0, the timer will stop after
 *       first arm. If both val->initial and val->interval is 0, the timer
 *       will be stop. While stat->initial reports the remaining time before
 *       next arm and stat->interval return current interval time
 */
int __SPIN_EXPORT__
spin_timer_ctl (spin_timer_t timer, const struct spin_itimespec *val,
                struct spin_itimespec *stat);


#endif
