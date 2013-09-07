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
     * @brief callback object that can be post to event loop and its handler
     * will be called immediately
     */
    class __SPIN_EXPORT__ callback
    {
      friend class main_loop;
    public:

      /**
       * @brief Default constructor, left the callback handler unspcecified
       */
      callback()
        : m_node()
        , m_handler()
      { }

      /**
       * @brief Constuct callback object with the handler initialized with an
       * lvalue of std::function<void()> object
       */
      callback(std::function<void()> handler)
        : m_node()
        , m_handler(std::move(handler))
      { }

      /** @brief Move constructor */
      callback(callback &&c)
        : m_node()
        , m_handler(std::move(c.m_handler))
      { m_node.swap_nodes(c.m_node); }

      /** @brief Copy constructor is forbidden */
      callback(const callback &) = delete;

      /**
       * @brief Provide a new handler
       * @return Return the old handler function
       */
      std::function<void()> set_handler(std::function<void()> handler)
      {
        std::function<void()> tmp(std::move(m_handler));
        m_handler = std::move(handler);
        return tmp;
      }

      /**
       * @brief Cancel this callback, if the callback have not been posted to
       * an main_loop, or the handler've already been called, this function
       * does nothing.
       * @retval true Actually cancel a posted callback
       * @retval false Nothing has been done
       */
      bool cancel()
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
     * @brief timed callback object that can be post to an main_loop and its
     * handler will be called in the future when specified time just come or
     * the future or will be called immediately if the specified time has
     * passed.
     */
    class __SPIN_EXPORT__ timed_callback
    {
      friend class main_loop;
    public:

      /**
       * @brief Default constructor that specify the alarm time to right now
       * and left the callback handler unspecified
       */
      timed_callback()
        : m_callback()
        , m_node()
        , m_time_point()
      { }

      /**
       * @brief Constructor that specify the alarm time but left the callback
       * handler unspecified
       */
      timed_callback(time::steady_time_point tp)
        : m_callback()
        , m_node()
        , m_time_point(std::move(tp))
      { }

      /**
       * @brief Constructor that specify the alarm time and specify the
       * callback handler
       */
      timed_callback(time::steady_time_point tp, std::function<void()> handler)
        : m_callback(std::move(handler))
        , m_node()
        , m_time_point(std::move(tp))
      { }

      /** @brief Move constructor */
      timed_callback(timed_callback &&t)
        : m_callback(std::move(t.m_callback))
        , m_node()
        , m_time_point(std::move(t.m_time_point))
      { m_node.swap_nodes(t.m_node); }

      /** @brief Copy constructor is forbidden */
      timed_callback(const timed_callback &) = delete;

      /** @brief Compare the specified alarm time of two timed_callback */
      friend bool operator < (const timed_callback &lhs,
                              const timed_callback &rhs)
      { return lhs.m_time_point < rhs.m_time_point; }

      /** @brief Compare the specified alarm time of two timed_callback */
      friend bool operator > (const timed_callback &lhs,
                              const timed_callback &rhs)
      { return lhs.m_time_point > rhs.m_time_point; }

      /** @brief Get the alarm time */
      const time::steady_time_point &get_time_point() const
      { return m_time_point; }

      /** @brief Cancel this timed_callback and reset the alarm time */
      time::steady_time_point
      reset_time_point(const time::steady_time_point &tp)
      {
        time::steady_time_point ret = m_time_point;
        cancel();
        m_time_point = tp;
        return ret;
      }

      /** @brief Reset the handler */
      std::function<void()> set_handler(std::function<void()> handler)
      { return m_callback.set_handler(std::move(handler)); }

      /** @brief Cancel this timed_callback, see callback::cancel() */
      bool cancel()
      {
        if (m_node.is_linked())
        {
          m_node.unlink();
          return true;
        }
        else
        { return m_callback.cancel(); }
      }

    private:
      callback m_callback;
      intrusive_set_node m_node;
      time::steady_time_point m_time_point;
    };

    typedef intrusive_list<callback, &callback::m_node> callback_list;
    typedef intrusive_multiset<timed_callback, &timed_callback::m_node>
      timed_callback_set;

    main_loop();
    ~main_loop();

    void run();

    void defer(callback &cb)
    { m_defered_callbacks.push_back(cb); }

    void defer(timed_callback &cb)
    { m_timed_callbacks.insert(cb); }

    void defer(callback_list &cblist)
    { cblist.splice(cblist.end(), m_defered_callbacks); }

    void defer(callback_list &&cblist)
    { defer(cblist); }

    void post(callback &cb);
    void post(timed_callback &cb);
    void post(callback_list &cblist);
    void post(callback_list &&cblist);

  private:

    callback_list wait_for_events();

    main_loop(const main_loop &) = delete;
    main_loop &operator = (const main_loop &) = delete;
    main_loop(main_loop &&) = delete;
    main_loop &operator = (main_loop &&) = delete;

    /** @brief Default instance */
    static main_loop default_instance;

    timed_callback_set m_timed_callbacks;
    callback_list m_posted_callbacks;
    callback_list m_defered_callbacks;
    std::mutex m_lock;
    std::condition_variable m_cond;
    std::atomic_size_t m_ref_counter;
  };


}

#endif
