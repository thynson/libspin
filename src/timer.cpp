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
  namespace
  {
    timer::duration check_interval(timer::duration &interval)
    {
      if (interval < timer::duration::zero())
        throw std::invalid_argument("interval cannot less than zero");
      return std::move(interval);
    }

    auto adjust_time_point(timer::time_point &tp, const timer::time_point &now,
        const timer::duration &duration) -> decltype((now - tp) / duration)
    {
      if (duration == timer::duration::zero())
        return 0;
      if (tp >= now)
      {
        return 0;
      }
      else
      {
        auto d = now - tp;
        auto ret = d / duration;
        tp += (d % duration) + duration;
        return ret;
      }

    }

    void update_timerfd(const system_handle &timerfd, timer::duration duration)
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

  intruse::rbtree<event_loop *, timer_service>
  timer_service::instance_table;

  timer_service::timer_service(event_loop &el,
      timer_service::private_constructable)
    : rbtree_node(&el)
    , enable_shared_from_this()
    , m_deadline_timer_queue()
    , m_timer_fd(timerfd_create, CLOCK_MONOTONIC, TFD_NONBLOCK)
  { }

  void timer_service::on_attach(event_loop &el)
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

  void timer_service::on_detach(event_loop &el)
  {
    int result = epoll_ctl(el.get_poll_handle().get_raw_handle(), EPOLL_CTL_DEL,
        m_timer_fd.get_raw_handle(), nullptr);

    assert(result == 0);
  }

  void timer_service::on_active(event_loop &el)
  {
    auto now = timer::clock::now();
    auto adapter = [](timer &t) noexcept -> task &{ return t.m_task; };
    auto u = m_deadline_timer_queue.upper_bound(now);
    auto b = make_transform_iterator(adapter, m_deadline_timer_queue.begin());
    auto e = make_transform_iterator(adapter, u);

    auto l = task::queue_type(b, e);

    for (auto i = m_deadline_timer_queue.begin(); i != u; )
    {
      timer &t = *i++;
      t.relay(now);
    }
    el.dispatch(std::move(l));
    if (m_deadline_timer_queue.empty())
      get_index(*this)->detach_event_source(*this);
    else
      update_timerfd(m_timer_fd, timer::get_index(m_deadline_timer_queue.front()) - now);

  }

  std::shared_ptr<timer_service>
  timer_service::get_instance(event_loop &el)
  {
    auto i = instance_table.find(&el);
    if (i == instance_table.end())
    {
      auto s = std::make_shared<timer_service>(el, private_constructable());
      instance_table.insert(*s);
      return s;
    }
    else
      return i->shared_from_this();
  }

  void timer_service::enqueue(timer &t) noexcept
  {
    if (m_deadline_timer_queue.empty())
      get_index(*this)->attach_event_source(*this);
    m_deadline_timer_queue.insert(t, intruse::policy_backmost);
    if (&m_deadline_timer_queue.front() == &t)
    {
      update_timerfd(m_timer_fd, timer::get_index(t) - timer::clock::now());
    }
  }

  timer::timer(timer_service &service, std::function<void()> procedure,
      timer::duration interval)
    : timer(service, std::move(procedure), clock::now(), std::move(interval))
  { }

  timer::timer(event_loop &loop, std::function<void()> procedure,
      timer::duration interval)
    : timer(loop, std::move(procedure), clock::now(), std::move(interval))
  { }

  timer::timer(timer_service &service, std::function<void()> procedure,
      timer::time_point tp, timer::duration interval)
    : rbtree_node(std::move(tp))
    , m_event_loop(service.get_event_loop())
    , m_interval(check_interval(interval))
    , m_task(std::move(procedure))
    , m_missed_counter(adjust_time_point(get_index(*this), clock::now(), m_interval))
    , m_timer_service(service.shared_from_this())
  {
    start();
  }

  timer::timer(event_loop &loop, std::function<void()> procedure,
      timer::time_point tp, timer::duration interval)
    : rbtree_node(std::move(tp))
    , m_event_loop(loop)
    , m_interval(check_interval(interval))
    , m_task(std::move(procedure))
    , m_missed_counter(adjust_time_point(get_index(*this), clock::now(), m_interval))
    , m_timer_service(timer_service::get_instance(m_event_loop))
  {
    start();
  }

  void timer::start()
  {
    if (!m_timer_service)
      return;
    auto &t = rbtree_node::get_index(*this);
    if (m_interval != duration::zero() || t > clock::now())
      m_timer_service->enqueue(*this);
  }

  void timer::relay(const time_point &now)
  {
    if (m_interval == timer::duration::zero())
    {
      m_timer_service = nullptr;
      unlink(*this);
    }
    else
    {
      auto next_tp = get_index(*this);
      m_missed_counter = adjust_time_point(next_tp, now, m_interval);
      update_index(*this, std::move(next_tp));
    }
  }

}
