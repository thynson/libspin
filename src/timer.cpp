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
  namespace __SPIN_INTERNAL__
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

    template<typename Clock>
    typename timer<Clock>::duration
    check_interval(typename timer<Clock>::duration &interval)
    {
      if (interval < timer<Clock>::duration::zero())
        throw std::invalid_argument("interval cannot less than zero");
      return std::move(interval);
    }

    template<typename Clock>
    auto adjust_time_point(typename timer<Clock>::time_point &tp,
        const typename timer<Clock>::time_point &now,
        const typename timer<Clock>::duration &duration)
      noexcept -> decltype((now - tp) / duration)
    {
      if (duration == timer<Clock>::duration::zero())
        return 0;
      if (tp >= now)
      {
        return 0;
      }
      else
      {
        auto d = now - tp;
        auto ret = d / duration;
        tp += (ret + 1) * duration;
        return ret;
      }

    }

    template<typename Clock>
    void update_timerfd(const system_handle &timerfd, typename timer<Clock>::duration duration)
    {
      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
      duration -= std::chrono::seconds(seconds);
      auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
      itimerspec itspc {
        { 0, 0 },
        { seconds, nanoseconds}
      };

      int result = timerfd_settime(timerfd.get_raw_handle(), 0, &itspc, nullptr);
      if (result == -1)
        throw_exception_for_last_error();
    }
  }

  template<typename Clock>
  timer_service<Clock>::timer_service(event_loop &el, typename timer_service<Clock>::service_tag)
    : service_template<timer_service<Clock>, event_loop *>(&el)
    , m_deadline_timer_queue()
    , m_timer_fd(clock_spec<Clock>::create_device())
  { }

  template<typename Clock>
  void timer_service<Clock>::on_attach(event_loop &el)
  {
    epoll_event epev;
    epev.data.ptr = static_cast<event_source*>(this);
    epev.events = EPOLLIN | EPOLLET;
    int result = epoll_ctl(el.get_poll_handle().get_raw_handle(), EPOLL_CTL_ADD,
        m_timer_fd.get_raw_handle(), &epev);

    if (result == -1)
    {
      int tmperrno = errno;
      errno = 0;
      throw std::system_error(tmperrno, std::system_category());
    }
  }

  template<typename Clock>
  void timer_service<Clock>::on_detach(event_loop &el)
  {
    int result = epoll_ctl(el.get_poll_handle().get_raw_handle(), EPOLL_CTL_DEL,
        m_timer_fd.get_raw_handle(), nullptr);

    if (result == -1)
    {
      errno = 0;
      std::terminate(); // Don't know how to handle this error
    }
  }

  template<typename Clock>
  void timer_service<Clock>::on_active(event_loop &el)
  {
    auto now = timer::clock::now();
    task::queue_type l;

    while (!m_deadline_timer_queue.empty() && m_deadline_timer_queue.front() <= now)
    {
      auto &t = m_deadline_timer_queue.front();
      l.push_back(t.m_task);
      t.relay(now);
    }

    el.dispatch(std::move(l));

    if (!m_deadline_timer_queue.empty())
      update_timerfd<Clock>(m_timer_fd, timer::get_index(m_deadline_timer_queue.front()) - now);

  }

  template<typename Clock>
  void timer_service<Clock>::enqueue(timer &t) noexcept
  {
    if (m_deadline_timer_queue.empty())
      timer_service::get_index(*this)->attach_event_source(*this);
    m_deadline_timer_queue.insert(t, intruse::policy_backmost);
    if (&m_deadline_timer_queue.front() == &t)
    {
      update_wakeup_time();
    }
  }

  template<typename Clock>
  void timer_service<Clock>::update_wakeup_time() noexcept
  {
    auto &t = m_deadline_timer_queue.front();
    update_timerfd<Clock>(m_timer_fd, timer::get_index(t) - timer::clock::now());
  }

  template<typename Clock>
  timer<Clock>::timer(timer_service &service, std::function<void()> procedure,
      typename timer<Clock>::duration interval)
    : timer(service, std::move(procedure), clock::now(), std::move(interval))
  { }

  template<typename Clock>
  timer<Clock>::timer(event_loop &loop, std::function<void()> procedure,
      typename timer<Clock>::duration interval)
    : timer(loop, std::move(procedure), clock::now(), std::move(interval))
  { }

  template<typename Clock>
  timer<Clock>::timer(timer_service &service, std::function<void()> procedure,
      typename timer<Clock>::time_point tp,
      typename timer<Clock>::duration interval)
    : intruse::rbtree_node<typename Clock::time_point, timer>(std::move(tp))
    , m_event_loop(service.get_event_loop())
    , m_interval(check_interval<Clock>(interval))
    , m_task(std::move(procedure))
    , m_missed_counter(adjust_time_point<Clock>(
          timer::get_index(*this), clock::now(), m_interval))
    , m_timer_service(service.shared_from_this())
  {
    start();
  }

  template<typename Clock>
  timer<Clock>::timer(event_loop &loop, std::function<void()> procedure,
      typename timer<Clock>::time_point tp,
      typename timer<Clock>::duration interval)
    : intruse::rbtree_node<typename Clock::time_point, timer>(std::move(tp))
    , m_event_loop(loop)
    , m_interval(check_interval<Clock>(interval))
    , m_task(std::move(procedure))
    , m_missed_counter(adjust_time_point<Clock>(
          timer::get_index(*this), clock::now(), m_interval))
    , m_timer_service(timer_service::get(m_event_loop))
  {
    start();
  }

  template<typename Clock>
  std::pair<typename Clock::time_point, typename Clock::duration>
  timer<Clock>::reset(
      typename timer::time_point initial,
      typename timer::duration interval)
  {
    reset_missed_counter();
    auto now = Clock::now();
    bool will_stop = initial < now && interval == Clock::duration::zero();

    if (will_stop)
    {
      std::pair<typename Clock::time_point, typename Clock::duration> ret(
          timer::get_index(*this), std::move(m_interval));
      if (timer::is_linked(*this))
        timer::unlink(*this);
      timer::update_index(*this, std::move(initial), intruse::policy_backmost);
      m_interval = std::move(interval);
      return ret;
    }

    m_missed_counter = adjust_time_point<Clock>(initial, now, interval);

    std::pair<typename Clock::time_point, typename Clock::duration> ret(
        timer::get_index(*this), std::move(m_interval));

    if (!m_timer_service->m_deadline_timer_queue.empty())
    {
      if (!timer::is_linked(*this))
      {
        // timer should already stopped
        assert(m_interval == Clock::duration::zero());
        assert(timer::get_index(*this) < Clock::now());
        timer::update_index(*this, std::move(initial), intruse::policy_backmost);
        m_interval = std::move(interval);
        start();
      }
      else
      {
        auto &front = m_timer_service->m_deadline_timer_queue.front();
        bool needs_update = (&front == this || initial < ret.first);
        timer::update_index(*this, std::move(initial), intruse::policy_backmost);
        m_interval = std::move(interval);
        if (needs_update)
          m_timer_service->update_wakeup_time();
      }
    }
    else
    {
      // timer should already stopped
      assert(m_interval == Clock::duration::zero());
      assert(timer::get_index(*this) < Clock::now());

      timer::update_index(*this, std::move(initial), intruse::policy_backmost);
      m_interval = std::move(interval);
      start();
    }
    return ret;
  }

  template<typename Clock>
  std::pair<typename Clock::time_point, typename Clock::duration>
  timer<Clock>::reset(typename timer::duration interval)
  {
    return reset(get_time_point(), interval);
  }

  /**
   * @brief Start a timer
   */
  template<typename Clock>
  void timer<Clock>::start()
  {
    // Client code should ensure t is greater than or equals to clock::now()
    //if (m_interval != duration::zero())
      m_timer_service->enqueue(*this);
  }

  template<typename Clock>
  void timer<Clock>::relay(const time_point &now)
  {
    if (m_interval == timer::duration::zero())
    {
      // We must do unlink first, if we release a shared_ptr first, which
      // would result in its destructor called, this timer may be unlink
      // twice. And second call to unlink will result in crash, althrouh we
      // can test if this timer is still linked, do unlink first without
      // checking must be right.
      timer::unlink(*this);
      m_timer_service = nullptr;
    }
    else
    {
      auto next_tp = timer::get_index(*this);
      m_missed_counter = adjust_time_point<Clock>(next_tp, now, m_interval);
      timer::update_index(*this, std::move(next_tp), intruse::policy_backmost);
    }
  }

  template class timer<std::chrono::steady_clock>;
  template class timer<std::chrono::system_clock>;

  template class timer_service<std::chrono::steady_clock>;
  template class timer_service<std::chrono::system_clock>;


}
