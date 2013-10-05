/*
 * Copyright (C) 2013 LAN Xingcan
 * All right reserved
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

#include <spin/socket.hpp>
#include <netinet/in.h>
#include <iostream>

using namespace spin;
using namespace std;

int main ()
{
  handle h { socket, PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0 };
  if (!h)
    throw std::system_error(errno, std::system_category());

  sockaddr_in inaddr;
  inaddr.sin_family = AF_INET;
  inaddr.sin_port = htons(1333);
  inaddr.sin_addr.s_addr = INADDR_ANY;
  socklen_t addrlen = sizeof(inaddr);

  int value = 1;
  if (::setsockopt(h.get_os_handle(), SOL_SOCKET, SO_REUSEPORT, &value,
        sizeof(value)) == -1)
    throw std::system_error(errno, std::system_category());


  if (::bind(h.get_os_handle(), reinterpret_cast<sockaddr*>(&inaddr), addrlen) != 0)
    throw std::system_error(errno, std::system_category());

  if (::listen(h.get_os_handle(), SOMAXCONN) == -1)
    throw std::system_error(errno, std::system_category());

  main_loop loop;
  stream_socket_listener listener(loop, std::move(h));
  listener.accept([](std::unique_ptr<stream_socket_peer> peer)
      {
        cout << "connected" << endl;
      });

  loop.run();
  return EXIT_FAILURE;
}
