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
#include "utils.hpp"

namespace spin {



  class __SPIN_EXPORT__ main_loop
  {
  public:

    /**
     * @brief task object that can be post to event loop and its handler
     * will be called immediately
     */
    class __SPIN_EXPORT__ task
    {
      friend class main_loop;
    public:

      /**
       * @brief Default constructor
       */
      task() noexcept
        : m_node()
        , m_proc()
      { }

      /**
       * @brief Constuct task object with the specified task procedure
       * @param proc The procedure for this task
       */
      task(std::function<void()> proc) noexcept
        : m_node()
        , m_proc(std::move(proc))
      { }

      /** @brief Move constructor */
      task(task &&c) noexcept
        : m_node()
        , m_proc(std::move(c.m_proc))
      { m_node.swap_nodes(c.m_node); }

      /** @brief Assign operator */
      task &operator = (task &&rvalue) noexcept
      {
        m_node.swap_nodes(rvalue.m_node);
        std::swap(m_proc, rvalue.m_proc);
        return *this;
      }

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
      { return m_node.is_linked(); }

      /**
       * @brief Cancel this task, if the task have not been dispatched to a
       * main_loop, or this task already finished, this function does nothing.
       * @retval true Actually cancel a posted task
       * @retval false Nothing has been done
       */
      bool cancel() noexcept;

    private:
      intrusive_list_node m_node;
      std::function<void()> m_proc;
    };

    /**
     * @brief deadline timer can be posted to or defered with an
     * main_loop and its handler will be called in the future when specified
     * time just come or the future or will be called immediately if the
     * specified time has passed.
     */
    class __SPIN_EXPORT__ deadline_timer
    {
      friend class main_loop;
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
      deadline_timer(main_loop &loop, std::function<void()> proc,
          time::steady_time_point tp, bool check_deadline = false) noexcept;

      /** @brief Move constructor */
      deadline_timer(deadline_timer &&t) noexcept;

      /** @brief Copy constructor is forbidden */
      deadline_timer(const deadline_timer &) = delete;

      /** @brief Destructor */
      ~deadline_timer() = default;

      /** @brief Compare the specified deadline of two deadline_timer */
      friend bool operator < (const deadline_timer &lhs,
                              const deadline_timer &rhs) noexcept
      { return lhs.m_deadline < rhs.m_deadline; }

      /** @brief Compare the specified deadline of two deadline_timer */
      friend bool operator > (const deadline_timer &lhs,
                              const deadline_timer &rhs) noexcept
      { return lhs.m_deadline > rhs.m_deadline; }

      /** @brief Get the alarm time */
      const time::steady_time_point &get_deadline() const noexcept
      { return m_deadline; }

      /** @brief Get the main loop this timer attached to */
      main_loop &get_main_loop() const noexcept
      { return m_main_loop; }

      /** @brief Get the task object */
      const task &get_task() const noexcept
      { return m_task; }

      /** @brief Test if this timer is expired */
      const bool is_expired() const noexcept
      { return !m_node.is_linked() && !m_task.is_dispatching(); }

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
      main_loop &m_main_loop;
      task m_task;
      intrusive_set_node m_node;
      time::steady_time_point m_deadline;
    };

    /** @brief intrusive list container of task */
    typedef intrusive_list<task, &task::m_node> task_list;

    /** @brief priority queue of deadline_timer that implemented by intrusive
     *   multiset */
    typedef intrusive_multiset<deadline_timer, &deadline_timer::m_node>
      deadline_timer_queue;

    /** @brief constructor */
    main_loop() noexcept;

    /** @brief destructor */
    ~main_loop() noexcept;

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
    void dispatch(task_list &tl) noexcept
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
    void post(task_list &t) noexcept;

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

    task_list wait_for_events();

    main_loop(const main_loop &) = delete;
    main_loop &operator = (const main_loop &) = delete;
    main_loop(main_loop &&) = delete;
    main_loop &operator = (main_loop &&) = delete;

    deadline_timer_queue m_deadline_timer_queue;
    task_list m_posted_tasks;
    task_list m_defered_tasks;
    std::mutex m_lock;
    std::condition_variable m_cond;
    std::atomic_size_t m_ref_counter;
  };
}

#endif
