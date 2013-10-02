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

#include <spin/main_loop.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <mutex>
#include <condition_variable>

namespace spin
{

  bool main_loop::task::cancel() noexcept
  {
    if (m_node.is_linked())
    {
      m_node.unlink();
      return true;
    }
    return false;
  }

  main_loop::deadline_timer::deadline_timer(main_loop &loop,
      std::function<void()> proc, time::steady_time_point deadline,
      bool check_deadline) noexcept
    : m_main_loop(&loop)
    , m_task(std::move(proc))
    , m_node()
    , m_deadline(std::move(deadline))
  {
    if (!check_deadline || m_deadline > decltype(m_deadline)::clock::now())
      m_main_loop->m_deadline_timer_queue.insert(*this);
  }

  main_loop::deadline_timer::deadline_timer(
      main_loop::deadline_timer &&t) noexcept
    : m_task(std::move(t.m_task))
    , m_node()
    , m_deadline(std::move(t.m_deadline))
  { m_node.swap_nodes(t.m_node); }

  time::steady_time_point
  main_loop::deadline_timer::reset_deadline(time::steady_time_point deadline,
      bool check_deadline) noexcept
  {
    m_node.unlink();
    auto &q = m_main_loop->m_deadline_timer_queue;
    std::swap(deadline, m_deadline);
    if (!check_deadline || m_deadline > decltype(m_deadline)::clock::now())
      q.insert(*this);
    return deadline;
  }

  main_loop::main_loop() noexcept
    : m_deadline_timer_queue()
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

    if (m_deadline_timer_queue.empty())
    {
      std::unique_lock<std::mutex> guard(m_lock);
      if (m_posted_tasks.empty())
      {
        if (tasks.empty() && m_ref_counter != 0)
        { m_cond.wait(guard); }
      }
      tasks.splice(tasks.end(), m_posted_tasks);
    }
    else
    {
      auto tp = m_deadline_timer_queue.begin()->m_deadline;
      std::unique_lock<std::mutex> guard(m_lock);

      if (m_posted_tasks.empty() && tasks.empty())
      {
        // There are timers, wait until the first expire time point
        std::cv_status status = m_cond.wait_until(guard, tp);
        if (status == std::cv_status::timeout)
        {
          auto get_task = [](deadline_timer &t) -> task &
          { return t.m_task; };

          typedef boost::transform_iterator<decltype(get_task),
                decltype(m_deadline_timer_queue.begin())> tranform_iterator;

          // Insert all timer event that have same time point with tp and
          // remove them from loop.m_timer_event_set
          auto tf = m_deadline_timer_queue.begin();
          auto te = m_deadline_timer_queue.upper_bound(*tf);

          tranform_iterator f(tf, get_task);
          tranform_iterator e(te, get_task);

          tasks.insert(tasks.end(), f, e);
          m_deadline_timer_queue.erase(tf, te);

          return tasks;
        }
      }
      tasks.splice(tasks.end(), m_posted_tasks);
    }
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
        (*x)(); // Invoke it
      }
    }
  }

  void main_loop::post(main_loop::task &t) noexcept
  {
    std::unique_lock<std::mutex> guard(m_lock);
    m_posted_tasks.push_back(t);
    m_cond.notify_one();
  }

  void main_loop::post(main_loop::task_list &tl) noexcept
  {
    std::unique_lock<std::mutex> guard(m_lock);
    m_posted_tasks.splice(m_posted_tasks.end(), tl);
    m_cond.notify_one();
  }
}
