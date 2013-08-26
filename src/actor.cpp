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

#include <spin/actor.hpp>

namespace spin
{
  actor::actor()
    : m_timed_callbacks()
    , m_defered_callbacks()
    , m_posted_callbacks()
    , m_notifier_lock()
    , m_condition_variable()
  { }

  actor::~actor()
  {
  }

  void actor::run()
  {
    m_stop = false;
    callback_list cblist;

    while (!m_stop)
    {
      {
        m_defered_callbacks.splice(m_defered_callbacks.end(), cblist);
        unique_lock holder(m_notifier_lock);
        m_posted_callbacks.splice(m_posted_callbacks.end(), cblist);
        if (cblist.empty())
          m_condition_variable.wait(holder);
      }

      for (auto i = cblist.begin(); i != cblist.end(); )
      {
        callback &cb = *i++;
        cb.m_node.unlink();
        cb.m_handler();
      }
    }
  }
}
