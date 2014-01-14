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
    auto now = time::steady_clock::now();
    auto adapter = [](deadline_timer &t) noexcept -> task &{ return t.m_task; };
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

  void timer_service::enqueue(deadline_timer &t) noexcept
  {
    m_deadline_timer_queue.insert(t, intruse::policy_backmost);
  }

  deadline_timer::deadline_timer(timer_service &service,
      std::function<void()> procedure, time::steady_time_point tp,
      bool check_timeout) noexcept
    : rbtree_node(std::move(tp))
    , m_timer_service(service.shared_from_this())
    , m_task(std::move(procedure))
    , m_missed_counter(0)
  {
    enqueue_to_timer_service(check_timeout);
  }

  deadline_timer::deadline_timer(event_loop &loop,
      std::function<void()> procedure, time::steady_time_point tp,
      bool check_timeout)
    : rbtree_node(std::move(tp))
    , m_timer_service(timer_service::get_instance(loop))
    , m_task(std::move(procedure))
    , m_missed_counter(0)
  { enqueue_to_timer_service(check_timeout); }

  void deadline_timer::enqueue_to_timer_service(bool check_timeout)
  {
    auto &t = rbtree_node::get_index(*this);
    if (!check_timeout || t > time::steady_clock::now())
      m_timer_service->enqueue(*this);
  }


}
