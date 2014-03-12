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

#include <spin/timer.hpp>
#include <spin/transform_iterator.hpp>

#include <stdexcept>

#include <sys/timerfd.h>
#include <sys/epoll.h>

namespace spin
{
  using namespace std::chrono;
  namespace
  {
    template<typename Clock>
    struct clock_spec;

    template<>
    struct clock_spec<std::chrono::steady_clock>
    {
      static system_handle create_device()
      {
        return system_handle(timerfd_create, CLOCK_MONOTONIC, TFD_NONBLOCK);
      }
    };

    template<>
    struct clock_spec<std::chrono::system_clock>
    {
      static system_handle create_device()
      {
        return system_handle(timerfd_create, CLOCK_REALTIME, TFD_NONBLOCK);
      }
    };

    /**
     * @brief Check whether interval, throws exception if interval is a
     * nagative time duration.
     */
    template<typename Clock>
    typename timer<Clock>::duration
    check_interval(typename timer<Clock>::duration &interval)
    {
      if (interval < timer<Clock>::duration::zero())
        throw std::invalid_argument("interval cannot less than zero");
      return std::move(interval);
    }

    /**
     * @brief Adjust @p tp according to @p base_time and @p duration
     * @returns An interger value suitable to be added to missed counter
     */
    template<typename Clock>
    auto adjust_time_point(typename timer<Clock>::time_point &tp,
        const typename timer<Clock>::time_point &base_time,
        const typename timer<Clock>::duration &duration)
      noexcept -> decltype((base_time - tp) / duration)
    {
      if (duration == timer<Clock>::duration::zero())
        return 0;
      if (tp >= base_time)
      {
        return 0;
      }
      else
      {
        auto d = base_time - tp;
        auto ret = d / duration;
        tp += (ret + 1) * duration;
        return ret;
      }

    }

    /** @brief Set alarm time for @p timerfd */
    template<typename Clock>
    void update_timerfd(const system_handle &timerfd,
        typename timer<Clock>::duration duration)
    {
      auto secs = duration_cast<seconds>(duration).count();
      duration -= seconds(secs);
      auto nanosecs = duration_cast<nanoseconds>(duration).count();
      itimerspec itspc {
        { 0, 0 },
        { secs, nanosecs}
      };

      int result = timerfd_settime(timerfd.get_raw_handle(), TFD_TIMER_ABSTIME,
          &itspc, nullptr);
      if (result == -1)
        throw_exception_for_last_error();
    }

  }

  template<typename Clock>
  timer_service<Clock>::timer_service(scheduler &schd)
    : std::enable_shared_from_this<timer_service>()
    , intruse::rbtree_node<scheduler *, timer_service>(&schd)
    , event_source(schd, clock_spec<Clock>::create_device())
    , m_deadline_timer_queue()
  { }

  template<typename Clock>
  timer_service<Clock>::~timer_service() = default;

  template<typename Clock>
  void timer_service<Clock>::on_emit() noexcept
  {
    auto now = timer::clock::now();
    task::queue_type l;

    while (!m_deadline_timer_queue.empty()
        && timer::get_index(m_deadline_timer_queue.front()) <= now)
    {
      auto &t = m_deadline_timer_queue.front();
      l.push_back(t.m_task);
      t.relay(now);
    }

    scheduler *el = timer_service::get_index(*this);
    el->dispatch(std::move(l));

    if (!m_deadline_timer_queue.empty())
      update_timerfd<Clock>(get_device(),
          timer::get_index(m_deadline_timer_queue.front()).time_since_epoch());

  }

  template<typename Clock>
  void timer_service<Clock>::enqueue(timer &t) noexcept
  {
    m_deadline_timer_queue.insert(t, intruse::policy_backmost);
    if (&m_deadline_timer_queue.front() == &t)
      update_wakeup_time();
  }

  template<typename Clock>
  void timer_service<Clock>::update_wakeup_time() noexcept
  {
    auto &t = m_deadline_timer_queue.front();
    update_timerfd<Clock>(get_device(), timer::get_index(t).time_since_epoch());
  }

  template<typename Clock>
  timer<Clock>::timer(timer_service &service, routine<> procedure,
      typename timer<Clock>::duration interval)
    : timer(service, std::move(procedure),
        interval == duration::zero() ? time_point::min() : clock::now() + interval,
        std::move(interval))
  { }

  template<typename Clock>
  timer<Clock>::timer(scheduler &loop, routine<> procedure,
      typename timer<Clock>::duration interval)
    : timer(loop, std::move(procedure),
        interval == duration::zero() ? time_point::min() : clock::now() + interval,
        std::move(interval))
  { }

