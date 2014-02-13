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

#include <spin/event_loop.hpp>
#include <spin/transform_iterator.hpp>

#include <mutex>

#include <sys/epoll.h>
#include <sys/eventfd.h>

namespace spin
{

  namespace
  {
    task::queue_type
    unqueue_posted_task(spin_lock &lock, task::queue_type &q) noexcept
    {
      std::lock_guard<spin_lock> guard(lock);
      return std::move(q);
    }
  }

  event_loop::event_loop()
    : m_poller_ptr()
    , m_dispatched_queue()
    , m_posted_queue()
    , m_lock()
  { }

  std::shared_ptr<poller> event_loop::get_poller()
  {
    auto p = m_poller_ptr.lock();
    if (p) return p;
    p = std::make_shared<poller>();
    m_poller_ptr = p;
    return p;
  }

  void event_loop::interrupt()
  {
    if (auto p = m_poller_ptr.lock())
      p->interrupt();
  }

  void event_loop::run()
  {
    for ( ; ; )
    {
      task::queue_type q(std::move(m_dispatched_queue));
      q.splice(q.end(), unqueue_posted_task(m_lock, m_posted_queue));

      if (auto p = m_poller_ptr.lock())
      {
        p->poll(q.empty());
        q.splice(q.end(), m_dispatched_queue);
      }

      if (q.empty())
        return;

      for (auto i = q.begin(); i != q.end(); )
      {
        auto &t = *i++;
        t.cancel();
        t();
      }
    }

  }

}
