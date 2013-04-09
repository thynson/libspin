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


/* XXX: We may use our symbol other than EPOLLIN/EPOLLOUT/EPOLLERR for
 * portability */

static int spin_poll_target_in_task_callback (spin_task_t task)
{
    spin_poll_target_t pt = CAST_IN_TASK_TO_POLL_TARGET (task);
    pt->cached_events |= EPOLLIN;
    pt->callback (EPOLLIN, pt);
    return 0;
}

static int spin_poll_target_out_task_callback (spin_task_t task)
{
    spin_poll_target_t pt = CAST_OUT_TASK_TO_POLL_TARGET (task);
    pt->cached_events |= EPOLLOUT;
    pt->callback (EPOLLOUT, pt);
    return 0;
}

static int spin_poll_target_err_task_callback (spin_task_t task)
{
    spin_poll_target_t pt = CAST_ERR_TASK_TO_POLL_TARGET (task);
    pt->cached_events |= EPOLLERR;
    pt->callback (EPOLLERR, pt);
    return 0;
}

/*
 * @brief Initialize a poll-target object
 */
void spin_poll_target_init (spin_poll_target_t pt, spin_loop_t loop,
                            int (*callback) (int, spin_poll_target_t pt))
{
    spin_task_init (&pt->in_task, spin_poll_target_in_task_callback);
    spin_task_init (&pt->out_task, spin_poll_target_out_task_callback);
    spin_task_init (&pt->err_task, spin_poll_target_err_task_callback);
    pt->loop = loop;
    pt->callback = callback;
    pt->cached_events = 0;
}


