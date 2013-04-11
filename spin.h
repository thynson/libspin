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

#define SPIN_DOWNCAST(type, member, args)\
    ((type *)((int8_t *)(args) - offsetof(type, member)))

extern struct spin_poller_t
{
    unsigned refcount;
    int epollfd;
    int pipe[2];
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_t thread;
    struct timespec basetime;
} spin_poller;

/**
 * @brief Do internal initialization, creating poll thread, etc
 * @retval 0 First init
 * @retval 1 Initialized
 * @retval -1 Failed
 * @see spin_uninit
 * @note This function may be called multriple times, and it's need to call
 *       spin_uninit same times, to do uninitialization
 */
extern int spin_init ();

/**
 * @brief Do uninitialize
 * @retval 0 Actually destroyed
 * @retval 1 Reference counter is greater than 0, pending
 * @retval -1 Failed
 * @see spin_init
 */
extern int spin_uninit ();

struct __spin_loop {

    /* task and bgtask may used across thread */
    prioque_t prioque;

    /* the following member should only used in running thread */
    link_list_t currtask;
    link_list_t nexttask;
    link_list_t polltask;

    /* only accessible when #spin_poller.lock is acquired, except being
     * initialized */
    link_list_t bgtask;
};

typedef struct __spin_task *spin_task_t;

struct __spin_task {
    link_node_t l;
    int (*callback) (spin_task_t);
};

#define CAST_LINK_NODE_TO_TASK(x) \
    SPIN_DOWNCAST(struct __spin_task, l, x)

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
    SPIN_DOWNCAST(struct __spin_timer, task, x)

#define CAST_PRIOQUE_NODE_TO_TIMER(x) \
    SPIN_DOWNCAST(struct __spin_timer, q, x)

struct __spin_poll_target;

typedef struct __spin_poll_target *spin_poll_target_t;

struct __spin_poll_target {
    /* TODO: pthread_spin_lock is needed */
    struct __spin_task in_task;
    struct __spin_task out_task;
    struct __spin_task err_task;
    spin_loop_t loop;
    int (*callback)(int, spin_poll_target_t);
    int cached_events;
};

void
spin_poll_target_init (spin_poll_target_t pt,
                       spin_loop_t loop,
                       int (*callback)(int event, spin_poll_target_t pt));

#define CAST_IN_TASK_TO_POLL_TARGET(x) \
    SPIN_DOWNCAST(struct __spin_poll_target, in_task, x)
#define CAST_OUT_TASK_TO_POLL_TARGET(x) \
    SPIN_DOWNCAST(struct __spin_poll_target, out_task, x)
#define CAST_ERR_TASK_TO_POLL_TARGET(x) \
    SPIN_DOWNCAST(struct __spin_poll_target, err_task, x)


struct spin_stream_spec {
    ssize_t (*read) (spin_stream_t stream, char *buff, size_t size);
    ssize_t (*write) (spin_stream_t stream, const char *buff, size_t size);
    int (*close) (spin_stream_t stream);
};

struct __spin_stream {
    struct __spin_poll_target poll_target;
    struct spin_stream_spec spec;
    struct __spin_task in_task;
    struct __spin_task out_task;
    struct spin_io_req *in_req;
    struct spin_io_req *out_req;
};

void spin_stream_init (spin_stream_t is, spin_loop_t loop,
                       const struct spin_stream_spec *spec);

#define CAST_POLL_TARGET_TO_STREAM(x) \
    SPIN_DOWNCAST(struct __spin_stream, poll_target, x)

#define CAST_TASK_TO_STREAM(x) \
    SPIN_DOWNCAST(struct __spin_stream, poll_target.task, x)

#define CAST_IN_TASK_TO_STREAM(x) \
    SPIN_DOWNCAST(struct __spin_stream, in_task, x);

#define CAST_OUT_TASK_TO_STREAM(x) \
    SPIN_DOWNCAST(struct __spin_stream, out_task, x);


#endif
