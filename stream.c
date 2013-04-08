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
        assert (in_req->count < in_req->size);

        read_size = in_req->size - in_req->count;

        if (read_size > tmpsize)
            read_size = tmpsize;

        retsize = stream->spec.read (stream, in_req->buff + in_req->count,
                                     read_size);

        if (retsize == -1) {
            /* TODO: Handle error and test EAGAIN */
            return -1;
        } else if (retsize == 0) {
            /* EOF */
            in_req->callback (in_req);
            return 1;
        }

        in_req->count += retsize;
        tmpsize -= retsize;

        if (retsize < read_size) {
            if (errno == EAGAIN) {
                return -1;
            }
        }
        if (in_req->count == in_req->size) {
            stream->in_req = NULL;
            in_req->callback (in_req);
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
        assert (out_req->count < out_req->size);

        write_size = out_req->size - out_req->count;

        if (write_size > tmpsize)
            write_size  = tmpsize;

        retsize = stream->spec.write (stream, out_req->buff + out_req->count,
                                      write_size);

        if (retsize == -1) {
            /* TODO: Handle error and test EAGAIN */
            return -1;
        } else if (retsize == 0) {
            /* XXX: Won't happen if read_size > 0 */
        }

        out_req->count += retsize;
        tmpsize -= retsize;

        if (retsize < write_size) {
            if (errno == EAGAIN) {
                return -1;
            }
        }

        if (out_req->count == out_req->size) {
            stream->out_req = NULL;
            out_req->callback(out_req);
        }
    }
    return 0;
}

static int stream_poll_target_callback (spin_poll_target_t pt)
{
    spin_stream_t stream = CAST_POLL_TARGET_TO_STREAM(pt);

    int ret;
    if (stream->poll_target.cached_events & EPOLLIN) {
        ret = stream_handle_read (stream);
        if (ret != 0) {
            stream->poll_target.cached_events &= ~EPOLLIN;
        }
    }

    if (stream->poll_target.cached_events & EPOLLOUT) {
        ret = stream_handle_write (stream);
        if (ret != 0) {
            stream->poll_target.cached_events &= ~EPOLLOUT;
        }
    }

    return 0;
}


void spin_stream_init (spin_stream_t is, spin_loop_t loop,
                       const struct spin_stream_spec *spec)
{
    spin_poll_target_init (&is->poll_target, loop,
                           stream_poll_target_callback);

    is->in_req = NULL;
    is->out_req = NULL;
}


int spin_stream_read (spin_stream_t stream, struct spin_io_req *req)
{
    if (stream == NULL || req == NULL
            || req->buff == NULL || req->count >= req->size) {
        errno = EINVAL;
        return -1;
    }

    if (stream->in_req != NULL) {
        errno = EBUSY;
        return -1;
    }

    stream->in_req = req;

    if (link_node_is_dettached (&stream->poll_target.task.l))
        link_list_attach_to_tail (&stream->poll_target.loop->nexttask,
                                  &stream->poll_target.task.l);

    return 0;
}

int spin_stream_write (spin_stream_t stream, struct spin_io_req *req)
{
    if (stream == NULL || req == NULL
            || req->buff == NULL || req->count >= req->size) {
        errno = EINVAL;
        return -1;
    }

    if (stream->out_req != NULL) {
        errno = EBUSY;
        return -1;
    }

    stream->out_req = req;
    if (link_node_is_dettached (&stream->poll_target.task.l))
        link_list_attach_to_tail (&stream->poll_target.loop->nexttask,
                                  &stream->poll_target.task.l);
    return 0;
}