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

#include "spin/event_loop.hpp"
#include <boost/iterator/transform_iterator.hpp>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace spin {

  event_loop::event_loop()
    : m_timed_callbacks()
    , m_upcoming_callbacks()
    , m_notified_callbacks()
    , m_ref_counter(0)
    , m_notifier_lock()
    , m_condition_variable()
  { }

  event_loop::~event_loop()
  { }

  event_loop::callback_list event_loop::wait_for_events()
  {
    event_loop::callback_list tasks;
    tasks.swap(m_upcoming_callbacks);

    if (m_timed_callbacks.empty()) {
      unique_lock guard(m_notifier_lock);
      if (m_notified_callbacks.empty()) {
        if (tasks.empty() && m_ref_counter != 0) {
          // No timer, just wait for other event
          do
            m_condition_variable.wait(guard);
          while (m_notified_callbacks.empty());
        }
      }
    } else {
      auto tp = m_timed_callbacks.begin()->m_time_point;
      unique_lock guard(m_notifier_lock);
      if (m_notified_callbacks.empty()) {
        if (tasks.empty()) {
          // There are timers, wait until the first expire time point
          do {
            std::cv_status status = m_condition_variable.wait_until(guard, tp);
            if (status == std::cv_status::timeout) {
              auto get_callback = [](timed_callback &t)->callback&
              { return t.m_callback; };

              typedef boost::transform_iterator<decltype(get_callback),
                    decltype(m_timed_callbacks.begin())> tranform_iterator;
              // Insert all timer event that have same time point with tp and
              // remove them from loop.m_timer_event_set
              auto tf = m_timed_callbacks.begin();
              auto te = m_timed_callbacks.upper_bound(*tf);
              tranform_iterator f(tf, get_callback);
              tranform_iterator e(tf, get_callback);
              tasks.insert(tasks.end(), f, e);
              m_timed_callbacks.erase(tf, te);
              return tasks;
            }
          } while (m_timed_callbacks.empty());
        }
      }
    }
    tasks.splice(tasks.end(), m_notified_callbacks);
    return tasks;
  }

  void event_loop::run()
  {
    for ( ; ; )
    {
      callback_list tasks = wait_for_events();
      if (tasks.empty())
        return;
      while (!tasks.empty())
      {
        auto x = tasks.begin();
        tasks.erase(x);
        x->m_handler();
      }
    }
  }

  void event_loop::dispatch(callback &ap)
  { m_upcoming_callbacks.push_back(ap); }

  void event_loop::dispatch(timed_callback &dp)
  { m_timed_callbacks.push_back(dp); }

  void event_loop::dispatch(callback_list &cblist)
  { cblist.splice(cblist.end(), m_upcoming_callbacks); }

  void event_loop::dispatch(callback_list &&cblist)
  { dispatch(cblist); }

  void event_loop::post(callback &cb)
  {
    unique_lock guard(m_notifier_lock);
    if (m_notified_callbacks.empty())
      m_condition_variable.notify_one();
    m_notified_callbacks.push_back(cb);
  }

  void event_loop::post(callback_list &cb)
  {
    unique_lock guard(m_notifier_lock);
    if (m_notified_callbacks.empty())
      m_condition_variable.notify_one();
    cb.splice(cb.end(), m_notified_callbacks);
  }

  void event_loop::post(callback_list &&cb)
  { post(cb); }

}
