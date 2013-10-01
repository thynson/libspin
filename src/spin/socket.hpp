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

#ifndef __SPIN_SOCKET_HPP_INCLUDED__
#define __SPIN_SOCKET_HPP_INCLUDED__

#include "poller.hpp"

namespace spin
{
  class stream_socket_peer
  {
    friend class stream_socket_server;
  public:
    ~stream_socket_peer();

    stream_socket_peer(stream_socket_peer &&peer) noexcept = default;
    stream_socket_peer &operator = (stream_socket_peer &&peer) noexcept = default;

    stream_socket_peer(const stream_socket_peer &peer) = delete;
    stream_socket_peer &operator = (const stream_socket_peer &peer) = delete;

    stream_socket_peer(main_loop &loop, handle socket);

    void read(char buff[], size_t size, std::function<void(size_t)> cb);

    void write(const char buff[], size_t size, std::function<void(size_t)> cb);

  private:
    class detail;
    handle m_handle;
    std::unique_ptr<detail> m_detail;;
  };



  class stream_socket_listener
  {
  public:
    stream_socket_listener(main_loop &loop, handle h);
    ~stream_socket_listener();

    std::function<void(std::unique_ptr<stream_socket_peer>)>
    accept(std::function<void(std::unique_ptr<stream_socket_peer>)> cb);
  protected:

    class detail;
    handle m_handle;
    std::unique_ptr<detail> m_detail;
  };

}

#endif
