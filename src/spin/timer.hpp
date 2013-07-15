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

#ifndef __SPIN_TIMER_HPP_INCLUDED__
#define __SPIN_TIMER_HPP_INCLUDED__

#include "event_loop.hpp"

namespace spin {

  class __SPIN_EXPORT__ timer
  {
  public:

    timer(event_loop &loop, const time_point &initial,
        const time_duration &interval, std::function<void()> &&handler,
        bool started = true)
      : m_event_loop(loop)
      , m_interval(interval)
      , m_callback_aux(handler)
      , m_callback(calculate_time_point(initial, interval),
          std::bind(std::move(on_timed_out), this))
      , m_started(started)
    {
      if (m_started)
        m_event_loop.post(m_callback);
    }

    timer(event_loop &loop, const time_duration &interval,
        std::function<void()> &&handler, bool started = true)
      : timer(loop, time_point(interval), interval, std::move(handler),
          started)
    { }

    timer(event_loop &loop, const time_point &initial,
        const time_duration &interval, const std::function<void()> &handler,
        bool started = true)
      : timer(loop, initial, interval, std::function<void()>(handler),
          started)
    { }

    timer(event_loop &loop, const time_duration &interval,
        const std::function<void()> &handler, bool started)
      : timer(loop, time_point(interval), interval,
          std::function<void()>(handler), started)
    { }

    ~timer();

    bool start()
    {
      bool actual_started = !m_started;
      m_started = true;
      if (actual_started)
        m_event_loop.post(m_callback);
      return actual_started;
    }

    bool stop()
    {
      bool actual_stopped = !!m_started;
      m_started = false;
      if (actual_stopped)
        m_callback.cancel();
      return actual_stopped;
    }

    std::pair<time_point, time_duration>
    reset(const time_point &initial, const time_duration &interval
        , bool started = true)
    {
      stop();
      auto ret = std::make_pair(m_callback.get_time_point(), m_interval);
      m_interval = interval;
      m_callback.reset_time_point(initial);
      if (started)
        start();
      return ret;
    }

    std::pair<time_point, time_duration>
    reset(const time_duration &interval, bool started = true)
    {
      stop();
      auto ret = std::make_pair(m_callback.get_time_point(), m_interval);
      m_interval = interval;
      m_callback.reset_time_point(time_point(interval));
      if (started)
        start();
      return ret;
    }

  private:

    static inline time_point
    calculate_time_point(const time_point &initial,
                         const time_duration &interval)
    {
      time_point tp = time_point::clock::now();
      if (initial >= tp)
        return tp;
      else
        return tp + (initial - tp) % interval;
    }

    static inline void on_timed_out(timer *t)
    {
      if (t->m_interval != time_duration::zero())
      {
        auto tp = t->m_callback.get_time_point() + t->m_interval;
        t->m_callback.reset_time_point(tp);
        t->m_event_loop.post(t->m_callback);
      }
      t->m_callback_aux();
    }

    event_loop &m_event_loop;
    time_duration m_interval;
    std::function<void()> m_callback_aux;
    timed_callback m_callback;
    bool m_started;
  };
}
#endif
