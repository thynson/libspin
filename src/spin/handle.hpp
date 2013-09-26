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

#ifndef __SPIN_HANDLE_HPP_INCLUDED__
#define __SPIN_HANDLE_HPP_INCLUDED__

#include <utility>
// For handle_t
#ifdef __unix__
typedef int handle_t;
#else
typedef void *handle_t;
#endif


namespace spin
{

  /** @brief RAAI Wrapper for handle_t */
  class handle
  {
  public:
    /** @brief Constructor */
    handle(handle_t) noexcept;

    /** @breif Destructor */
    ~handle() noexcept;

    /** @breif Move constructor */
    handle(handle &&x) noexcept
    { std::swap(x.m_handle, m_handle); }

    /** @brief Move assign operator */
    handle &operator = (handle &&x) noexcept
    { std::swap(x.m_handle, m_handle); return *this; }

    /** @brief Forbidden copy constructor */
    handle(const handle &) = delete;

    /** @brief Forbidden copy assign operator */
    handle &operator = (const handle &) = delete;

    /** @brief Get handle for system call */
    handle_t get_handle() const noexcept
    { return m_handle; }

  private:
    handle_t m_handle;
  };


}

#endif
