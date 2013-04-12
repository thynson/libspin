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


static inline int spin_loop_is_busy (spin_loop_t loop)
{
    return !link_list_is_empty (&loop->nexttask)
        || !link_list_is_empty (&loop->currtask)
        || !link_list_is_empty (&loop->polltask)
        || !prioque_is_empty (loop->prioque);
}

spin_loop_t spin_loop_create (void)
{
    spin_loop_t ret;

    if (spin_init () < 0)
        return NULL;

    ret = (spin_loop_t) calloc (1, sizeof(*ret));
    if (ret == NULL)
        return NULL;

    ret->prioque = prioque_create();
    if (ret->prioque == NULL)
        goto clean_and_exit;

    link_list_init (&ret->currtask);
    link_list_init (&ret->nexttask);
    link_list_init (&ret->polltask);
    link_list_init (&ret->bgtask);

    return ret;
clean_and_exit:

    if (ret != NULL) {
        if (ret->prioque != NULL)
            prioque_destroy (ret->prioque);
        free (ret);
    }

    return NULL;
}

int spin_loop_destroy (spin_loop_t loop)
{
    assert (loop != NULL);
    int busy;
    pthread_mutex_lock (&spin_poller.lock);
    busy = spin_loop_is_busy (loop);
    pthread_mutex_unlock (&spin_poller.lock);
    if (busy) {
        errno = EBUSY;
        return -1;
    }
    prioque_destroy (loop->prioque);
    free (loop);
    spin_uninit ();
    return 0;
}

int wait_for_task (spin_loop_t loop)
{
    int ret = 0, currtask_not_empty = 0;
    prioque_weight_t ticks;
    struct timespec now;
    prioque_node_t *pnode;

    ret = prioque_front (loop->prioque, &pnode);

    if (ret == -1)
        return -1;

    link_list_cat (&loop->currtask, &loop->nexttask);

    if (!link_list_is_empty (&loop->currtask))
        currtask_not_empty = 1;

    if (pnode == NULL) {
        pthread_mutex_lock (&spin_poller.lock);
        if (link_list_is_empty (&loop->bgtask)) {
            if (currtask_not_empty) {
                ret = 0;
            } else if (!link_list_is_empty (&loop->polltask)) {
                do
                    pthread_cond_wait (&spin_poller.cond, &spin_poller.lock);
                while (link_list_is_empty (&loop->bgtask));
                link_list_cat (&loop->currtask, &loop->bgtask);
            } else
                ret = 1;
        } else
            /* If no task is polling, then we don't need to wait */
            link_list_cat (&loop->currtask, &loop->bgtask);
        pthread_mutex_unlock (&spin_poller.lock);
        return ret;
    } else {
        struct timespec ts = spin_poller.basetime;
        prioque_get_node_weight (loop->prioque, pnode, &ticks);

        timespec_now (&now);
        timespec_add_milliseconds (&ts, ticks);
        pthread_mutex_lock (&spin_poller.lock);

        /* We don't need to check if there are task in bgtask list or
         * polltask list, but just wait till timedout */
        do {
            ret = pthread_cond_timedwait (&spin_poller.cond,
                                          &spin_poller.lock, &ts);
            if (ret == 0) {
                link_list_cat (&loop->currtask, &loop->bgtask);
                break;
            } else if (ret == ETIMEDOUT)
                break;
        } while (link_list_is_empty (&loop->bgtask));
        pthread_mutex_unlock (&spin_poller.lock);

        if (ret == ETIMEDOUT) {
            spin_timer_t timer = CAST_PRIOQUE_NODE_TO_TIMER (pnode);
            prioque_remove (loop->prioque, pnode);
            link_list_attach_to_tail (&loop->currtask, &timer->task.l);
            return 0;
        } else {
            errno = ret;
            return -1;
        }
    }
}


int spin_loop_run (spin_loop_t loop)
{
    int ret;
    if (loop == NULL) {
        errno = EINVAL;
        return -1;
    }
    link_list_cat (&loop->currtask, &loop->nexttask);
    while ((ret = wait_for_task (loop)) == 0) {
        while (!link_list_is_empty(&loop->currtask)) {
            link_node_t *node = loop->currtask.head;
            spin_task_t task = CAST_LINK_NODE_TO_TASK(node);
            link_list_dettach (&loop->currtask, node);
            task->callback(task);
        }
    }
    return 0;
}
