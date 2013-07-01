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
#include <condition_variable>
#include <atomic>
#include <memory>
#include <chrono>
#include "utils.hpp"
#include "callable.hpp"

namespace spin {

  class __SPIN_EXPORT__ event_loop
  {
    class __SPIN_INTERNAL__ poller;
    class __SPIN_INTERNAL__ delayed_callback;
  public:
    event_loop();
    ~event_loop();
    void run();
    void post(callable &c);
    void post_delayed(callable &c, const time_point &tp);
    void post_delayed(callable &c, time_point &&tp);

  private:

    event_loop(const event_loop &) = delete;
    event_loop &operator = (const event_loop &) = delete;
    event_loop(event_loop &&) = delete;
    event_loop &operator = (event_loop &&) = delete;

    multiset<delayed_callback> m_delayed_callback_list;
    list<callable> m_pending_callback_list;
    list<callable> m_notified_callback_list;
    list<callable> m_io_event_list; //TODO:
    std::shared_ptr<poller> m_poller;

  };

}

#endif
