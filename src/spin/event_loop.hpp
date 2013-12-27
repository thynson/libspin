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
#include <bitset>
#include <spin/utils.hpp>

namespace spin
{

  class __SPIN_EXPORT__ event_loop
  {
  public:

    /**
     * @brief task object that can be post to event loop and its handler
     * will be called immediately
     */
    class __SPIN_EXPORT__ task : public intruse::list_node<task>
    {
      friend class event_loop;
    public:

      /**
       * @brief Default constructor
       */
      task() noexcept = default;

      /**
       * @brief Constuct task object with the specified task procedure
       * @param proc The procedure for this task
       */
      task(std::function<void()> proc) noexcept
        : list_node()
        , m_proc(std::move(proc))
      { }

      /** @brief Move constructor */
      task(task &&) = default;

      /** @brief Assign operator */
      task &operator = (task && rvalue) = default;

      /** @brief Copy constructor is forbidden */
      task(const task &) = delete;

      /** @brief Copy by lvalue is forbidden */
      task &operator = (const task &) = delete;

      /** @brief Destructor */
      ~task() = default;

      /**
       * @brief Provide a new handler
       * @return Return the old handler function
       */
      std::function<void()> set_proc(std::function<void()> proc) noexcept
      {
        std::swap(m_proc, proc);
        return proc;
      }

      /** @Brief Execute this task */
      void operator () ()
      { if (m_proc) m_proc(); }

      /** @breif Test if this task is under dispatching */
      bool is_dispatching() const noexcept
      { return list_node<task>::is_linked(*this); }

      /**
       * @brief Cancel this task, if the task have not been dispatched to a
       * event_loop, or this task already finished, this function does nothing.
       * @retval true Actually cancel a posted task
       * @retval false Nothing has been done
       */
      bool cancel() noexcept;

    private:
      std::function<void()> m_proc;
    };

    /**
     * @brief deadline timer can be posted to or defered with an
     * event_loop and its handler will be called in the future when specified
     * time just come or the future or will be called immediately if the
     * specified time has passed.
     */
    class __SPIN_EXPORT__ deadline_timer
      : public intruse::rbtree_node<time::steady_time_point, deadline_timer>
    {
      friend class event_loop;
    public:

      /**
       * @brief Constructor that specify the alarm time and specify the
       * task handler
       * @param loop The main loop that this task attached to
       * @param proc The procedure to be executed when deadline come
       * @param deadline The deadline time point of this timer
       * @param check_deadline Check if specified deadline has already expired
       * and in that situation the proc will not be dispatch to execute
       */
      deadline_timer(event_loop &loop, std::function<void()> proc,
          time::steady_time_point tp, bool check_deadline = false) noexcept;

      /** @brief Move constructor */
      deadline_timer(deadline_timer &&t) noexcept;

      /** @brief Copy constructor is forbidden */
      deadline_timer(const deadline_timer &) = delete;

      /** @brief Destructor */
      ~deadline_timer() = default;

      /** @brief Get the alarm time */
      const time::steady_time_point &get_deadline() const noexcept
      { return rbtree_node::get_index(*this); }

      /** @brief Get the main loop this timer attached to */
      event_loop &get_event_loop() const noexcept
      { return m_event_loop; }

      /** @brief Get the task object */
      const task &get_task() const noexcept
      { return m_task; }

      /** @brief Test if this timer is expired */
      bool is_expired() const noexcept
      { return rbtree_node::is_linked(*this); }

      /**
       * @brief Cancel this deadline_timer and reset the deadline time
       * @param deadline The new deadline
       * @param check_deadline Same effect with constructor
       * @returns The original deadline
       */
      time::steady_time_point reset_deadline(time::steady_time_point deadline,
          bool check_deadline = false) noexcept;

      /** @brief Reset the handler */
      std::function<void()> set_proc(std::function<void()> proc) noexcept
      { return m_task.set_proc(std::move(proc)); }

    private:
      event_loop &m_event_loop;
      task m_task;
    };

    /** @brief intrusive list container of task */
    using task_queue = intruse::list<task>;

    /** @brief priority queue of deadline_timer that implemented by intrusive
     *   multiset */
    using deadline_timer_queue
      = intruse::rbtree<time::steady_time_point, deadline_timer>;

    /** @brief constructor */
    event_loop() noexcept;

    /** @brief destructor */
    ~event_loop() noexcept;

    void run();

    /**
     * @brief Dispatch a task to this main loop and it will be executed at
     * next round
     * @note This function should not be used across thread, instead, use
     * #post
     * @see #post
     */
    void dispatch(task &t) noexcept
    { m_defered_tasks.push_back(t); }


    /**
     * @brief Dispatch tasks batchly to this main loop and they will be
     * executed at next round
     * @note This function should not be used across thread, instead, use
     * #post
     * @see #post
     */
    void dispatch(task_queue &tl) noexcept
    { m_defered_tasks.splice(m_defered_tasks.end(), tl); }

    /**
     * @brief Post a task to this main loop and it will be executed
     * @note This function ensure cross-thread safety
     * @see #dispatch
     */
    void post(task &t) noexcept;

    /**
     * @brief Post a task to this main loop and it will be executed
     * @note This function ensure cross-thread safety
     * @see #dispatch
     */
    void post(task_queue &t) noexcept;

    template<typename Proc>
    task set_task(Proc &&proc) noexcept
    {
      task t(std::forward<Proc>(proc));
      dispatch(t);
      return t;
    }


    void ref() noexcept
    { m_ref_counter++; }

    void unref() noexcept
    { m_ref_counter--; }

  private:

    task_queue wait_for_events();

    event_loop(const event_loop &) = delete;
    event_loop &operator = (const event_loop &) = delete;
    event_loop(event_loop &&) = delete;
    event_loop &operator = (event_loop &&) = delete;

    deadline_timer_queue m_deadline_timer_queue;
    task_queue m_posted_tasks;
    task_queue m_defered_tasks;
    std::mutex m_lock;
    std::condition_variable m_cond;
    std::atomic_size_t m_ref_counter;
  };
}

#endif
