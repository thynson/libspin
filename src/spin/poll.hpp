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

#ifndef __SPIN_POLL_HPP_INCLUDED__
#define __SPIN_POLL_HPP_INCLUDED__

#include <spin/system.hpp>
#include <spin/intruse/list.hpp>

#include <memory>

namespace spin
{
  class poller;


  /**
   * @brief Abstract pollable interface
   */
  class __SPIN_EXPORT__ poll_handler
  {
    friend class poller;
  public:
    poll_handler(poll_handler&&) = delete;
    poll_handler(const poll_handler&) = delete;
  protected:
    poll_handler() = default;
    virtual ~poll_handler() = 0;
    virtual void on_readable() noexcept = 0;
    virtual void on_writable() noexcept = 0;
    virtual void on_error() noexcept = 0;
  };

  class __SPIN_EXPORT__ poller : private poll_handler
  {
  public:

    poller();

    ~poller() = default;

    system_handle &get_poll_handle() noexcept
    { return m_poll_handle; }

    void interrupt();

    void poll(bool allow_blocking);

  private:

    void on_readable() noexcept override { }

    void on_writable() noexcept override { }

    void on_error() noexcept override { }

    system_handle m_poll_handle;
    system_handle m_interrupter;
  };
}

#endif
