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

#include "config.h"

/** @breif Event loop handle */
typedef struct __spin_loop *spin_loop_t;

/**
 * @brief Create an event loop
 */
spin_loop_t __SPIN_EXPORT__
spin_loop_create();

/**
 * @brief Destroy an event loop
 * @param loop The even loop
 * @note you should not close an spin object within event loop!  the control
 *       flow of the program will block on spin_loop_run if the are any event
 *       to be fired. you should only close an spin object before or after
 *       calling spin_loop_run
 */
int __SPIN_EXPORT__
spin_loop_destroy (spin_loop_t loop);

/**
 * @brief Run an event loop
 * @param loop The event loop
 * @note this function will block until the all associated task finished.
 */
int __SPIN_EXPORT__
spin_loop_run (spin_loop_t loop);


#endif
