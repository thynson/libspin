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

#ifndef __SPIN_SOCKET_H__
#define __SPIN_SOCKET_H__

#include "config.h"
#include "loop.h"
#include "stream.h"
#include <sys/socket.h>

typedef struct __spin_tcp_server *spin_tcp_server_t;

/**
 * @brief Connect to a server
 * @param loop The loop to handle this socket
 * @param addr The address of the server
 * @param callback The callback to be called when connected
 */
int __SPIN_EXPORT__
spin_tcp_connect (spin_loop_t loop, const struct sockaddr_storage *addr,
                  void (*callback) (spin_stream_t socket));

/**
 * @brief Create a tcp server
 * @param loop The loop to handle this server
 * @param fd The listening socket fd, should be bind to an address first
 * @param connected The callback to be called when a client is connected
 */
spin_tcp_server_t __SPIN_EXPORT__
spin_tcp_server_from_fd (spin_loop_t loop, int fd,
                         void (*connected) (spin_stream_t,
                                            const struct sockaddr_storage *));

/**
 * @brief Close a tcp server
 * @param srv The server to be closed
 */
int spin_tcp_server_destroy (spin_tcp_server_t srv);


#endif
