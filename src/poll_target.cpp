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

#include "spin/poll_target.hpp"

namespace spin {

  poll_target::poll_target(event_loop &loop)
    : m_loop(loop)
    , m_dispatcher([this]()
        {
          bitset result = on_state_changed(m_state);
          bool need_dispatch = result.any();;
          m_state = result;
          if (need_dispatch)
            m_loop.dispatch(m_dispatcher);

        })
    , m_receiver([this]()
        {
          bool need_dispatch;
          {
            std::unique_lock<spinlock> lock_guard(m_lock);
            need_dispatch = !m_state.any();
            m_state |= m_pending;
            m_pending.reset();
          }
          if (need_dispatch)
            m_loop.dispatch(m_dispatcher);

        })
    , m_lock()
    , m_state()
  { }

  poll_target::~poll_target()
  { }

  void poll_target::notify(poll_target::bitset bitset)
  {
    bool need_dispatch;
    {
      std::unique_lock<spinlock> lock_guard(m_lock);
      need_dispatch = !m_pending.any();
      m_pending |= bitset;
    }
    if (need_dispatch)
      m_loop.dispatch(m_receiver);
  }
}