  template<typename Clock>
  timer<Clock>::timer(timer_service &service, routine<> procedure,
      typename timer<Clock>::time_point tp,
      typename timer<Clock>::duration interval)
    : intruse::rbtree_node<typename Clock::time_point, timer>(std::move(tp))
    , m_scheduler(service.get_scheduler())
    , m_interval(check_interval<Clock>(interval))
    , m_task(std::move(procedure))
    , m_missed_counter()
    , m_timer_service(nullptr)
  { start(); }

  template<typename Clock>
  timer<Clock>::timer(scheduler &loop, routine<> procedure,
      typename timer<Clock>::time_point tp,
      typename timer<Clock>::duration interval)
    : intruse::rbtree_node<typename Clock::time_point, timer>(std::move(tp))
    , m_scheduler(loop)
    , m_interval(check_interval<Clock>(interval))
    , m_task(std::move(procedure))
    , m_missed_counter()
    , m_timer_service(nullptr)
  { start(); }

  template<typename Clock>
  timer<Clock>::~timer() = default;

  template<typename Clock>
  std::tuple<typename Clock::time_point, typename Clock::duration, std::uint64_t>
  timer<Clock>::reset(
      typename timer::time_point initial,
      typename timer::duration interval)
  {
    auto missed_counter = reset_missed_counter();
    auto now = Clock::now();

    bool will_stop = initial == time_point::min() && interval == duration::zero();

    if (will_stop)
    {
      auto ret = std::make_tuple(timer::get_index(*this),
          std::move(m_interval), missed_counter);
      if (timer::template is_linked<void>(*this))
        timer::unlink(*this);
      timer::update_index(*this, std::move(initial), intruse::policy_backmost);
      m_interval = std::move(interval);
      return ret;
    }

    m_missed_counter = adjust_time_point<Clock>(initial, now, interval);

    auto ret = std::make_tuple(timer::get_index(*this),
        std::move(m_interval), missed_counter);

    bool stopped = m_timer_service == nullptr;

    if (stopped)
    {
      assert (!timer::template is_linked<void>(*this)); // timer should not in the queue
      assert(m_interval == Clock::duration::zero()); // interval should be zero
      assert(timer::get_index(*this) == time_point::min());

      timer::update_index(*this, std::move(initial), intruse::policy_backmost);
      m_interval = std::move(interval);
      start();
    }
    else
    {
      assert (timer::template is_linked<void>(*this)); // timer should still in the queue
      assert (!m_timer_service->m_deadline_timer_queue.empty());

      auto &front = m_timer_service->m_deadline_timer_queue.front();
      bool needs_update = (&front == this || initial < std::get<0>(ret));
      timer::update_index(*this, std::move(initial), intruse::policy_backmost);
      m_interval = std::move(interval);
      if (needs_update)
        m_timer_service->update_wakeup_time();
    }
    return ret;
  }

  template<typename Clock>
  std::tuple<typename Clock::time_point, typename Clock::duration, std::uint64_t>
  timer<Clock>::reset(typename timer::duration interval)
  { return reset(get_time_point(), interval); }

  template<typename Clock>
  std::tuple<typename Clock::time_point, typename Clock::duration, std::uint64_t>
  timer<Clock>::stop()
  { return reset(time_point::min(), duration::zero()); }

  /**
   * @brief Start a timer
   */
  template<typename Clock>
  void timer<Clock>::start()
  {
    if (timer::get_index(*this) == time_point::min()
        && m_interval == duration::zero())
      // Don't start the timer
      return;

    if (!m_timer_service)
      m_timer_service = timer_service::get(m_scheduler);

    // Client code should ensure t is greater than or equals to clock::now()
    m_timer_service->enqueue(*this);
  }

  template<typename Clock>
  void timer<Clock>::relay(const time_point &now)
  {
    if (m_interval == timer::duration::zero())
    {
      // Stop the timer
      // We must do unlink first, if we release a shared_ptr first, which
      // would result in its destructor called, this timer may be unlink
      // twice. And second call to unlink will result in crash, althrouh we
      // can test if this timer is still linked, do unlink first without
      // checking must be right.
      timer::unlink(*this);
      timer::update_index(*this, time_point::min());
      m_timer_service = nullptr;
    }
    else
    {
      auto next_tp = timer::get_index(*this);
      m_missed_counter += adjust_time_point<Clock>(next_tp, now, m_interval);
      timer::update_index(*this, std::move(next_tp), intruse::policy_backmost);
    }
  }

  template class timer<std::chrono::steady_clock>;
  template class timer<std::chrono::system_clock>;

  template class timer_service<std::chrono::steady_clock>;
  template class timer_service<std::chrono::system_clock>;


}
