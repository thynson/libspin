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

#include <spin/event_source.hpp>

#include <sys/epoll.h>

namespace spin
{

  event_source::event_source(scheduler &schd, system_handle device)
    : m_monitor(schd.get_event_monitor())
    , m_device(std::move(device))
    , m_callback([this](int events)
        {
          if (events & EPOLLERR)
            this->on_error();
          if (events & EPOLLIN)
            this->on_emit();
        })
  {
    ::epoll_event epev;
    epev.events = EPOLLIN | EPOLLET;
    epev.data.ptr = &m_callback;

    int result = ::epoll_ctl(m_monitor->m_monitor.get_raw_handle(),
        EPOLL_CTL_ADD, m_device.get_raw_handle(), &epev);

    if (result == -1)
      throw_exception_for_last_error();
  }

  void event_source::on_emit() noexcept{ }

  void event_source::on_error() noexcept {}

  io_event_source::io_event_source(scheduler &schd, system_handle device, readonly_t)
    : io_event_source(schd, std::move(device), EPOLLIN | EPOLLET)
  { }

  io_event_source::io_event_source(scheduler &schd, system_handle device, writeonly_t)
    : io_event_source(schd, std::move(device), EPOLLOUT | EPOLLET)
  { }

  io_event_source::io_event_source(scheduler &schd, system_handle device, readwrite_t)
    : io_event_source(schd, std::move(device), EPOLLIN | EPOLLOUT | EPOLLET)
  { }

  io_event_source::io_event_source(scheduler &schd, system_handle device, int events)
    : m_monitor(schd.get_event_monitor())
    , m_device(std::move(device))
    , m_callback([this](int events)
        {
          if (events & EPOLLERR)
            this->on_error();

          if (events & EPOLLIN)
            this->on_readable();

          if (events & EPOLLOUT)
            this->on_writable();
        })
  {
    ::epoll_event epev;
    epev.events = events;
    epev.data.ptr = &m_callback;


    int result = ::epoll_ctl(m_monitor->m_monitor.get_raw_handle(),
        EPOLL_CTL_ADD, m_device.get_raw_handle(), &epev);

    if (result == -1)
      throw_exception_for_last_error();
  }

  void io_event_source::on_readable() noexcept{ }

  void io_event_source::on_writable() noexcept{ }

  void io_event_source::on_error() noexcept {}
}
