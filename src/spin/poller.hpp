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
#include "enable_singleton.hpp"
#include "handle.hpp"
#include <bitset>

namespace spin
{

  class poller : public enable_singleton<poller, true>
  {
  public:

    enum
    {
      POLL_READABLE,
      POLL_WRITABLE,
      POLL_ERROR,
      MAX_POLL
    };

    typedef std::bitset<MAX_POLL> poll_state;

    class context
    {
      friend class poller;
    public:
      context(main_loop &loop);

      virtual ~context() noexcept;

      virtual void poll_state_changed(poll_state ps) = 0;

      void change_poll_state(poll_state ps);

      main_loop &get_main_loop() const noexcept
      { return m_main_loop; }

    private:
      main_loop &m_main_loop;
      std::shared_ptr<poller> m_poller;
      spin_lock m_lock;
      poll_state m_current_state;
      poll_state m_polled_state;
      main_loop::task m_poster;
      main_loop::task m_dispatcher;
    };

    static singleton_factory get_instance;

  protected:
    poller();
  private:
    void poll() noexcept;

    handle m_poller;
    handle m_exit_notifier;
    std::thread m_thread;
  };
}

#endif
