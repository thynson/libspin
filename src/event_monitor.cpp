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

#include <spin/event_monitor.hpp>

#include <sys/eventfd.h>
#include <sys/epoll.h>

namespace spin
{


  event_monitor::event_monitor()
    : m_interrupt_callback([] (int) {})
    , m_interrupter { eventfd, 0, EFD_NONBLOCK }
    , m_monitor { epoll_create1, EPOLL_CLOEXEC }
  {

    ::epoll_event epev;
    epev.events = EPOLLET | EPOLLIN;
    epev.data.ptr = &m_interrupt_callback;
    int result = ::epoll_ctl(m_monitor.get_raw_handle(),
        EPOLL_CTL_ADD, m_interrupter.get_raw_handle(), &epev);
    if (result == -1)
      throw_exception_for_last_error();
  }

  void event_monitor::interrupt()
  {
    eventfd_t efd = 1;
    int result = ::eventfd_write(m_interrupter.get_raw_handle(), efd);
    if (result == -1)
      throw_exception_for_last_error();
  }

  void event_monitor::wait(bool allow_blocking)
  {
    std::array<::epoll_event, 128> evarray;
    int timeout = allow_blocking ? -1 : 0;
    int result = ::epoll_wait(m_monitor.get_raw_handle(),
        evarray.data(), evarray.size(), timeout);

    if (result == -1)
    {
      if (errno == EINTR)
      {
        errno = 0;
        return ;
      }
      else
        throw_exception_for_last_error();
    }

    for (int i = 0; i < result; i++)
    {
      const std::function<void(int)> *pfunc
        = reinterpret_cast<const std::function<void(int)>*>(evarray[i].data.ptr);
      (*pfunc)(evarray[i].events);
    }
  }
}
