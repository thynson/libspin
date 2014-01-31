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

#include <spin/thread_pool.hpp>

namespace spin
{

  thread_pool::managered_thread::managered_thread(thread_pool &pool)
    : std::thread(std::bind(&thread_pool::routine, &pool))
  { }

  thread_pool::managered_thread::~managered_thread() noexcept
  { join(); }


  thread_pool::thread_pool(thread_pool::singleton_tag)
    : m_max_thread_count(std::thread::hardware_concurrency())
    , m_idle_thread_count(0)
    , m_exited(false)
    , m_mutex()
    , m_task()
    , m_idle()
    , m_pending_task_lock()
    , m_pending_tasks()
    , m_queued_tasks()
    , m_threads()
  {
    for (unsigned i = 0; i < m_max_thread_count; ++i)
      m_threads.emplace_back(*this);
  }

  thread_pool::~thread_pool() noexcept
  {
    m_exited = true;
    m_task.notify_all();
  }

  void thread_pool::enqueue(std::function<void()> task)
  {
    std::list<std::function<void()>> l;
    l.emplace_back(std::move(task));
    enqueue(l);
  }

  void thread_pool::enqueue(std::list<std::function<void()>> &task)
  {
    std::unique_lock<spin_lock> pending_task_guard(m_pending_task_lock);
    m_pending_tasks.splice(m_pending_tasks.end(), task);
    pending_task_guard.unlock();
    m_task.notify_all();
  }

  void thread_pool::routine()
  {
    for ( ; ; )
    {
      std::function<void()> current;
      std::unique_lock<std::mutex> thread_pool_guard(m_mutex);

      while (m_queued_tasks.empty())
      {
        if (m_exited)
          return;

        std::unique_lock<spin_lock> pending_task_guard(m_pending_task_lock);
        if (!m_pending_tasks.empty())
        {
          m_queued_tasks.splice(m_queued_tasks.end(), m_pending_tasks);
          break;
        }
        pending_task_guard.unlock();

        if (m_queued_tasks.empty())
        {
          m_idle_thread_count++;
          if (m_idle_thread_count == m_max_thread_count)
            m_idle.notify_one();
          m_task.wait(thread_pool_guard);
          m_idle_thread_count--;
        }
      }

      if (m_exited)
        return;
      current.swap(m_queued_tasks.front());
      m_queued_tasks.pop_front();
      thread_pool_guard.unlock();
      current();
    }
  }

  void thread_pool::wait()
  {
    std::unique_lock<std::mutex> thread_pool_guard(m_mutex);
    while (m_idle_thread_count != m_max_thread_count)
      m_idle.wait(thread_pool_guard);
  }
}
