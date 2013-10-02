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
      : poller::context(loop, socket,
          (1 << poller::POLL_WRITABLE)
          | (1 << poller::POLL_READABLE)
          | (1 << poller::POLL_ERROR))
    { }

    virtual ~detail() override = default;

    void on_poll_event(poller::poll_flag ps) override
    {  }
  private:
  };


  stream_socket_peer::stream_socket_peer(main_loop &loop, handle socket)
    : m_handle(std::move(socket))
    , m_detail(std::unique_ptr<detail>(new detail(loop, m_handle)))
  {
  }

  stream_socket_peer::~stream_socket_peer() noexcept
  {
    auto guard = m_detail->get_poller().lock();
    m_handle.close();
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
      : poller::context(loop, h,
            (1 << poller::POLL_READABLE) | (1 << poller::POLL_ERROR))
      , m_callback()
      , m_accept_task(std::bind(&detail::do_accept, this))
    {}

    void on_poll_event(poller::poll_flag ps) override
    {
      if (ps[poller::POLL_READABLE])
      {
        do_accept();
      }
    }

    void do_accept()
    {
      if (m_callback)
      {
        sockaddr_storage addr;
        socklen_t len = sizeof(addr);
        int fd = ::accept(get_handle().get_handle(),
            reinterpret_cast<sockaddr*>(&addr), &len);
        if (fd != -1)
        {
          m_callback(std::unique_ptr<stream_socket_peer>(
              new stream_socket_peer(get_main_loop(), handle(fd))));
          get_main_loop().dispatch(m_accept_task);
        }
        else
        {
          if (errno == EAGAIN) {
            errno = 0;
            change_poll_flag(poller::POLL_READABLE);
          }
        }
      }
    }

    std::function<void(std::unique_ptr<stream_socket_peer>)> m_callback;
    main_loop::task m_accept_task;
  };

  stream_socket_listener::stream_socket_listener(main_loop &loop, handle h)
    : m_handle (std::move(h))
    , m_detail (std::unique_ptr<detail>(new detail(loop, m_handle)))
  {
  }

  stream_socket_listener::~stream_socket_listener()
  {
    auto guard = m_detail->get_poller().lock();
    m_handle.close();
  }


  std::function<void(std::unique_ptr<stream_socket_peer>)>
  stream_socket_listener::accept(
      std::function<void(std::unique_ptr<stream_socket_peer>)> cb)
  {
    std::swap(m_detail->m_callback, cb);
    if (cb == nullptr)
      m_detail->do_accept();
    return cb;
  }


}
