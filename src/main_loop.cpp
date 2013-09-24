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

#include "spin/main_loop.hpp"
#include <boost/iterator/transform_iterator.hpp>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace spin {

  main_loop main_loop::default_instance;

  main_loop::main_loop() noexcept
    : m_timed_task_queue()
    , m_defered_tasks()
    , m_posted_tasks()
    , m_lock()
    , m_cond()
    , m_ref_counter()
  { }

  main_loop::~main_loop() noexcept
  { }

  main_loop::task_list main_loop::wait_for_events()
  {
    main_loop::task_list tasks;
    tasks.swap(m_defered_tasks);

    if (m_timed_task_queue.empty())
    {
      std::unique_lock<std::mutex> guard(m_lock);
      if (m_posted_tasks.empty())
      {
        if (tasks.empty() && m_ref_counter != 0)
        {
          // No timer, just wait for other event
          do
            m_cond.wait(guard);
          while (m_posted_tasks.empty());
        }
      }
    }
    else
    {
      auto tp = m_timed_task_queue.begin()->m_time_point;
      std::unique_lock<std::mutex> guard(m_lock);

      if (m_posted_tasks.empty())
      {
        if (tasks.empty())
        {
          // There are timers, wait until the first expire time point
          do
          {
            std::cv_status status = m_cond.wait_until(guard, tp);
            if (status == std::cv_status::timeout)
            {
              auto get_task = [](timed_task &t) -> task &
              { return t.m_task; };

              typedef boost::transform_iterator<decltype(get_task),
                    decltype(m_timed_task_queue.begin())> tranform_iterator;

              // Insert all timer event that have same time point with tp and
              // remove them from loop.m_timer_event_set
              auto tf = m_timed_task_queue.begin();
              auto te = m_timed_task_queue.upper_bound(*tf);

              tranform_iterator f(tf, get_task);
              tranform_iterator e(tf, get_task);

              tasks.insert(tasks.end(), f, e);
              m_timed_task_queue.erase(tf, te);

              return tasks;
            }
          }
          while (m_timed_task_queue.empty());
        }
      }
    }
    tasks.splice(tasks.end(), m_posted_tasks);
    return tasks;
  }

  void main_loop::run()
  {
    for ( ; ; )
    {
      task_list tasks = wait_for_events();
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

  void main_loop::post(task &cb) noexcept
  {
    std::unique_lock<std::mutex> guard(m_lock);
    if (m_posted_tasks.empty())
      m_cond.notify_one();
    m_posted_tasks.push_back(cb);
  }

  void main_loop::post(timed_task &cb) noexcept
  {
    std::unique_lock<std::mutex> guard(m_lock);
    if (m_timed_task_queue.empty())
      m_cond.notify_one();
    m_timed_task_queue.insert(cb);
  }
}
