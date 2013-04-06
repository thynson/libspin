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
#include "spin/spin.h"
#include "linklist.h"
#include "prioque.h"
#include "timespec.h"

#define SPIN_DEFINE_DOWNCAST(type, member, args)\
    ((type *)((int8_t *)(args) - offsetof(type, member)))



enum {
    SPIN_LOOP_PREPARE = 0,
    SPIN_LOOP_RUN,
    SPIN_LOOP_EXIT
};

extern pthread_once_t startup_timespec_once;
extern struct timespec startup_timespec;

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
    size_t refcount;

    /* Misc */
    int dummy_pipe[2];
};

typedef struct __spin_task *spin_task_t;

struct __spin_task {
    union {
        prioque_node_t q;
        link_node_t l;
    } node;
    int (*callback) (spin_task_t);
};

#define CAST_PRIOQUE_NODE_TO_TASK(x) \
    SPIN_DEFINE_DOWNCAST(struct __spin_task, node.q, x)

#define CAST_LINK_NODE_TO_TASK(x) \
    SPIN_DEFINE_DOWNCAST(struct __spin_task, node.l, x)

static inline void spin_task_init (spin_task_t task,
                                   int (*callback)(spin_task_t))
{
    task->node.q.__offset = 0;
    task->node.l.prev = NULL;
    task->node.l.next = NULL;
    task->callback = callback;
}

struct __spin_timer {
    struct __spin_task task;
    spin_loop_t loop;
    int (*callback) (void *);
    void *context;
};

#define CAST_TASK_TO_TIMER(x) \
    SPIN_DEFINE_DOWNCAST(struct __spin_timer, task, x)

struct __spin_poll_target;

typedef struct __spin_poll_target *spin_poll_target_t;

struct __spin_poll_target {
    struct __spin_task task;
    int (*callback)(int event, spin_poll_target_t);
    int events;
};

#define CAST_TASK_TO_POLL_TARGET(x) \
    SPIN_DEFINE_DOWNCAST(spin_poll_target_t, task, x)

#endif
