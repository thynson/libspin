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
    SPIN_DOWNCAST(struct __spin_socket, stream, x)

struct __spin_tcp_server {
    struct __spin_poll_target poll_target;
    int fd;
    void (*connected) (spin_stream_t sock, const struct sockaddr_storage *);
    struct __spin_task in_task;
};

static ssize_t
spin_socket_read (spin_stream_t stream, char *buff, size_t size)
{
    spin_socket_t socket = CAST_STREAM_TO_SOCKET (stream);
    ssize_t ret;
    do
        ret = recv (socket->fd, buff, size, 0);
    while (ret == -1 && errno == EINTR);
    if (ret > 0 && ret < size) {
        errno = EAGAIN;
    }

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

static void
spin_socket_connected (spin_poll_target_t pt)
{
    spin_socket_t s = CAST_STREAM_TO_SOCKET (CAST_POLL_TARGET_TO_STREAM (pt));
    int event = pt->cached_events;

    link_list_dettach(&s->stream.poll_target.loop->polltask,
                      &s->stream.out_task.l);
    if (event & EPOLLERR) {
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
}

static int set_nonblocking (int fd)
{
    int flags;
    flags = fcntl (fd, F_GETFL);
    if (flags == -1)
        return -1;
    flags |= O_NONBLOCK;
    return fcntl (fd, F_SETFL, flags);
}

int spin_tcp_connect (spin_loop_t loop, const struct sockaddr_storage *addr,
                      void (*callback) (spin_stream_t))
{
    int ret;
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
    ret = set_nonblocking (sock->fd);

    event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    event.data.ptr = &sock->stream.poll_target;

    ret = epoll_ctl (spin_poller.epollfd, EPOLL_CTL_ADD, sock->fd, &event);

    link_list_attach_to_tail (&loop->polltask,
                              &sock->stream.out_task.l);


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

static void
spin_tcp_server_accept (spin_task_t t)
{
    spin_tcp_server_t srv = SPIN_DOWNCAST (struct __spin_tcp_server,
                                           in_task, t);
    size_t max_connect_once = 255;
    int fd;
    struct sockaddr_storage addr;
    socklen_t length = sizeof (addr);

    while (max_connect_once > 0) {
        spin_socket_t s;
        struct spin_stream_spec spec;
        struct epoll_event event;
        fd = accept (srv->fd, (struct sockaddr *)&addr, &length);
        if (fd ==-1)
            break;
        max_connect_once--;
        if (set_nonblocking (fd) == -1) {
            /* XXX */
            close (fd);
            continue;
        }

        s = (spin_socket_t) malloc (sizeof (*s));
        if (s == NULL) {
            close (fd);
            break;
        }
        s->fd = fd;
        s->callback = NULL;
        spec.read = &spin_socket_read;
        spec.write = &spin_socket_write;
        spec.close = &spin_socket_close;

        spin_stream_init (&s->stream, srv->poll_target.loop, &spec);
        event.events = EPOLLIN | EPOLLOUT | EPOLLET;
        event.data.ptr = &s->stream.poll_target;
        epoll_ctl (spin_poller.epollfd, EPOLL_CTL_ADD, fd, &event);
        srv->connected (&s->stream, &addr);
    }

    if (max_connect_once == 0) {
        spin_loop_next_round (srv->poll_target.loop, &srv->in_task);
    } else if (errno == EAGAIN) {
        spin_loop_wait_event (srv->poll_target.loop, &srv->in_task);
    }
}

static void
spin_tcp_server_poll_target_callback (spin_poll_target_t pt)
{
    int event = pt->cached_events;
    spin_tcp_server_t s = SPIN_DOWNCAST (struct __spin_tcp_server,
                                         poll_target, pt);
    if (event & EPOLLIN)
        spin_loop_fire_event (s->poll_target.loop, &s->in_task);
}


spin_tcp_server_t
spin_tcp_server_from_fd (spin_loop_t loop, int fd,
                         void (*connected) (spin_stream_t,
                                            const struct sockaddr_storage *))
{
    struct epoll_event event;
    spin_tcp_server_t srv;
    int ret = set_nonblocking (fd);

    if (ret == -1)
        return NULL;

    srv = (spin_tcp_server_t) malloc (sizeof (*srv));
    if (srv == NULL)
        return NULL;

    spin_poll_target_init (&srv->poll_target, loop,
                           spin_tcp_server_poll_target_callback);
    spin_task_init (&srv->in_task, spin_tcp_server_accept);

    srv->fd = fd;
    srv->connected = connected;

    event.events = EPOLLIN | EPOLLET;
    event.data.ptr = &srv->poll_target;
    ret = listen (fd, SOMAXCONN);

    if (ret == -1) {
        free (srv);
        return NULL;
    }

    ret = epoll_ctl (spin_poller.epollfd, EPOLL_CTL_ADD, fd, &event);

    if (ret == -1) {
        free (srv);
        return NULL;
    }

    spin_loop_wait_event (loop, &srv->in_task);

    return srv;
}

int spin_tcp_server_destroy (spin_tcp_server_t srv)
{
    /* TODO: Not implemented */
    return -1;
}
