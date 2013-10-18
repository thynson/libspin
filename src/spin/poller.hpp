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

#ifndef __SPIN_POLLER_HPP_INCLUDED__
#define __SPIN_POLLER_HPP_INCLUDED__

#include "main_loop.hpp"
#include "system.hpp"
#include "singleton.hpp"
#include <bitset>

namespace spin
{

  class poller
  {
    friend class singleton<poller>;
  public:

    enum
    {
      POLL_READABLE,
      POLL_WRITABLE,
      POLL_ERROR,
      MAX_POLL
    };

    typedef std::bitset<MAX_POLL> poll_flag;

    ~poller() noexcept;

    std::unique_lock<std::mutex> lock() noexcept
    { return std::unique_lock<std::mutex>(m_lock); }

    class context
    {
      friend class poller;
    public:
      context(main_loop &loop, system_handle &h, poll_flag flag);

      virtual ~context() noexcept;

      virtual void on_poll_event(poll_flag ps) = 0;

      void clear_poll_flag(poll_flag ps) noexcept;

      main_loop &get_main_loop() const noexcept
      { return m_main_loop; }

      poller &get_poller() const noexcept
      { return *m_poller; }

    protected:
      system_handle &get_handle() const
      { return m_handle; }

    private:
      main_loop &m_main_loop;
      system_handle &m_handle;
      std::shared_ptr<poller> m_poller;
      spin_lock m_lock;
      poll_flag m_current_state;
      poll_flag m_polled_state;
      main_loop::task m_poster;
      main_loop::task m_dispatcher;
    };

  protected:
    poller();

  private:

    void poll() noexcept;

    system_handle m_poller;
    system_handle m_exit_notifier;
    std::thread m_thread;
    std::mutex m_lock;
  };
}

#endif
