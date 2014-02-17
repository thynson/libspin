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

#include <spin/poll.hpp>

#include <stdexcept>
#include <cassert>

#include <sys/epoll.h>
#include <sys/eventfd.h>

namespace spin
{

  namespace
  {
    void register_pollable(basic_pollable &target, system_raw_handle epollfd,
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

  basic_pollable::~basic_pollable() = default;

  pollable::pollable(std::shared_ptr<poller> p, system_handle handle,
      pollable::poll_argument_readable_t)
    : basic_pollable()
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
    : basic_pollable()
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
    : basic_pollable()
    , m_poller(std::move(p))
    , m_handle(std::move(handle))
  {
    if (!m_handle)
      throw std::logic_error("handle is invalid for pollable object");

    register_pollable(*this, m_poller->get_poll_handle().get_raw_handle(),
        m_handle.get_raw_handle(), EPOLLIN | EPOLLOUT);
  }

  poller::poller()
    : m_poll_handle{ epoll_create1, EPOLL_CLOEXEC }
    , m_interrupter{ eventfd, 0, EFD_NONBLOCK | EFD_CLOEXEC }
  {
    register_pollable(*this, m_poll_handle.get_raw_handle(),
        m_interrupter.get_raw_handle(), EPOLLIN);
  }

  poller::~poller() = default;

  void poller::interrupt()
  {
    eventfd_write(m_interrupter.get_raw_handle(), 1);
  }

  void poller::poll(bool allow_blocking)
  {
    int nfds;
    do
    {
      std::array<epoll_event, 1024> evarray;
      nfds = epoll_wait(m_poll_handle.get_raw_handle(), evarray.data(),
          evarray.size(), allow_blocking ? -1 : 0);

      if (nfds == -1)
      {
        int tmperrno = 0;
        std::swap(tmperrno, errno);
        assert (tmperrno == EINTR);
        continue;
      }
      else
      {
        assert ((decltype(evarray)::size_type) nfds <= evarray.size());

        for (auto i = evarray.begin(); i != evarray.begin() + nfds; ++i)
        {
          basic_pollable *p = reinterpret_cast<basic_pollable *>(i->data.ptr);

          if (i->events & EPOLLERR)
            p->on_error();

          if (i->events & EPOLLIN)
            p->on_readable();

          if (i->events & EPOLLOUT)
            p->on_writable();
        }
      }
    }
    while(nfds == 1024);
  }

}
