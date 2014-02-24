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

#ifndef __SPIN_TIMER_HPP_INCLUDED__
#define __SPIN_TIMER_HPP_INCLUDED__

#include <spin/intruse/rbtree.hpp>
#include <spin/task.hpp>
#include <spin/event_loop.hpp>
#include <spin/pollable.hpp>

#include <memory>
#include <type_traits>

namespace spin
{

  template<typename Clock>
  class timer_service;

  template<typename Clock>
  class timer;

  using steady_timer_service = timer_service<std::chrono::steady_clock>;
  using system_timer_service = timer_service<std::chrono::system_clock>;

  using steady_timer = timer<std::chrono::steady_clock>;
  using system_timer = timer<std::chrono::system_clock>;

  template<typename Clock>
  class timer : public intruse::rbtree_node<typename Clock::time_point, timer<Clock>>
  {
    friend class timer_service<Clock>;
  public:

    using timer_service = ::spin::timer_service<Clock>;
    using clock = Clock;
    using time_point = typename Clock::time_point;
    using duration = typename Clock::duration;

    /**
     * @brief Create a timer from timer_service
     * @param service the timer_service to be attached to
     * @param procedure The callback function
     * @param interval The time duration between each call to procedure, if
     * it's equals to @a duration::zero(), then the timer will not start
     */
    explicit timer(timer_service &service, std::function<void()> procedure,
        duration interval = duration::zero());

    /**
     * @brief Create a timer from an event loop, attaching to an existing
     * timer_service or create an instance of timer_service if needed
     * @param loop the event loop that this timer and corresponding
     * timer_service will be attached to
     * @param procedure The callback function
     * @param interval The time duration between each call to procedure, if
     * it's equals to @a duration::zero(), then the timer will not start
     */
    explicit timer(event_loop &loop, std::function<void()> procedure,
        duration interval = duration::zero());

    /**
     * @brief Create a timer from timer_service
     * @param service the timer_service to be attached to
     * @param procedure The callback function
     * @param initial The time duration between now and the time first call to
     * procedure
     * @param interval The time duration between each call to procedure, a
     * zero interval result in one-shot behaviour
     * @note If @p initial is equals to @a time_point::min(), then the timer
     * will not start
     */
    explicit timer(timer_service &service, std::function<void()> procedure,
        time_point initial, duration interval = duration::zero());

    /**
     * @brief Create a timer from an event loop, attaching to an existing
     * timer_service or create an instance of timer_service if needed
     * @param loop the event loop that this timer and corresponding
     * timer_service will be attached to
     * @param procedure The callback function
     * @param initial The time duration between now and the time first call to
     * procedure
     * @param interval The time duration between each call to procedure, a
     * zero interval result in one-shot behaviour
     * @note If @p initial is equals to @a time_point::min(), then the timer
     * will not start
     */
    explicit timer(event_loop &loop, std::function<void()> procedure,
        time_point initial, duration interval = duration::zero());

    /** @brief Destructor */
    ~timer();

    /** @brief Get the attaching timer service */
    std::shared_ptr<timer_service> get_timer_service() const noexcept
    { return m_timer_service; }

    /**
     * @brief Reset this timer with its initial alarm time and interval
     * changed
     * @param initial The time duration between now and the next time that
     * procedure is called
     * @param interval The new time duration between each call to procedure
     * @returns Return the original time point of next time procedure is
     * called, the original timer interval and the original missed counter
     */
    std::tuple<time_point, duration, std::uint64_t>
    reset(time_point initial, duration interval = duration::zero());

    /**
     * @brief Reset this timer with its interval changed
     * @param interval The new time duration between each call to procedure
     * @returns Return the original time point of next time procedure is
     * called and the original timer interval and the original missed counter
     */
    std::tuple<time_point, duration, std::uint64_t> reset(duration interval);

    /**
     * @brief Stop the timer
     * @returns Return the original time point of next time procedure is
     * called and the original timer interval and the original missed counter
     */
    std::tuple<time_point, duration, std::uint64_t> stop();

    /** @brief Get the interval of this timer */
    const duration &get_interval() const noexcept
    { return m_interval; }

    /** @brief Get the next time point this timer alarm */
    const time_point &get_time_point() const noexcept
    { return timer::get_index(*this); }

    /** @brief Reset the callback procedure */
    std::function<void()> reset_procedure (std::function<void()> procedure);

    /** @brief Get the missed counter of this timer, and reset it to zero */
    std::uint64_t reset_missed_counter()
    {
      auto ret = m_missed_counter;
      m_missed_counter = 0;
      return ret;
    }

    /** @brief Get the missed counter of this timer */
    std::uint64_t get_missed_counter() const
    { return m_missed_counter; }

  private:

    void start();

    void relay(const time_point &now);

    event_loop &m_event_loop;
    duration m_interval;
    task m_task;
    std::uint64_t m_missed_counter;
    std::shared_ptr<timer_service> m_timer_service;
  };


  template<typename Clock>
  class timer_service :
    public std::enable_shared_from_this<timer_service<Clock>>,
    public intruse::rbtree_node<event_loop *, timer_service<Clock>>,
    public pollable
  {
  public:
    friend class timer<Clock>;
    using clock = Clock;
    using timer = ::spin::timer<Clock>;
    using time_point = typename clock::time_point;
    using duration = typename clock::duration;

    virtual ~timer_service() override;

    static std::shared_ptr<timer_service> get(event_loop &el)
    {
      auto i = instance_table.find(&el);
      if (i == instance_table.end())
      {
        std::shared_ptr<timer_service> ret;
        ret.reset(new timer_service(el));
        instance_table.insert(*ret);
        return ret;
      }
      else
        return i->shared_from_this();
    }

    event_loop &get_event_loop() noexcept
    { return *timer_service::get_index(*this); }

    const event_loop &get_event_loop() const noexcept
    { return *timer_service::get_index(*this); }

  protected:
    void on_readable() noexcept override;

  private:

    static intruse::rbtree<event_loop *, timer_service> instance_table;
    timer_service(event_loop &el);
    void enqueue(timer &t) noexcept;
    void update_wakeup_time() noexcept;
    intruse::rbtree<time_point, timer> m_deadline_timer_queue;
  };

  template<typename Clock>
  intruse::rbtree<event_loop*, timer_service<Clock>>
  timer_service<Clock>::instance_table;

  extern template class __SPIN_EXPORT__ timer<std::chrono::steady_clock>;
  extern template class __SPIN_EXPORT__ timer<std::chrono::system_clock>;
  extern template class __SPIN_EXPORT__ timer_service<std::chrono::steady_clock>;
  extern template class __SPIN_EXPORT__ timer_service<std::chrono::system_clock>;

}

#endif
