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

#ifndef __SPIN_STREAM_H__
#define __SPIN_STREAM_H__

#include "config.h"
#include "loop.h"
#include <stddef.h>

typedef struct __spin_stream *spin_stream_t;

struct spin_io_req {
    char *buff;
    size_t size;
    size_t count;
    void *context;
    void (*callback) (struct spin_io_req *req);
};

int __SPIN_EXPORT__
spin_stream_read (spin_stream_t stream, struct spin_io_req *req);

int __SPIN_EXPORT__
spin_stream_write (spin_stream_t stream, struct spin_io_req *req);

#endif
