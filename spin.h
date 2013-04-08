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
    link_list_t polltask;
    prioque_t prioque;

    /* Misc */
    int dummy_pipe[2];
};

typedef struct __spin_task *spin_task_t;

struct __spin_task {
    link_node_t l;
    int (*callback) (spin_task_t);
};

#define CAST_LINK_NODE_TO_TASK(x) \
    SPIN_DEFINE_DOWNCAST(struct __spin_task, l, x)

static inline void spin_task_init (spin_task_t task,
                                   int (*callback)(spin_task_t))
{
    task->l.prev = NULL;
    task->l.next = NULL;
    task->callback = callback;
}

struct __spin_timer {
    struct __spin_task task;
    prioque_node_t q;
    spin_loop_t loop;
    int (*callback) (void *);
    void *context;
    prioque_weight_t tick;
    unsigned interval;
};

#define CAST_TASK_TO_TIMER(x) \
    SPIN_DEFINE_DOWNCAST(struct __spin_timer, task, x)

#define CAST_PRIOQUE_NODE_TO_TIMER(x) \
    SPIN_DEFINE_DOWNCAST(struct __spin_timer, q, x)

struct __spin_poll_target;

typedef struct __spin_poll_target *spin_poll_target_t;

struct __spin_poll_target {
    struct __spin_task task;
    struct __spin_task bgtask;
    spin_loop_t loop;
    int (*callback)(spin_poll_target_t);
    int cached_events;
    int notified_events;
};

void
spin_poll_target_init (spin_poll_target_t pt,
                       spin_loop_t loop,
                       int (*callback)(spin_poll_target_t pt));

#define CAST_TASK_TO_POLL_TARGET(x) \
    SPIN_DEFINE_DOWNCAST(struct __spin_poll_target, task, x)
#define CAST_BGTASK_TO_POLL_TARGET(x) \
    SPIN_DEFINE_DOWNCAST(struct __spin_poll_target, bgtask, x)


struct spin_stream_spec {
    ssize_t (*read) (spin_stream_t stream, char *buff, size_t size);
    ssize_t (*write) (spin_stream_t stream, const char *buff, size_t size);
    ssize_t (*close) (spin_stream_t stream);
};

struct __spin_stream {
    struct __spin_poll_target poll_target;
    struct spin_stream_spec spec;
    struct spin_io_req *in_req;
    struct spin_io_req *out_req;
};

#define CAST_POLL_TARGET_TO_STREAM(x) \
    SPIN_DEFINE_DOWNCAST(struct __spin_stream, poll_target, x)

#define CAST_TASK_TO_STRAM(x) \
    SPIN_DEFINE_DOWNCAST(struct __spin_stream, poll_target.task, x)

#endif