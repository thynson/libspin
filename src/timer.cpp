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
    timer::duration check_interval(timer::duration interval)
    {
      if (interval < timer::duration::zero())
        throw std::invalid_argument("interval cannot less than zero");
      return interval;
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
  { el.add_event_source(*this); }

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

    el.dispatch(task::queue_type(b, e));
    m_deadline_timer_queue.erase(m_deadline_timer_queue.begin(), u);
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
    m_deadline_timer_queue.insert(t, intruse::policy_backmost);
    if (&m_deadline_timer_queue.front() == &t)
    {
      auto duration = timer::get_index(t) - timer::clock::now();
      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
      duration -= std::chrono::seconds(seconds);
      auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
      itimerspec itspc {
        { 0, 0 },
        { seconds, nanoseconds}
      };

      timerfd_settime(m_timer_fd.get_raw_handle(), 0, &itspc, nullptr);
    }
  }

  std::shared_ptr<timer_service> timer::init_timer_service(timer_service &service)
  {
    auto tp = get_index(*this);
    auto duration = tp - clock::now();
    if (duration < duration::zero())
    {
      if ( m_interval <= decltype(m_interval)::zero())
        return nullptr;
      duration %= m_interval;
      tp = clock::now() + (-duration) % m_interval;
      update_index(*this, std::move(tp));
    }
    return service.shared_from_this();
  }

  std::shared_ptr<timer_service> timer::init_timer_service()
  {
    auto tp = get_index(*this);
    auto duration = tp - clock::now();
    if (duration < duration::zero())
    {
      if ( m_interval <= decltype(m_interval)::zero())
        return nullptr;
      duration %= m_interval;
      tp = clock::now() + (-duration) % m_interval;
      update_index(*this, std::move(tp));
    }
    return timer_service::get_instance(m_event_loop);
  }

  timer::timer(timer_service &service, std::function<void()> procedure,
      timer::time_point tp, timer::duration interval, bool check_timeout) noexcept
    : rbtree_node(std::move(tp))
    , m_event_loop(service.get_event_loop())
    , m_interval(check_interval(std::move(interval)))
    , m_procedure(std::move(procedure))
    , m_task(std::bind(&timer::invoke_procedure, this))
    , m_missed_counter(0)
    , m_timer_service(init_timer_service(service))
  {
    enqueue_to_timer_service(check_timeout);
  }

  timer::timer(event_loop &loop, std::function<void()> procedure,
      timer::time_point tp, timer::duration interval, bool check_timeout)
    : rbtree_node(std::move(tp))
    , m_event_loop(loop)
    , m_interval(std::move(interval))
    , m_procedure(std::move(procedure))
    , m_task(std::bind(&timer::invoke_procedure, this))
    , m_missed_counter(0)
    , m_timer_service(init_timer_service())
  { enqueue_to_timer_service(check_timeout); }

  void timer::enqueue_to_timer_service(bool check_timeout)
  {
    if (!m_timer_service)
      return;
    auto &t = rbtree_node::get_index(*this);
    if (!check_timeout || t > clock::now())
      m_timer_service->enqueue(*this);
  }

  void timer::invoke_procedure()
  {
    if (m_procedure) m_procedure();

    if (m_interval == duration::zero())
      m_timer_service = nullptr;
    else
    {
      update_index(*this, get_index(*this) + m_interval);
      enqueue_to_timer_service(false);
    }
  }

}
