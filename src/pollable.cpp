/*
 * Copyright (C) 2014 LAN Xingcan
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

#include <spin/pollable.hpp>

#include <sys/epoll.h>

namespace spin
{

  namespace
  {
    void register_pollable(poll_handler &target, system_raw_handle epollfd,
        system_raw_handle targetfd, int events)
    {
      ::epoll_event evt;
      evt.data.ptr = &target;
      evt.events = events | EPOLLET;

      int result = epoll_ctl(epollfd,
          EPOLL_CTL_ADD, targetfd, &evt);

      if (result == -1)
        throw_exception_for_last_error();
    }
  }

  pollable::pollable(scheduler &el, system_handle handle,
      pollable::poll_argument_readable_t)
    : poll_handler()
    , m_poller(el.get_poller())
    , m_handle(std::move(handle))
  {
    if (!m_handle)
      throw std::logic_error("handle is invalid for pollable object");

    register_pollable(*this, m_poller->get_poll_handle().get_raw_handle(),
        m_handle.get_raw_handle(), EPOLLIN);
  }

  pollable::pollable(scheduler &el, system_handle handle,
      pollable::poll_argument_writable_t)
    : poll_handler()
    , m_poller(el.get_poller())
    , m_handle(std::move(handle))
  {
    if (!m_handle)
      throw std::logic_error("handle is invalid for pollable object");

    register_pollable(*this, m_poller->get_poll_handle().get_raw_handle(),
        m_handle.get_raw_handle(), EPOLLOUT);
  }

  pollable::pollable(scheduler &el, system_handle handle,
      pollable::poll_argument_duplex_t)
    : poll_handler()
    , m_poller(el.get_poller())
    , m_handle(std::move(handle))
  {
    if (!m_handle)
      throw std::logic_error("handle is invalid for pollable object");

    register_pollable(*this, m_poller->get_poll_handle().get_raw_handle(),
        m_handle.get_raw_handle(), EPOLLIN | EPOLLOUT);
  }

  pollable::pollable(std::shared_ptr<poller> p, system_handle handle,
      pollable::poll_argument_readable_t)
    : poll_handler()
    , m_poller(std::move(p))
    , m_handle(std::move(handle))
  {
    if (!m_handle)
      throw std::logic_error("handle is invalid for pollable object");

    register_pollable(*this, m_poller->get_poll_handle().get_raw_handle(),
        m_handle.get_raw_handle(), EPOLLIN);
  }

  pollable::pollable(std::shared_ptr<poller> p, system_handle handle,
      pollable::poll_argument_writable_t)
    : poll_handler()
    , m_poller(std::move(p))
    , m_handle(std::move(handle))
  {
    if (!m_handle)
      throw std::logic_error("handle is invalid for pollable object");

    register_pollable(*this, m_poller->get_poll_handle().get_raw_handle(),
        m_handle.get_raw_handle(), EPOLLOUT);
  }

  pollable::pollable(std::shared_ptr<poller> p, system_handle handle,
      pollable::poll_argument_duplex_t)
    : poll_handler()
    , m_poller(std::move(p))
    , m_handle(std::move(handle))
  {
    if (!m_handle)
      throw std::logic_error("handle is invalid for pollable object");

    register_pollable(*this, m_poller->get_poll_handle().get_raw_handle(),
        m_handle.get_raw_handle(), EPOLLIN | EPOLLOUT);
  }
}
