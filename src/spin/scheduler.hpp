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

#include <spin/task.hpp>
#include <spin/system.hpp>
#include <spin/spin_lock.hpp>
#include <spin/utils.hpp>
#include <spin/event_monitor.hpp>

#include <mutex>
#include <memory>

namespace spin
{
  /**
   * @brief scheduler schedules task execution and event handling
   */
  class __SPIN_EXPORT__ scheduler
  {
  public:

    /** @brief Default constructor */
    scheduler();

    /** @brief Default destructor */
    ~scheduler() = default;

    scheduler(const scheduler &) = delete;

    scheduler(scheduler &&) = delete;

    scheduler &operator = (scheduler &&) = delete;

    scheduler &operator = (const scheduler &) = delete;

    /** @brief Execute scheduled tasks and waiting for events */
    void run();

    /**
     * @brief Dispatch a task
     * @param t The task to be executed
     * @note This function does not ensure thread safety, if a task is created
     * in another thread rather than the one where scheduler is running,
     * consider #post
     * @see #post
     */
    void dispatch(task &t) noexcept
    { m_dispatched_queue.push_back(t); }

    /**
     * @brief Dispatch a batch of tasks
     * @param q The queue of tasks to be executed
     * @note This function does not ensure thread safety, if a task is created
     * in another thread rather than the one where scheduler is running,
     * consider #post
     * @see #post
     */
    void dispatch(task::queue_type q) noexcept
    { m_dispatched_queue.splice(m_dispatched_queue.end(), std::move(q)); }

    /**
     * @brief Post a task to scheduler
     * @param t The task to be executed
     * @note This function ensures thread safety, if a task is is created in a
     * thread where the scheduler is running, consider use #dispatch
     * @see #dispatch
     */
    void post(task &t) noexcept
    {
      std::lock_guard<spin_lock> guard(m_lock);
      m_posted_queue.push_back(t);
      interrupt();
    }

    /**
     * @brief Post a batch of tasks to scheduler
     * @param q The queue of tasks to be executed
     * @note This function ensures thread safety, if a task is is created in a
     * thread where the scheduler is running, consider use #dispatch
     * @see #dispatch
     */
    void post(task::queue_type q) noexcept
    {
      std::lock_guard<spin_lock> guard(m_lock);
      m_posted_queue.splice(m_posted_queue.end(), std::move(q));
      interrupt();
    }

    /**
     * @brief Interrupt the scheduler from another thread
     *
     * This function is useful when you want to wakeup a scheduler from
     * blocking state from another thread, calling interrupt in the thread
     * where scheduler is running is meaningless.
     */
    void interrupt();

    /**
     * @brief Test if this scheduler has tasks to execute
     * @note A task is being executed after it has been unqueued from the
     * scheduler, so this function will return false for the last task of this
     * scheduler
     */
    bool has_tasks() noexcept
    {
      bool ret = !m_dispatched_queue.empty();
      if (ret) return ret;
      std::lock_guard<spin_lock> guard(m_lock);
      return !m_posted_queue.empty();
    }

    std::shared_ptr<event_monitor> get_event_monitor();

  private:

    std::weak_ptr<event_monitor> m_event_monitor_ptr;
    task::queue_type m_dispatched_queue;
    task::queue_type m_posted_queue;
    spin_lock m_lock;
  };
}

#endif
