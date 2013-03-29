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

/* This header file is for internal usage */

#ifndef __SPIN_TIMESPEC_H__
#define __SPIN_TIMESPEC_H__

#include <stdint.h>
#include <time.h>
#include <assert.h>


static inline void timespec_add_milliseconds (struct timespec *ts, int64_t ms)
{
    assert (ts != NULL);
    uint64_t sec = ms / 1000;
    ms -= sec * 1000;

    ts->tv_nsec += ms * 1000000;
    ts->tv_sec += ts->tv_nsec / 1000000000 + sec;
    ts->tv_nsec = ts->tv_nsec % 1000000000;
}

static inline int64_t timespec_diff_millisecons (struct timespec *lhs,
                                                 struct timespec *rhs)
{
    int64_t ms;
    assert (lhs != NULL);
    assert (rhs != NULL);
    ms = 0;
    ms += (lhs->tv_nsec - rhs->tv_nsec) / 1000000;
    ms += (lhs->tv_sec - rhs->tv_sec) * 1000;
    return ms;
}

static inline void timespec_now (struct timespec *ts)
{
    assert (ts != NULL);
    clock_gettime (CLOCK_MONOTONIC, ts);
}

#endif
