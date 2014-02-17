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

  poll_handler::~poll_handler() = default;

  poller::poller()
    : m_poll_handle{ epoll_create1, EPOLL_CLOEXEC }
    , m_interrupter{ eventfd, 0, EFD_NONBLOCK | EFD_CLOEXEC }
  {
    ::epoll_event epev;
    epev.events = EPOLLIN | EPOLLET;
    epev.data.ptr = this;
    int ret = epoll_ctl (m_poll_handle.get_raw_handle(),
        EPOLL_CTL_ADD, m_interrupter.get_raw_handle(), &epev);
    if (ret != 0)
      throw_exception_for_last_error();
  }

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
          poll_handler *p = reinterpret_cast<poll_handler *>(i->data.ptr);

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
