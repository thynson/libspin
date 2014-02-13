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


  class basic_pollable
  {
  protected:
    virtual ~basic_pollable() = 0;
    virtual void on_readable() = 0;
    virtual void on_writable() = 0;
    virtual void on_error() = 0;
  };

  class pollable : public basic_pollable
                 , public intruse::list_node<pollable>
  {
    friend class poller;
  protected:

    struct poll_argument_readable_t {  } poll_argument_readable;
    struct poll_argument_writable_t {  } poll_argument_writable;
    struct poll_argument_duplex_t   {  } poll_argument_duplex;

    pollable(poller &p, system_handle handle, poll_argument_readable_t);

    pollable(poller &p, system_handle handle, poll_argument_writable_t);

    pollable(poller &p, system_handle handle, poll_argument_duplex_t);

    virtual void on_readable() { };
    virtual void on_writable() { };
    virtual void on_error() { };

    const system_handle &get_handle() const noexcept
    { return m_handle; }

  private:
    std::shared_ptr<poller> m_poller;
    system_handle m_handle;
  };

  class poller : public std::enable_shared_from_this<poller>
               , private basic_pollable
  {
  public:

    poller();

    virtual ~poller() override;

    system_handle &get_poll_handle() noexcept
    { return m_poll_handle; }

    void interrupt();

    void poll(bool allow_blocking);

  private:

    void on_readable() override { }
    void on_writable() override { }
    void on_error() override { }

    system_handle m_poll_handle;
    system_handle m_interrupter;
  };
}

#endif
