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
#include <string.h> /* For strerror_r */

typedef struct __spin_socket *spin_socket_t;

enum {
    SOCK_CLOSE_READ = (1 << 0),
    SOCK_CLOSE_WRITE = (1 << 1)
};

struct __spin_socket {
    struct __spin_stream stream;
    int fd;
    void (*callback) (spin_stream_t);
    int state;
};

#define CAST_STREAM_TO_SOCKET(x) \
    SPIN_DOWNCAST(struct __spin_socket, stream, x)

struct __spin_tcp_server {
    struct __spin_poll_target poll_target;
    int fd;
    void (*connected) (spin_stream_t sock, const struct sockaddr_storage *);
    struct __spin_task in_task;
};

static int
spin_socket_read (spin_stream_t stream, char *buff, size_t *size)
{
    spin_socket_t socket = CAST_STREAM_TO_SOCKET (stream);
    ssize_t ret;

    assert (*size <= SSIZE_MAX && *size > 0);
retry:
    ret = recv (socket->fd, buff, *size, 0);

    if (ret >= 0) {
        if (ret < *size) {
            *size = ret;
            return 1;
        } else {
            *size = ret;
            return 0;
        }
    } else {
        assert (ret == -1);
        *size = 0;
        switch (errno) {
        /* Need retry */
        case EINTR:
            goto retry;

        /* We should wait for an EPOLLIN event */
        case EAGAIN:
            return 1;

        /* Fatal error: socket becomes unusable and should be closed */
        case EPIPE:
        case ECONNRESET:
        case ETIMEDOUT:
        case ENOMEM:
        case ENOBUFS:
            /* XXX: Will ENOMEM and ENOBUFS make a socket unusable? */
            /* TODO: Inform user to close socket */
            return -1;

        /* Fatal error: must be a bugs in this library */
        case EFAULT:
        case EINVAL:
        case EBADF:
        case ENOTSOCK:
        case ENOTCONN:
        default: /* We put unknown error here */
            {
                int err = errno;
                assert (err != 0);
                char tmp[1024] = { 0 };
                /* This is GNU version of strerror_r */
                const char *str = strerror_r (err, tmp, sizeof(tmp));
                spin_debug ("Fatal error in %s", __func__);
                spin_debug ("%s", str);
                abort ();
            }
        }
        return ret;
    }
}

static int
spin_socket_write  (spin_stream_t stream, const char *buff, size_t *size)
{
    spin_socket_t socket = CAST_STREAM_TO_SOCKET (stream);
    ssize_t ret;

    assert (*size <= SSIZE_MAX && *size > 0);
retry:
    ret = send (socket->fd, buff, *size, MSG_NOSIGNAL);
    if (ret > 0) {
        if (ret < *size) {
            *size = ret;
            return 1;
        } else {
            *size = ret;
            return 0;
        }
    } else {
        assert (ret == -1);
        *size = 0;
        switch (errno) {
        /* Need retry */
        case EINTR:
            goto retry;

        /* We should wait for an EPOLLIN event */
        case EAGAIN:
            return 1;

        /* Fatal error: socket becomes unusable and should be closed */
        case EPIPE:
        case ETIMEDOUT:
        case ECONNRESET:
        case ENOMEM:
        case ENOBUFS:
            /* XXX: Will ENOMEM and ENOBUFS make a socket unusable? */
            /* TODO: Inform user to close socket */
            return -1;

        /* Fatal error: must be a bugs in this library */
        case EFAULT:
        case EINVAL:
        case EBADF:
        case ENOTSOCK:
        case ENOTCONN:
        /* Since we are neither UNIX socket nor UDP socket, the following
         * errno should not appear. */
        case EACCES:
        case EDESTADDRREQ:
        default: /* We put unknown error here */
            {
                int err = errno;
                assert (err != 0);
                char tmp[1024] = { 0 };
                /* This is GNU version of strerror_r */
                const char *str = strerror_r (err, tmp, sizeof(tmp));
                spin_debug ("Fatal error in %s", __func__);
                spin_debug ("%s", str);
                abort ();
            }
        }
        return ret;
    }
}

static int
spin_socket_close (spin_stream_t stream)
{
    spin_socket_t socket = CAST_STREAM_TO_SOCKET (stream);
    return close (socket->fd);
}

