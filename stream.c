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

#define IO_MAX_ONCE_SIZE 4096


static int stream_handle_read (spin_stream_t stream)
{
    size_t tmpsize = IO_MAX_ONCE_SIZE;

    while (tmpsize > 0 && stream->in_req != NULL) {
        size_t read_size;
        ssize_t retsize;
        struct spin_io_req *in_req = stream->in_req;

        assert (stream->spec.read != NULL);
        assert (in_req->buff != NULL);
        assert (in_req->size < in_req->maxsize);

        read_size = in_req->maxsize - in_req->size;

        if (read_size > tmpsize)
            read_size = tmpsize;

        retsize = stream->spec.read (stream, in_req->buff + in_req->size,
                                     read_size);

        if (retsize == -1) {
            /* TODO: Handle error and test EAGAIN */
            return -1;
        } else if (retsize == 0) {
            /* EOF */
            in_req->callback (in_req);
            return 1;
        }

        in_req->size += retsize;
        tmpsize -= retsize;


        if (in_req->size >= in_req->minsize) {
            stream->in_req = NULL;
            in_req->callback (in_req);
        } else if (retsize < read_size) {
            if (errno == EAGAIN) {
                return -1;
            }
        }
    }
    return 0;
}

static int stream_handle_write (spin_stream_t stream)
{
    size_t tmpsize = IO_MAX_ONCE_SIZE;

    while (tmpsize > 0 && stream->out_req != NULL) {
        size_t write_size;
        ssize_t retsize;
        struct spin_io_req *out_req = stream->out_req;

        assert (stream->spec.write != NULL);
        assert (out_req->size < out_req->maxsize);

        write_size = out_req->maxsize - out_req->size;

        if (write_size > tmpsize)
            write_size  = tmpsize;

        retsize = stream->spec.write (stream, out_req->buff + out_req->size,
                                      write_size);

        if (retsize == -1) {
            /* TODO: Handle error and test EAGAIN */
            return -1;
        } else if (retsize == 0) {
            /* XXX: Won't happen if read_size > 0 */
        }

        out_req->size += retsize;
        tmpsize -= retsize;

        if (out_req->size >= out_req->minsize) {
            stream->out_req = NULL;
            out_req->callback(out_req);
        } else if (retsize < write_size) {
            if (errno == EAGAIN) {
                return -1;
            }
        }

    }
    return 0;
}

static int stream_poll_target_callback (spin_poll_target_t pt)
{
    spin_stream_t stream = CAST_POLL_TARGET_TO_STREAM (pt);
    int event = pt->cached_events;

    /* TODO: Compare the cached_event sand notified_events, else link list may
     * be corrupted */
    if ((event & EPOLLIN) && (stream->in_req != NULL)) {
        link_list_dettach (&stream->poll_target.loop->polltask,
                           &stream->in_task.l);
        link_list_attach_to_tail (&stream->poll_target.loop->nexttask,
                                  &stream->in_task.l);
    }

    if ((event & EPOLLOUT) && (stream->out_req != NULL)) {
        link_list_dettach (&stream->poll_target.loop->polltask,
                           &stream->out_task.l);
        link_list_attach_to_tail (&stream->poll_target.loop->nexttask,
                                  &stream->out_task.l);
    }
    return 0;
}

static int stream_in_task_callback (spin_task_t task)
{
    int ret;
    spin_stream_t stream = CAST_IN_TASK_TO_STREAM (task);
    ret = stream_handle_read (stream);
    if (ret != 0) {
        if (stream->in_req != NULL) {
            link_list_attach_to_tail (&stream->poll_target.loop->polltask,
                                      &stream->out_task.l);
        }
        stream->poll_target.cached_events &= ~EPOLLIN;
    } else if (stream->in_req != NULL) {
        link_list_attach_to_tail (&stream->poll_target.loop->nexttask,
                                  &stream->in_task.l);
    }
    return 0;
}

static int stream_out_task_callback (spin_task_t task)
{
    int ret;
    spin_stream_t stream = CAST_OUT_TASK_TO_STREAM (task);
    ret = stream_handle_write (stream);
    if (ret != 0) {
        if (stream->out_req != NULL) {
            link_list_attach_to_tail (&stream->poll_target.loop->polltask,
                                      &stream->out_task.l);
        }
        stream->poll_target.cached_events &= ~EPOLLOUT;
    } else if (stream->out_req != NULL) {
        link_list_attach_to_tail (&stream->poll_target.loop->nexttask,
                                  &stream->out_task.l);
    }
    return 0;
}

void spin_stream_init (spin_stream_t is, spin_loop_t loop,
                       const struct spin_stream_spec *spec)
{
    spin_poll_target_init (&is->poll_target, loop,
                           stream_poll_target_callback);

    spin_task_init (&is->in_task, stream_in_task_callback);
    spin_task_init (&is->out_task, stream_out_task_callback);
    is->spec = *spec;
    is->in_req = NULL;
    is->out_req = NULL;
}

int spin_stream_read (spin_stream_t stream, struct spin_io_req *req)
{
    if (stream == NULL || req == NULL
            || req->buff == NULL || req->size >= req->maxsize) {
        errno = EINVAL;
        return -1;
    }

    if (stream->in_req != NULL) {
        errno = EBUSY;
        return -1;
    }

    stream->in_req = req;
    if (stream->poll_target.cached_events & EPOLLIN)
        link_list_attach_to_tail (&stream->poll_target.loop->nexttask,
                                  &stream->in_task.l);
    else
        link_list_attach_to_tail (&stream->poll_target.loop->polltask,
                                  &stream->in_task.l);

    return 0;
}

int spin_stream_write (spin_stream_t stream, struct spin_io_req *req)
{
    if (stream == NULL || req == NULL
            || req->buff == NULL || req->size >= req->maxsize) {
        errno = EINVAL;
        return -1;
    }

    if (stream->out_req != NULL) {
        errno = EBUSY;
        return -1;
    }

    stream->out_req = req;
    /* TODO: Read write use different task */
    if (stream->poll_target.cached_events & EPOLLOUT)
        link_list_attach_to_tail (&stream->poll_target.loop->nexttask,
                                  &stream->out_task.l);
    else
        link_list_attach_to_tail (&stream->poll_target.loop->polltask,
                                  &stream->out_task.l);
    return 0;
}
