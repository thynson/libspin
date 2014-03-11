/*
 * Copyright (C) 2013  LAN Xingcan
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

#ifndef __SPIN_UTILS_HPP_INCLUDED__
#define __SPIN_UTILS_HPP_INCLUDED__

#include <spin/environment.hpp>
#include <spin/spin_lock.hpp>

#include <spin/intruse/list.hpp>
#include <spin/intruse/rbtree.hpp>

#include <utility>


namespace spin
{
  template<typename Callable>
  class block_guard
  {
    static_assert(noexcept((std::declval<Callable>()())),
        "Callable functor may not throw exception");

    Callable m_callable;

  public:
    block_guard(const block_guard &) = delete;
    block_guard(block_guard &&) = default;

    block_guard(Callable &&callable)
      : m_callable(std::forward<Callable>(callable))
    {}

    ~block_guard() noexcept
    { m_callable(); }

  };

  template<typename Callable>
  block_guard<Callable> make_block_guard(Callable &&callable)
  {
    return {std::forward<Callable>(callable)};
  }
}

#endif
