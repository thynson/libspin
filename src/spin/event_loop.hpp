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

#include <spin/task.hpp>
#include <spin/system.hpp>
#include <spin/spin_lock.hpp>
#include <spin/poll.hpp>
#include <spin/utils.hpp>

#include <mutex>
#include <memory>

namespace spin
{

  class event_loop;

  class __SPIN_EXPORT__ event_loop
  {
  public:

    event_loop();

    ~event_loop() = default;

    event_loop(const event_loop &) = delete;

    event_loop(event_loop &&) = delete;

    event_loop &operator = (event_loop &&) = delete;

    event_loop &operator = (const event_loop &) = delete;

    void run();

    void dispatch(task &t) noexcept
    { m_dispatched_queue.push_back(t); }

    void dispatch(task::queue_type q) noexcept
    { m_dispatched_queue.splice(m_dispatched_queue.end(), std::move(q)); }

    void post(task &t) noexcept
    {
      std::lock_guard<spin_lock> guard(m_lock);
      m_posted_queue.push_back(t);
      interrupt();
    }

    void post(task::queue_type q) noexcept
    {
      std::lock_guard<spin_lock> guard(m_lock);
      m_posted_queue.splice(m_posted_queue.end(), std::move(q));
      interrupt();
    }

    std::shared_ptr<poller> get_poller();

    void interrupt();

    /*
    void attach_event_source(event_source &s)
    {
      m_event_sources.push_back(s);
      s.on_attach(*this);
    }

    void detach_event_source(event_source &s)
    {
      s.on_detach(*this);
      event_source::unlink(s);
    }
    */

  private:

    std::weak_ptr<poller> m_poller_ptr;
    task::queue_type m_dispatched_queue;
    task::queue_type m_posted_queue;
    spin_lock m_lock;
  };
}

#endif
