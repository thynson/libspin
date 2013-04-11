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


static int spin_poll_target_task_callback (spin_task_t task)
{
    spin_poll_target_t pt = CAST_TASK_TO_POLL_TARGET (task);
    pthread_spin_lock (&pt->lock);
    pt->cached_events |= pt->notified_events;
    pt->notified_events = 0;
    pthread_spin_unlock (&pt->lock);
    pt->callback (pt);
    return 0;
}

/*
 * @brief Initialize a poll-target object
 */
void spin_poll_target_init (spin_poll_target_t pt, spin_loop_t loop,
                            int (*callback) (spin_poll_target_t pt))
{
    spin_task_init (&pt->task, spin_poll_target_task_callback);
    pthread_spin_init (&pt->lock, 0);
    pt->loop = loop;
    pt->callback = callback;
    pt->cached_events = 0;
}


