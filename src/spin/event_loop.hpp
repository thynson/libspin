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

#ifndef __SPIN_LOOP_HPP_INCLUDED__
#define __SPIN_LOOP_HPP_INCLUDED__

#include <thread>
#include <condition_variable>
#include <memory>
#include <chrono>
#include "utils.hpp"

namespace spin {

    class callback
    {
      friend class event_loop;
    public:

      callback()
        : m_node()
        , m_handler()
      { }

      callback(const std::function<void()> &handler)
        : m_node()
        , m_handler(handler)
      { }

      callback(std::function<void()> &&handler)
        : m_node()
        , m_handler(std::move(handler))
      { }

      callback(callback &&c)
        : callback(std::move(c.m_handler))
      { m_node.swap_nodes(c.m_node); }

      callback(const callback &) = delete;

      std::function<void()> set_handler (const std::function<void()> &handler)
      {
        std::function<void()> tmp(std::move(m_handler));
        m_handler = handler;
        return tmp;
      }

    private:
      list_node m_node;
      std::function<void()> m_handler;
    };

    class timed_callback : public callback
    {
      friend class event_loop;
    public:

      timed_callback(time_point tp)
        : callback()
        , m_node()
        , m_time_point(tp)
      { }

      timed_callback(time_point tp, std::function<void()> &&handler)
        : callback(std::forward<std::function<void()>>(handler))
        , m_node()
        , m_time_point(tp)
      { }

      timed_callback(time_point tp, const std::function<void()> &handler)
        : callback(handler)
        , m_node()
        , m_time_point(tp)
      { }

      friend bool operator < (const timed_callback &lhs,
                              const timed_callback &rhs)
      { return lhs.m_time_point < rhs.m_time_point; }

      friend bool operator > (const timed_callback &lhs,
                              const timed_callback &rhs)
      { return lhs.m_time_point > rhs.m_time_point; }

    private:
      set_node m_node;
      time_point m_time_point;
    };

  class __SPIN_EXPORT__ event_loop
  {
    class __SPIN_INTERNAL__ poller;
  public:


    typedef list<callback, &callback::m_node> callback_list;
    typedef multiset<timed_callback, &timed_callback::m_node>
      timed_callback_set;

    static bool cancel(callback &cb);
    static bool cancel(timed_callback &cb);

    event_loop();
    ~event_loop();
    void run();

    void post(callback &cb);
    void post(timed_callback &cb);


  private:

    event_loop(const event_loop &) = delete;
    event_loop &operator = (const event_loop &) = delete;
    event_loop(event_loop &&) = delete;
    event_loop &operator = (event_loop &&) = delete;

    timed_callback_set m_timed_callbacks;
    callback_list m_upcoming_callbacks;
    callback_list m_notified_callbacks;
    callback_list m_io_event_list;

    std::shared_ptr<poller> m_poller;

  };

}

#endif
