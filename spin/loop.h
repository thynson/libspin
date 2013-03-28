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

#ifndef __SPIN_LOOP_H__
#define __SPIN_LOOP_H__

typedef struct __spin_loop *spin_loop_t;
typedef struct __spin_task *spin_task_t;

spin_loop_t spin_loop_create();

void spin_loop_destroy (spin_loop_t loop);

int spin_loop_run (spin_loop_t loop);

spin_task_t spin_task_create (spin_loop_t loop, unsigned msecs,
                              int (*callback)(void*), void *args);

void spin_task_destroy (spin_task_t task);

int spin_task_pause (spin_task_t task);

int spin_task_resume (spin_task_t task);


#endif
