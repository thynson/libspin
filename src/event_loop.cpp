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

#include <spin/event_loop.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <mutex>
#include <condition_variable>

namespace spin
{

  bool event_loop::task::cancel() noexcept
  {
    if (list_node::is_linked(*this))
    {
      list_node::unlink(*this);
      return true;
    }
    return false;
  }

  event_loop::deadline_timer::deadline_timer(event_loop &loop,
      std::function<void()> proc, time::steady_time_point deadline,
      bool check_deadline) noexcept
    : rbtree_node(std::move(deadline))
    , m_event_loop(loop)
    , m_task(std::move(proc))
  {
    auto &dl = rbtree_node::get_index(*this);
    if (!check_deadline || dl > decltype(deadline)::clock::now())
      m_event_loop.m_deadline_timer_queue.insert(*this);
  }

  event_loop::deadline_timer::deadline_timer(
      event_loop::deadline_timer &&t) noexcept
    : rbtree_node(std::move(t))
    , m_event_loop(t.m_event_loop)
    , m_task(std::move(t.m_task))
  { }

  time::steady_time_point
  event_loop::deadline_timer::reset_deadline(time::steady_time_point deadline,
      bool check_deadline) noexcept
  {
    //rbtree_node::unlink(*this);
    //auto &q = m_event_loop.m_deadline_timer_queue;
    deadline = rbtree_node::update_index(*this, std::move(deadline),
        intruse::policy_backmost);

    if (check_deadline
        && rbtree_node::get_index(*this) > decltype(deadline)::clock::now()
        && rbtree_node::is_linked(*this))
      rbtree_node::unlink(*this);
    return deadline;
  }

  event_loop::event_loop() noexcept
    : m_deadline_timer_queue()
    , m_posted_tasks()
    , m_defered_tasks()
    , m_lock()
    , m_cond()
    , m_ref_counter()
  { }

  event_loop::~event_loop() noexcept
  { }

  event_loop::task_queue event_loop::wait_for_events()
  {
    task_queue tasks;
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
      auto tp = deadline_timer::get_index(m_deadline_timer_queue.front());
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
          auto te = m_deadline_timer_queue.upper_bound(tf, *tf);

          tranform_iterator f(tf, get_task);
          tranform_iterator e(te, get_task);

          tasks.insert(tasks.end(), f, e);

          //tasks.insert(tasks.end(), f, e);
          m_deadline_timer_queue.erase(tf, te);

          return tasks;
        }
      }
      tasks.splice(tasks.end(), m_posted_tasks);
    }
    return tasks;
  }

  void event_loop::run()
  {
    for ( ; ; )
    {
      task_queue q = wait_for_events();
      if (q.empty())
        return;
      while (!q.empty())
      {
        auto &t = q.front();
        intruse::list_node<task>::unlink(t);
        t();
      }
    }
  }

  void event_loop::post(event_loop::task &t) noexcept
  {
    std::unique_lock<std::mutex> guard(m_lock);
    m_posted_tasks.push_back(t);
    m_cond.notify_one();
  }

  void event_loop::post(event_loop::task_queue &tl) noexcept
  {
    if (tl.empty())
      return;
    std::unique_lock<std::mutex> guard(m_lock);
    m_posted_tasks.splice(m_posted_tasks.end(), tl);
    m_cond.notify_one();
  }
}
