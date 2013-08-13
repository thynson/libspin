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

  class __SPIN_EXPORT__ poll_target
  {
  public:
    enum
    {
      POLL_IN,
      POLL_OUT,
      POLL_ERROR,
      POLL_MAX
    };

    typedef std::bitset<POLL_MAX> bitset;

    poll_target(event_loop &loop);

    virtual ~poll_target();

    void notify(bitset bitset);

    event_loop &get_event_loop() const
    { return m_loop; }

  private:
    friend class poller;
    virtual bitset on_state_changed(bitset event) = 0;
    event_loop &m_loop;
    callback m_dispatcher;
    callback m_receiver;
    spinlock m_lock;
    bitset m_state;
    bitset m_pending;
  };


}

#endif
