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

spin_timer_t __SPIN_EXPORT__
spin_timer_create (spin_loop_t loop, unsigned msecs,
                  int (*callback)(void*), void *args);

int __SPIN_EXPORT__
spin_timer_destroy (spin_timer_t timer);

int __SPIN_EXPORT__
spin_timer_pause (spin_timer_t timer);

int __SPIN_EXPORT__
spin_timer_resume (spin_timer_t timer);

#endif
