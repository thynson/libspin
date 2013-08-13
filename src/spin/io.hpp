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

#include <bitset>
#include "event_loop.hpp"
#include "event_source.hpp"

namespace spin
{

  class __SPIN_EXPORT__ io_event_source : public event_source
  {
  public:
    enum {
      EVENT_INPUT,
      EVENT_OUTPUT,
      EVENT_ERROR,
      FLAG_MAX;
    };

    typedef std::bitset<FLAG_MAX> flag;
    typedef std::function<void(flag&)> io_operator;

    io_event_source(event_loop &loop, bool enabled = false)
      : event_source(loop, enabled)
      , m_dispatcher([this]()
          {
            do_operation()
          })
      , m_state_monitor([this]()
          {
            bool will_invoke_dispatcher;
            {
              std::unique_lock<spinlock> lock_guard(m_lock);
              will_invoke_dispatcher = !m_current_state.any();
              m_current_state |= m_monitor_result;
              m_monitor_result.reset();
            }
            if (will_invoke_dispatcher)
              this->get_event_loop().dispatch(m_dispatcher);
          })
      , m_lock()
      , m_current_state()
      , m_monitor_result()
    { }

    io_event_source(io_event_source &&tmp)
      : event_source(std::move(tmp))
      , m_flag(std::move(tmp.m_flag))
    { }

    io_event_source &operator = (io_event_source &&tmp)
    {
      std::swap(*this, tmp);
      return *this;
    }

    io_event_source(const io_event_source &) = delete;

    io_event_source &operator = (const io_event_source &) = delete;

  protected:
    virtual void do_operation(flag &f);
  private:
    callback m_dispatcher;
    callback m_state_monitor;
    spinlock m_lock;
    flag m_current_state;
    flag m_monitor_result;
  };

  class __SPIN_EXPORT__ io_service
  {

  };
}
