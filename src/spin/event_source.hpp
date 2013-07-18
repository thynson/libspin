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

#include "event_loop.hpp"

namespace spin
{

  class __SPIN_EXPORT__ event_source
  {
  public:

    event_source(event_loop &loop, bool enabled = false)
      : m_loop(&loop)
      , m_enabled(false)
    {
      if (enabled)
        enable();
    }

    event_source(event_source &&t)
      : m_loop(t.m_loop)
      , m_enabled(t.m_enabled)
    { }

    event_source(event_loop &) = delete;

    virtual ~event_source() = default;

    event_source &operator = (event_source &) = delete;

    event_source &operator = (event_source &&t)
    {
      m_loop = t.m_loop;
      m_enabled = t.m_enabled;
      return *this;
    }

    event_loop &get_event_loop() const
    { return *m_loop; }

    void enable()
    {
      if (m_enabled)
        return;
      m_enabled = true;
      m_loop->m_ref_counter++;
      on_state_changed(true);
    }

    void disable()
    {
      if (!m_enabled)
        return;
      m_enabled = false;
      m_loop->m_ref_counter--;
      on_state_changed(false);
    }

  protected:
    virtual void on_state_changed(bool enabled) = 0;
  private:
    event_loop *m_loop;
    bool m_enabled;
  };
}
