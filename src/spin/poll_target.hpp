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

#ifndef __SPIN_POLL_TARGET_HPP_INCLUDED__
#define __SPIN_POLL_TARGET_HPP_INCLUDED__

#include <bitset>
#include "utils.hpp"
#include "event_loop.hpp"
#include "spinlock.hpp"

namespace spin {

  class __SPIN_EXPORT__ poll_target : public list_node<poll_target> {
  public:
    enum {
      POLL_IN,
      POLL_OUT,
      POLL_ERROR,
      POLL_MAX
    };

    typedef std::bitset<POLL_MAX> bitset;

    poll_target(event_loop &loop);

    virtual ~poll_target();

    void notify(bitset bitset);

  private:
    virtual bitset callback(bitset event) = 0;

    struct __SPIN_INTERNAL__ event_dispatcher : public async_procedure {
      event_dispatcher(poll_target &pt);
      virtual ~event_dispatcher() = default;
      virtual void callback();
      poll_target &m_poll_target;
    };

    struct __SPIN_INTERNAL__ event_receiver : public async_procedure {
      event_receiver(poll_target &pt);
      virtual ~event_receiver() = default;
      virtual void callback();
      poll_target &m_poll_target;
    };

    event_loop &m_loop;
    event_dispatcher m_dispatcher;
    event_receiver m_receiver;
    spinlock m_lock;
    bitset m_state;
    bitset m_pending;
  };


}

#endif
