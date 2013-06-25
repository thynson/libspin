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

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <chrono>
#include "utils.hpp"

namespace spin {

  class event_loop;

  class __SPIN_EXPORT__ event : public list_node<event> {
  public:
    virtual ~event();
    virtual void callback();
  };

  class __SPIN_EXPORT__ timer_event : public event
                                   , public set_node<timer_event> {
  public:
    timer_event();
    virtual ~timer_event();

    friend bool operator < (const timer_event &lhs, const timer_event &rhs)
    { return lhs.m_tp < rhs.m_tp; }

    friend bool operator > (const timer_event &lhs, const timer_event &rhs)
    { return lhs.m_tp > rhs.m_tp; }

    const time_point &get_time_point()
    { return m_tp; }

  private:
    time_point m_tp;
  };

  class __SPIN_EXPORT__ io_event : public event
                                 , public list_node<io_event> {
  };



  class __SPIN_EXPORT__ poller_thread {
  public:
    ~poller_thread();

    static std::shared_ptr<poller_thread> get_instance();

    const time_point base_timestamp;
    const int epollfd;

    static std::mutex s_lock;
    static std::condition_variable s_condition_variable;

  private:
    poller_thread(unique_lock &uq);
    poller_thread(const poller_thread &) = delete;
    poller_thread(poller_thread &&) = delete;
    poller_thread &operator = (const poller_thread &) = delete;
    poller_thread &operator = (poller_thread &&) = delete;

    static void poller();
    static std::weak_ptr<poller_thread> s_instance;
    int m_pipe[2];
    std::thread m_tid;
  };

  class __SPIN_EXPORT__ event_loop {
  public:
    event_loop();
    ~event_loop();
    void run ();

  private:

    event_loop(const event_loop &) = delete;
    event_loop &operator = (const event_loop &) = delete;
    event_loop(event_loop &&) = delete;
    event_loop &operator = (event_loop &&) = delete;

    list<event> wait_for_events();

    list<event> m_pending_event_list;
    list<event> m_notified_event_list;
    list<io_event> m_io_event_list;
    multiset<timer_event> m_timer_event_set;
    std::shared_ptr<poller_thread> m_poller_thread;

  };

}

#endif
