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

typedef struct __spin_socket *spin_socket_t;

struct __spin_socket {
    struct __spin_stream stream;
    int fd;
    void (*callback) (spin_stream_t);
};

#define CAST_STREAM_TO_SOCKET(x) \
    SPIN_DEFINE_DOWNCAST(struct __spin_socket, stream, x)

static ssize_t
spin_socket_read (spin_stream_t stream, char *buff, size_t size)
{
    spin_socket_t socket = CAST_STREAM_TO_SOCKET (stream);
    ssize_t ret;
    do
        ret = recv (socket->fd, buff, size, 0);
    while (ret == -1 && errno == EINTR);
    return ret;
}

static ssize_t
spin_socket_write  (spin_stream_t stream, const char *buff, size_t size)
{
    spin_socket_t socket = CAST_STREAM_TO_SOCKET (stream);
    ssize_t ret;
    do
        ret = send (socket->fd, buff, size, MSG_NOSIGNAL);
    while (ret == -1 && errno == EINTR);
    return ret;
}

static int
spin_socket_close (spin_stream_t stream)
{
    spin_socket_t socket = CAST_STREAM_TO_SOCKET (stream);
    return close (socket->fd);
}

static int
spin_socket_connected (spin_poll_target_t pt)
{
    spin_socket_t s = CAST_STREAM_TO_SOCKET (CAST_POLL_TARGET_TO_STREAM (pt));

    if (s->stream.poll_target.cached_events & EPOLLERR) {
        void (*callback) (spin_stream_t);
        callback = s->callback;
        close (s->fd);
        free (s);
        callback (NULL);
    } else {
        struct spin_stream_spec spec;
        int cached_events = s->stream.poll_target.cached_events;
        spec.read = &spin_socket_read;
        spec.write = &spin_socket_write;
        spec.close = &spin_socket_close;

        spin_stream_init (&s->stream, s->stream.poll_target.loop, &spec);
        s->stream.poll_target.cached_events = cached_events;
        s->callback (&s->stream);
    }
    return 0;
}

int spin_tcp_connect (spin_loop_t loop, const struct sockaddr_storage *addr,
                      void (*callback) (spin_stream_t))
{
    int flags, ret;
    struct epoll_event event;
    socklen_t length = sizeof (*addr);
    spin_socket_t sock = (spin_socket_t) malloc (sizeof (*socket));

    if (sock == NULL)
        return -1;
    spin_poll_target_init (&sock->stream.poll_target, loop,
                           spin_socket_connected);

    sock->callback = callback;
    sock->fd = socket (addr->ss_family, SOCK_STREAM, 0);
    if (sock->fd == -1)
        goto cleanup_and_exit;

    flags = fcntl (sock->fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl (sock->fd, F_SETFL, flags);

    event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    event.data.ptr = &sock->stream.poll_target;

    ret = epoll_ctl (loop->epollfd, EPOLL_CTL_ADD, sock->fd, &event);
    link_list_attach_to_tail (&loop->polltask,
                              &sock->stream.poll_target.task.l);


    ret = connect (sock->fd, (const struct sockaddr *)addr, length);

    if (ret == -1 && errno != EINPROGRESS)
        goto cleanup_and_exit;
    else if (ret == 0) {
        /* Will it happen? */
    }

    return 0;
cleanup_and_exit:

    if (sock != NULL) {
        if (sock->fd > 0)
            close (sock->fd);

        free (sock);
    }
    return -1;
}


