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
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>


namespace spin
{

  // @brief Implement detail of stream_socket_peer
  class stream_socket_peer::detail : public poller::context
  {
  public:
    detail(main_loop &loop, handle &socket)
      : poller::context(loop, socket)
    { }

    virtual ~detail() override = default;

    void poll_state_changed(poller::poll_state ps) override
    {  }
  private:
  };


  stream_socket_peer::stream_socket_peer(main_loop &loop, handle socket)
    : m_handle(std::move(socket))
    , m_detail(std::unique_ptr<detail>(new detail(loop, m_handle)))
  {
  }


  void stream_socket_peer::read(char buff[], size_t size,
      std::function<void(size_t size)> cb)
  {
  }

  void stream_socket_peer::write(const char buff[], size_t size,
      std::function<void(size_t size)> cb)
  {
  }


  class stream_socket_listener::detail : public poller::context
  {
  public:
    detail(main_loop &loop, handle &h)
      : poller::context(loop, h)
    {}

    void poll_state_changed(poller::poll_state ps) override
    { }

  };

  stream_socket_listener::stream_socket_listener(main_loop &loop, handle &h)
    : m_handle (std::move(h))
    , m_detail (std::unique_ptr<detail>(new detail(loop, h)))
  {
  }


  void stream_socket_listener::accept(
      std::function<void(std::unique_ptr<stream_socket_peer>)> cb)
  {
  }


}
