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

#ifndef __SPIN_EVENT_MONITOR_HPP_INCLUDED__
#define __SPIN_EVENT_MONITOR_HPP_INCLUDED__

#include <spin/system.hpp>
#include <spin/routine.hpp>

namespace spin
{

  class __SPIN_EXPORT__ event_monitor
  {
    friend class io_event_source;
    friend class event_source;
  public:
    event_monitor();

    ~event_monitor() = default;

    void interrupt();

    void wait(bool allow_blocking);

  private:

    routine<int> m_interrupt_callback;
    system_handle m_interrupter;
    system_handle m_monitor;
  };

}

#endif