static void
spin_socket_connected (int event, spin_poll_target_t pt)
{
    spin_socket_t s = CAST_STREAM_TO_SOCKET (CAST_POLL_TARGET_TO_STREAM (pt));

    s->state = 0;
    link_list_dettach(&s->stream.poll_target.loop->polltask,
                      &s->stream.out_task.l);
    if (event & EPOLLERR) {
        void (*callback) (spin_stream_t);
        callback = s->callback;
        close (s->fd);
        free (s);
        callback (NULL);
    } else {
        /* TODO: Fix here */
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
    sock->state = 0;

    if (sock->fd == -1)
        goto cleanup_and_exit;
    ret = spin_set_nonblocking (sock->fd);

    event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    event.data.ptr = &sock->stream.poll_target;

    ret = epoll_ctl (spin_global.epollfd, EPOLL_CTL_ADD, sock->fd, &event);

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

static int
spin_tcp_server_accept_client (spin_tcp_server_t tcpserver)
{
    struct sockaddr_storage addr;
    socklen_t length;
    spin_socket_t s;
    struct epoll_event event;
    struct spin_stream_spec spec;
    int fd;

retry:
    length = sizeof (addr);
    fd = accept (tcpserver->fd, (struct sockaddr *) &addr, &length);

    if (fd > 0) {
        s = (spin_socket_t) malloc (sizeof (*s));
        if (s == NULL) {
            /*TODO： Report error */
            return -1;
        }
        s->fd = fd;
        s->callback = NULL;
        s->state = 0;
        spec.read = &spin_socket_read;
        spec.write = &spin_socket_write;
        spec.close = &spin_socket_close;

        spin_stream_init (&s->stream, tcpserver->poll_target.loop, &spec);
        event.events = EPOLLIN | EPOLLOUT | EPOLLET;
        event.data.ptr = &s->stream.poll_target;
        epoll_ctl (spin_global.epollfd, EPOLL_CTL_ADD, fd, &event);
        tcpserver->connected (&s->stream, &addr);
        return 0;
    } else {
        assert (fd == -1);
        switch (errno) {

        /* In such case(s) we should ignore and retry */
        case EINTR:
        case ECONNABORTED:
        case EPROTO:
            goto retry;

        /* In such case(s) we should return and wait an EPOLLIN
         * NOTE: On Linux EAGAIN is equals to EWOULDBLOCK, list both of them
         * here will result in compile error. */
        case EAGAIN:
            break;

        /* Fatal error, must be a bug of this library */
        case EBADF:
        case EINVAL:
        case EFAULT:
        case ENOTSOCK:
        case EOPNOTSUPP:

            abort();
            break;

        /* Non-fatal error, but should report to user */
        case EPERM:
        case ENOBUFS:
        case ENOMEM:
        case EMFILE:
        case ENFILE:
            /* TODO: Report error */
            return -1;
        default:
            abort();
        }

        errno = 0;
        return 1;
    }
}

static void
spin_tcp_server_accept (spin_task_t t)
{
    spin_tcp_server_t srv = SPIN_DOWNCAST (struct __spin_tcp_server,
                                           in_task, t);
    size_t max_connect_once = 255;
    int result = 0;

    while (max_connect_once-- > 0 && result == 0)
        result = spin_tcp_server_accept_client (srv);

    if (max_connect_once == 0) {
        spin_loop_next_round (srv->poll_target.loop, &srv->in_task);
    } else if (result == 1) {
        spin_poll_target_clean_cached_event (&srv->poll_target, EPOLLIN);
        spin_loop_wait_event (srv->poll_target.loop, &srv->in_task);
    } else {
        /* TODO */
    }
}

static void
spin_tcp_server_poll_target_callback (int event, spin_poll_target_t pt)
{
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
    int ret = spin_set_nonblocking (fd);

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

    ret = epoll_ctl (spin_global.epollfd, EPOLL_CTL_ADD, fd, &event);

    if (ret == -1) {
        free (srv);
        return NULL;
    }

    spin_loop_wait_event (loop, &srv->in_task);

    return srv;
}

static void spin_tcp_server_end_destroy (spin_task_t task)
{
    spin_poll_target_t pt = CAST_TASK_TO_POLL_TARGET (task);
    spin_tcp_server_t srv = SPIN_DOWNCAST (struct __spin_tcp_server,
                                           poll_target, pt);
    close (srv->fd);
    free (srv);
}


int spin_tcp_server_destroy (spin_tcp_server_t srv)
{
    /* TODO: Not implemented */
    if (srv == NULL) {
        errno = EINVAL;
        return -1;
    }

    epoll_ctl (spin_global.epollfd, EPOLL_CTL_DEL, srv->fd, NULL);
    if (!link_node_is_dettached (&srv->in_task.l)) {
        if (spin_poll_target_test_event (&srv->poll_target, EPOLLIN))
            link_list_dettach (&srv->poll_target.loop->nexttask,
                               &srv->in_task.l);
        else
            link_list_dettach (&srv->poll_target.loop->polltask,
                               &srv->in_task.l);
    }

    spin_poll_target_begin_destroy (&srv->poll_target,
                                    spin_tcp_server_end_destroy);

    return 0;
}
