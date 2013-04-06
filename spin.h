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

#ifndef __SPIN_H_INCLUDED__
#define __SPIN_H_INCLUDED__

#include <sys/epoll.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include "spin/loop.h"
#include "linklist.h"
#include "prioque.h"
#include "timespec.h"

enum {
    SPIN_LOOP_PREPARE = 0,
    SPIN_LOOP_RUN,
    SPIN_LOOP_EXIT
};

struct __spin_loop {
    pthread_t poll_thread;
    pthread_mutex_t lock;
    pthread_cond_t guard;

    /* task and bgtask may used across thread */
    int epollfd;
    link_list_t bgtask;

    /* the following member should only used in running thread */
    link_list_t currtask;
    link_list_t nexttask;
    prioque_t prioque;
    unsigned refcount;

    /* Misc */
    int dummy_pipe[2];
};

struct __spin_task {
    union {
        prioque_node_t q;
        link_node_t l;
    } node;
    int (*callback) (spin_task_t);
};

#define CAST_PRIOQUE_NODE_TO_TASK(x) \
    ((spin_task_t)((int8_t *)(x) - offsetof(struct __spin_task, node.q)))

#define CAST_LINK_NODE_TO_TASK(x) \
    ((spin_task_t)((int8_t *)(x) - offsetof(struct __spin_task, node.l)))

#endif
