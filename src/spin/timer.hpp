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
#include <spin/service_template.hpp>

#include <memory>
#include <type_traits>

namespace spin
{

  class timer_service;

  class __SPIN_EXPORT__ timer :
    public intruse::rbtree_node<std::chrono::steady_clock::time_point, timer>
  {
    friend class timer_service;
  public:

    using clock = std::chrono::steady_clock;
    using time_point = clock::time_point;
    using duration = clock::duration;

    explicit timer(timer_service &service,
        std::function<void()> procedure = std::function<void()>(),
        duration interval = duration::zero());

    explicit timer(event_loop &loop,
        std::function<void()> procedure = std::function<void()>(),
        duration interval = duration::zero());

    explicit timer(timer_service &service,
        std::function<void()> procedure = std::function<void()>(),
        time_point tp = clock::now(), duration interval = duration::zero());

    explicit timer(event_loop &loop,
        std::function<void()> procedure = std::function<void()>(),
        time_point tp = clock::now(), duration interval = duration::zero());

    ~timer() = default;;

    std::shared_ptr<timer_service> get_timer_service() const noexcept
    { return m_timer_service; }

    std::function<void()> reset_procedure (std::function<void()> procedure);

  private:

    void start();

    void relay(const time_point &now);

    event_loop &m_event_loop;
    duration m_interval;
    task m_task;
    std::uint64_t m_missed_counter;
    std::shared_ptr<timer_service> m_timer_service;
  };


  class __SPIN_EXPORT__ timer_service :
    public service_template<timer_service, event_loop *>,
    public event_source
  {
  public:
    timer_service(event_loop &el, service_tag);

    virtual ~timer_service() noexcept override {};

    event_loop &get_event_loop() noexcept
    { return *get_identity(); }

    const event_loop &get_event_loop() const noexcept
    { return *get_identity(); }

    void enqueue(timer &t) noexcept;

  protected:
    void on_attach(event_loop &el) override;
    void on_active(event_loop &el) override;
    void on_detach(event_loop &el) override;

  private:

    intruse::rbtree<timer::time_point, timer> m_deadline_timer_queue;
    system_handle m_timer_fd;

  };

}

#endif
