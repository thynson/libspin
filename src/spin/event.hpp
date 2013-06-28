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

#ifndef __SPIN_EVENT_HPP_INCLUDED__
#define __SPIN_EVENT_HPP_INCLUDED__

#include "utils.hpp"

namespace spin {

  class __SPIN_EXPORT__ event : public list_node<event>
  {
  public:
    virtual ~event() {}
    virtual void callback() = 0;
  };

  class __SPIN_EXPORT__ timer_event : public event
                                    , public set_node<timer_event>
  {
  public:
    timer_event(const time_point &tp) : m_tp(tp) {}

    timer_event(time_point &&tp) : m_tp(tp) {}
    virtual ~timer_event() {}

    void cancel()
    { set_node<timer_event>::unlink(); }

    friend bool operator < (const timer_event &lhs, const timer_event &rhs)
    { return lhs.m_tp < rhs.m_tp; }

    friend bool operator > (const timer_event &lhs, const timer_event &rhs)
    { return lhs.m_tp > rhs.m_tp; }

    const time_point &get_time_point()
    { return m_tp; }

  private:
    time_point m_tp;
  };

  class __SPIN_EXPORT__ io_event : public event
                                 , public list_node<io_event>
  {
  };
}
#endif
