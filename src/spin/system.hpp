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

#ifndef __SPIN_SYSTEM_HPP_INCLUDED__
#define __SPIN_SYSTEM_HPP_INCLUDED__

#include "environment.hpp"
#include <utility>
#include <system_error>
#include <cerrno>

// For handle_t
#ifdef __unix__
typedef int os_handle_t;
#endif


namespace spin
{

  /** @brief RAAI Wrapper for handle_t */
  class __SPIN_EXPORT__ handle
  {
  public:
    /** @brief Delegate constructor */
    template<typename Callable, typename ...Types>
    handle(Callable &&callable, Types &&...args)
      : handle(construct_aux(std::forward<Callable>(callable),
            std::forward<Types>(args)...))
    {}

    /** @brief Constructor */
    handle(os_handle_t os_handle) noexcept;

    /** @breif Destructor */
    ~handle() noexcept;

    /** @breif Move constructor */
    handle(handle &&x) noexcept
      : m_os_handle(0)
    { std::swap(x.m_os_handle, m_os_handle); }

    /** @brief Move assign operator */
    handle &operator = (handle &&x) noexcept
    { std::swap(x.m_os_handle, m_os_handle); return *this; }

    /** @brief Forbidden copy constructor */
    handle(const handle &) = delete;

    /** @brief Forbidden copy assign operator */
    handle &operator = (const handle &) = delete;

    /** @brief Get handle for system call */
    os_handle_t get_os_handle() const noexcept
    { return m_os_handle; }

    /** @brief Close the handle */
    void close() noexcept;

    /** @breif Test if the managed handle is valid */
    operator bool() const noexcept;

  private:
    /**
     * @brief Helper function for delegation template constructor
     * @tparam Callable The handle constructor type
     * @tparam Types Parameters' type pack
     * @param callable The handle constructor
     * @param args Arguments for callable
     * @throws std::system_error if callable return an invalid handle type
     * (e.g. incase of -1 is returned for UNIX system, std::runtime_error with
     * error message retrieved from strerror_r will be throw)
     */
    template<typename Callable, typename ...Types>
    static os_handle_t construct_aux(Callable &&callable, Types &&...args)
    {
      os_handle_t x = std::forward<Callable>(callable)(
          std::forward<Types>(args)...);
#ifdef __unix__
      if (x == -1)
      {
        throw std::system_error(errno, std::system_category());
      }
      return x;
#endif
    }
    os_handle_t m_os_handle;
  };


}

#endif
