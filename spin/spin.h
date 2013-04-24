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

#ifndef __SPIN_SPIN_H__
#define __SPIN_SPIN_H__

#include "config.h"
#include "loop.h"
#include "timer.h"
#include "stream.h"
#include "socket.h"

/**
 * @brief Do internal initialization, creating poll thread, etc
 * @retval 0 First init
 * @retval 1 Initialized
 * @retval -1 Failed
 * @see spin_uninit
 */
int __SPIN_EXPORT__
spin_init (void);

/**
 * @brief Do uninitialize
 * @retval 0 Actually destroyed
 * @retval -1 Failed
 * @see spin_init
 */
int __SPIN_EXPORT__
spin_uninit (void);

#endif
