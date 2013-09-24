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
       * @brief Default constructor, left the task handler unspcecified
       */
      task() noexcept
        : m_node()
        , m_handler()
      { }

      /**
       * @brief Constuct task object with the handler initialized with an
       * std::function<void()> object
       */
      task(std::function<void()> handler)
        : m_node()
        , m_handler(std::move(handler))
      { }

      /** @brief Move constructor */
      task(task &&c) noexcept
        : m_node()
        , m_handler(std::move(c.m_handler))
      { m_node.swap_nodes(c.m_node); }

      /** @brief Copy constructor is forbidden */
      task(const task &) = delete;

      /**
       * @brief Provide a new handler
       * @return Return the old handler function
       */
      std::function<void()> set_handler(std::function<void()> handler) noexcept
      {
        std::function<void()> tmp(std::move(m_handler));
        m_handler = std::move(handler);
        return tmp;
      }

      /**
       * @brief Cancel this task, if the task have not been posted to
       * an main_loop, or the handler've already been called, this function
       * does nothing.
       * @retval true Actually cancel a posted task
       * @retval false Nothing has been done
       */
      bool cancel() noexcept
      {
        if (m_node.is_linked())
        {
          m_node.unlink();
          return true;
        }
        return false;
      }

    private:
      intrusive_list_node m_node;
      std::function<void()> m_handler;
    };

    /**
     * @brief timed task object that can be posted to or defered with an
     * main_loop and its handler will be called in the future when specified
     * time just come or the future or will be called immediately if the
     * specified time has passed.
     */
    class __SPIN_EXPORT__ timed_task
    {
      friend class main_loop;
    public:

      /**
       * @brief Default constructor that specify the alarm time to right now
       * and left the task handler unspecified
       */
      timed_task() noexcept
        : m_task()
        , m_node()
        , m_time_point()
      { }

      /**
       * @brief Constructor that specify the alarm time but left the task
       * handler unspecified
       */
      timed_task(time::steady_time_point tp)
        : m_task()
        , m_node()
        , m_time_point(std::move(tp))
      { }

      /**
       * @brief Constructor that specify the alarm time and specify the
       * task handler
       */
      timed_task(time::steady_time_point tp, std::function<void()> handler)
        : m_task(std::move(handler))
        , m_node()
        , m_time_point(std::move(tp))
      { }

      /** @brief Move constructor */
      timed_task(timed_task &&t) noexcept
        : m_task(std::move(t.m_task))
        , m_node()
        , m_time_point(std::move(t.m_time_point))
      { m_node.swap_nodes(t.m_node); }

      /** @brief Copy constructor is forbidden */
      timed_task(const timed_task &) = delete;

      /** @brief Compare the specified alarm time of two timed_task */
      friend bool operator < (const timed_task &lhs,
                              const timed_task &rhs) noexcept
      { return lhs.m_time_point < rhs.m_time_point; }

      /** @brief Compare the specified alarm time of two timed_task */
      friend bool operator > (const timed_task &lhs,
                              const timed_task &rhs) noexcept
      { return lhs.m_time_point > rhs.m_time_point; }

      /** @brief Get the alarm time */
      const time::steady_time_point &get_time_point() const noexcept
      { return m_time_point; }

      /** @brief Cancel this timed_task and reset the alarm time */
      time::steady_time_point
      reset_time_point(const time::steady_time_point &tp) noexcept
      {
        time::steady_time_point ret = m_time_point;
        cancel();
        m_time_point = tp;
        return ret;
      }

      /** @brief Reset the handler */
      std::function<void()> set_handler(std::function<void()> handler) noexcept
      { return m_task.set_handler(std::move(handler)); }

      /** @brief Cancel this timed_task, see task::cancel() */
      bool cancel() noexcept
      {
        if (m_node.is_linked())
        {
          m_node.unlink();
          return true;
        }
        else
        { return m_task.cancel(); }
      }

    private:
      task m_task;
      intrusive_set_node m_node;
      time::steady_time_point m_time_point;
    };

    /** @brief intrusive list container of task */
    typedef intrusive_list<task, &task::m_node> task_list;

    /** @brief priority queue of timed_task that implemented by intrusive
     *   multiset */
    typedef intrusive_multiset<timed_task, &timed_task::m_node>
      timed_task_queue;

    /** @brief constructor */
    main_loop() noexcept;

    /** @brief destructor */
    ~main_loop() noexcept;

    void run();

    /** @brief defer a task to next round of loop
     *  @note this function should not be used across thread
     *  @see #post */
    void defer(task &t) noexcept
    { m_defered_tasks.push_back(t); }

    /** @brief defer a timed task to until its alarm time
     *  @note this function should not be used across thread
     *  @see #post */
    void defer(timed_task &t) noexcept
    { m_timed_task_queue.insert(t); }

    /** @brief post a task that will executed
     *  @note this function ensure cross thread safety
     *  @see #defer */
    void post(task &cb) noexcept;

    /** @brief post a timed task that will be executed at its specified time
     *  @note this function ensure cross thread safety
     *  @see #defer */
    void post(timed_task &cb) noexcept;

  private:

    task_list wait_for_events();

    main_loop(const main_loop &) = delete;
    main_loop &operator = (const main_loop &) = delete;
    main_loop(main_loop &&) = delete;
    main_loop &operator = (main_loop &&) = delete;

    /** @brief Default instance */
    static main_loop default_instance;

    timed_task_queue m_timed_task_queue;
    task_list m_posted_tasks;
    task_list m_defered_tasks;
    std::mutex m_lock;
    std::condition_variable m_cond;
    std::atomic_size_t m_ref_counter;
  };


}

#endif
