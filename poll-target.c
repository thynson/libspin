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

/*
 * @brief Run foregound callback
 */
static int spin_poll_target_task_callback (spin_task_t task)
{
    spin_poll_target_t pt = CAST_TASK_TO_POLL_TARGET (task);
    pt->callback (pt);
    if (pt->cached_events != 0)
        link_list_attach_to_tail(&pt->loop->nexttask, &task->l);
    return 0;
}

/*
 * @brief Update cache event flag
 *
 * This function will be called when epoll reports event happened for
 * associated file descriptor. And run the callback if necessary
 */
static int spin_poll_target_bgtask_callback (spin_task_t task)
{
    spin_poll_target_t pt = CAST_BGTASK_TO_POLL_TARGET (task);
    pt->cached_events = pt->notified_events;
    if (link_node_is_dettached (&pt->task.l)) {
        link_list_attach_to_tail (&pt->loop->nexttask, &pt->task.l);
    }
    return 0;
}

/*
 * @brief Initialize a poll-target object
 */
void spin_poll_target_init (spin_poll_target_t pt, spin_loop_t loop,
                            int (*callback) (spin_poll_target_t pt))
{
    spin_task_init (&pt->task, spin_poll_target_task_callback);
    spin_task_init (&pt->bgtask, spin_poll_target_bgtask_callback);
    pt->loop = loop;
    pt->callback = callback;
}

