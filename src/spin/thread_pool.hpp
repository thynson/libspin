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

#ifndef __SPIN_THREAD_POOL_HPP__
#define __SPIN_THREAD_POOL_HPP__


#include <spin/singleton.hpp>
#include <spin/spin_lock.hpp>
#include <spin/routine.hpp>

#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace spin
{


  class __SPIN_EXPORT__ thread_pool : public singleton<thread_pool>
  {
  public:

    constexpr static bool volatility = true;

    thread_pool(singleton_tag);

    ~thread_pool() noexcept;

    void enqueue(routine<> task);

    void enqueue(std::list<routine<>> &tasks);

    void wait();

    unsigned get_current_idel_thread_count() const
    { return m_idle_thread_count; }

    unsigned get_max_thread_count() const
    { return m_max_thread_count; }

  private:

    void thread_routine();

    class managered_thread : public std::thread
    {
    public:
      managered_thread(thread_pool &pool);

      ~managered_thread() noexcept;
    };

    unsigned m_max_thread_count;
    std::atomic<unsigned> m_idle_thread_count;
    std::atomic_bool m_exited;
    std::mutex m_mutex;
    std::condition_variable m_task;
    std::condition_variable m_idle;
    spin_lock m_pending_task_lock;

    std::list<routine<>> m_pending_tasks;
    std::list<routine<>> m_queued_tasks;
    std::list<managered_thread> m_threads;
  };
}

#endif
