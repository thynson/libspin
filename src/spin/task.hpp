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

#include <spin/environment.hpp>
#include <spin/intruse/list.hpp>
#include <spin/task.hpp>
#include <spin/routine.hpp>

namespace spin
{
  class __SPIN_EXPORT__ task : public intruse::list_node<task>
  {
  public:

    using queue_type = intruse::list<task>;

    task() noexcept
      : list_node()
      , m_routine()
    { }

    task(routine<> r) noexcept
      : list_node()
      , m_routine(std::move(r))
    { }

    task(task &&) = default;

    task &operator = (task &&) = default;

    task(const task &) = delete;

    task &operator = (const task &) = delete;

    ~task() = default;

    routine<> reset_routine(routine<> proc) noexcept
    {
      std::swap(m_routine, proc);
      return proc;
    }

    void operator () ()
    { m_routine(); }

    bool is_canceled() const noexcept
    { return !list_node<task>::is_linked(*this); }

    bool cancel() noexcept
    { return list_node<task>::unlink(*this); }

  private:

    routine<> m_routine;
  };
}


#endif
