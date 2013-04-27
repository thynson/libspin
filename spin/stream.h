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

/** @breif Stream handle */
typedef struct __spin_istream *spin_istream_t;
typedef struct __spin_ostream *spin_ostream_t;

/** @brief A struct descript I/O request
 *  @note It's sugguest to **extends** this structure for storing context */
struct spin_io_req {
    /** @brief Buff for data */
    char *buff;

    /** @brief Capacity of the buff */
    size_t maxsize;

    /** @brief Minimal size to be completed before calling callback,
     *         should not greater than maxsize */
    size_t minsize;

    /** @brief Actually completed length of data in the buffer, user should
     *         initialize it to 0 */
    size_t size;

    /** @brief Callback function which will be called when requested I/O
     *         operation has completed */
    void (*callback) (struct spin_io_req *req);
};

struct spin_pipe_req {
    void (*callback) (struct spin_pipe_req *req);
    size_t size;
    size_t minsize;
    size_t maxsize;
};

spin_streamop_t

/**
 * @brief Read from a stream asynchronously
 * @param istream The stream to be read from
 * @param req The description for this I/O operation request
 */
int __SPIN_EXPORT__
spin_read (spin_istream_t istream, struct spin_io_req *req);

/**
 * @brief Write to a stream asynchronously
 * @param ostream The stream to be written to
 * @param req The description for this I/O operation request
 */
int __SPIN_EXPORT__
spin_write (spin_ostream_t ostream, struct spin_io_req *req);

int __SPIN_EXPORT__
spin_pipe (spin_istream_t istream, spin_ostream_t ostream,
#endif
