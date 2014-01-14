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
#include <spin/time.hpp>

#include <memory>
#include <type_traits>

namespace spin
{

  class timer_service;
  class deadline_timer;
  class cycle_timer;

  class __SPIN_EXPORT__ deadline_timer :
    public intruse::rbtree_node<time::steady_time_point, deadline_timer>
  {
    friend class timer_service;
  public:

    explicit deadline_timer(timer_service &service,
        std::function<void()> procedure = std::function<void()>(),
        time::steady_time_point tp = time::steady_clock::now(),
        bool check_timeout = false) noexcept;

    explicit deadline_timer(event_loop &loop,
        std::function<void()> procedure = std::function<void()>(),
        time::steady_time_point tp = time::steady_clock::now(),
        bool check_timeout = false);

    ~deadline_timer();

    timer_service &get_timer_service() const noexcept
    { return *m_timer_service; }

    void reset_deadline(time::steady_time_point tp,
        bool check_timeout = false) noexcept;

    std::function<void()> reset_procedure (std::function<void()> procedure);

    void cancel() noexcept
    { reset_deadline(time::steady_clock::now(), true); }

  private:

    void enqueue_to_timer_service(bool);

    std::shared_ptr<timer_service> m_timer_service;
    task m_task;
    std::uint64_t m_missed_counter;
  };

  class __SPIN_EXPORT__ timer_service :
    public intruse::rbtree_node<event_loop *, timer_service>,
    public std::enable_shared_from_this<timer_service>,
    public event_source
  {
  private:
    struct __SPIN_INTERNAL__ private_constructable {};

    static intruse::rbtree<event_loop *, timer_service>
    instance_table;

    intruse::rbtree<time::steady_time_point, deadline_timer> m_deadline_timer_queue;
    system_handle m_timer_fd;

    void on_attach(event_loop &el) override;
    void on_active(event_loop &el) override;
    void on_detach(event_loop &el) override;


  public:
    timer_service(event_loop &el, private_constructable);

    virtual ~timer_service() noexcept override {};

    /**
     * @brief Get timer service instance for given event_loop
     */
    static std::shared_ptr<timer_service> get_instance(event_loop &el);

    void enqueue(deadline_timer &t) noexcept;
  };

}

#endif
