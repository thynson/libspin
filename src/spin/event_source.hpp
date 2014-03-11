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

#ifndef __SPIN_EVENT_SOURCE_HPP_INCLUDED__
#define __SPIN_EVENT_SOURCE_HPP_INCLUDED__

#include <spin/scheduler.hpp>

#include <memory>

namespace spin
{

  class __SPIN_EXPORT__ event_source
  {
    friend class event_monitor;
  public:
    event_source (const event_source &) = delete;
    event_source (event_source &&) = delete;
    event_source &operator = (const event_source &) = delete;
    event_source &operator = (event_source &&) = delete;

    const system_handle &get_device() const
    { return m_device; }
  protected:
    event_source(scheduler &schd, system_handle event_object);
    virtual ~event_source() = default;
    virtual void on_emit() noexcept;
    virtual void on_error() noexcept;
  private:
    std::shared_ptr<event_monitor> m_monitor;
    system_handle m_device;
    std::function<void(int)> m_callback;
  };

  class __SPIN_EXPORT__ io_event_source
  {
    friend class event_monitor;
  public:
    struct readonly_t {} readonly;
    struct writeonly_t {} writeonly;
    struct readwrite_t {} readwrite;

    io_event_source (const io_event_source &) = delete;
    io_event_source (io_event_source &&) = delete;

    io_event_source &operator = (const io_event_source &) = delete;
    io_event_source &operator = (io_event_source &&) = delete;

    const system_handle &get_device() const
    { return m_device; }
  protected:
    io_event_source(scheduler &schd, system_handle device, readonly_t);
    io_event_source(scheduler &schd, system_handle device, writeonly_t);
    io_event_source(scheduler &schd, system_handle device, readwrite_t);

    virtual ~io_event_source() = default;
    virtual void on_readable() noexcept;
    virtual void on_writable() noexcept;
    virtual void on_error() noexcept;

  private:
    io_event_source(scheduler &schd, system_handle device, int events);

    std::shared_ptr<event_monitor> m_monitor;
    system_handle m_device;
    std::function<void(int)> m_callback;
  };

}
#endif
