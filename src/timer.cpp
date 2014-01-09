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

#include <stdexcept>

#include <sys/timerfd.h>
#include <sys/epoll.h>

namespace spin
{
  intruse::rbtree<event_loop *, timer_service>
  timer_service::instance_table;

  timer_service::timer_service(event_loop &el,
      timer_service::private_constructable) noexcept
    : rbtree_node(&el)
    , enable_shared_from_this()
    , m_deadline_timer_queue()
    , m_timer_fd(timerfd_create, CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)
  {
    // TODO: register event source
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
