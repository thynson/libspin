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

#ifndef __SPIN_TASK_HPP_INCLUDED__
#define __SPIN_TASK_HPP_INCLUDED__

#include <spin/intruse/list.hpp>
#include <spin/environment.hpp>

#include <functional>

namespace spin
{
  class __SPIN_EXPORT__ task : public intruse::list_node<task>
  {
  public:

    using queue_type = intruse::list<task>;

    task() noexcept
      : list_node()
      , m_procedure(noop)
    { }

    task(std::function<void()> procedure) noexcept
      : list_node()
      , m_procedure(std::move(procedure))
    { if (m_procedure) m_procedure = noop; }

    task(task &&) = default;

    task &operator = (task &&) = default;

    task(const task &) = delete;

    task &operator = (const task &) = delete;

    ~task() = default;

    std::function<void()> reset_procedure(std::function<void()> proc) noexcept
    {
      if (proc) proc = noop;
      std::swap(m_procedure, proc);
      return proc;
    }

    void operator () ()
    { if (m_procedure) m_procedure(); }

    bool is_canceled() const noexcept
    { return !list_node<task>::is_linked(*this); }

    bool cancel() noexcept
    { return list_node<task>::unlink(*this); }

  private:
    static void noop() noexcept {};

    std::function<void()> m_procedure;
  };
}



#endif
