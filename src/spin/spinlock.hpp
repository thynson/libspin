/*
 * Copyright (C) 2013
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

#ifndef __SPIN_SPINLOCK_HPP_INCLUDED__
#define __SPIN_SPINLOCK_HPP_INCLUDED__

#include <atomic>
#include "utils.hpp"

namespace spin {

  class __SPIN_EXPORT__ spinlock
  {
  private:
    std::atomic_bool m_lock;

  public:
    spinlock(bool locked = false) noexcept
      : m_lock(locked)
    { }

    ~spinlock() noexcept
    { }

    spinlock(const spinlock &) = delete;

    spinlock(spinlock &&tmp) = delete;

    void lock() noexcept
    {
      bool expected = false;
      while (m_lock.compare_exchange_weak(expected, true,
          std::memory_order_release,
          std::memory_order_relaxed));
    }

    void unlock() noexcept
    {
      m_lock.store(false);
    }
  };
}

#endif
