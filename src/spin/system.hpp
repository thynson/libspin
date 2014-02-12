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

#ifdef __unix__
typedef int system_raw_handle;
#else
#error "Unsupported platform"
#endif


namespace spin
{

  /** @brief RAAI Wrapper for handle_t */
  class __SPIN_EXPORT__ system_handle
  {
  public:
    /** @brief Delegate constructor */
    template<typename Callable, typename ...Types>
    system_handle(Callable &&callable, Types &&...args)
      : system_handle(construct_aux(std::forward<Callable>(callable),
            std::forward<Types>(args)...))
    {}

    /** @brief Constructor */
    system_handle(system_raw_handle rawhandle) noexcept;

    /** @brief Destructor */
    ~system_handle() noexcept;

    /** @brief Move constructor */
    system_handle(system_handle &&x) noexcept
      : m_raw_handle(0)
    { std::swap(x.m_raw_handle, m_raw_handle); }

    /** @brief Move assign operator */
    system_handle &operator = (system_handle &&x) noexcept
    { std::swap(x.m_raw_handle, m_raw_handle); return *this; }

    /** @brief Forbidden copy constructor */
    system_handle(const system_handle &) = delete;

    /** @brief Forbidden copy assign operator */
    system_handle &operator = (const system_handle &) = delete;

    /** @brief Get system_handle for system call */
    system_raw_handle get_raw_handle() const noexcept
    { return m_raw_handle; }

    /** @brief Close the system_handle */
    void close() noexcept;

    /** @brief Test if the managed system_handle is valid */
    operator bool() const noexcept;

  private:
    /**
     * @brief Helper function for delegation template constructor
     * @tparam Callable The system_handle constructor type
     * @tparam Types Parameters' type pack
     * @param callable The system_handle constructor
     * @param args Arguments for callable
     * @throws std::system_error if callable return an invalid system_handle type
     * (e.g. incase of -1 is returned for UNIX system, std::runtime_error with
     * error message retrieved from strerror_r will be throw)
     */
    template<typename Callable, typename ...Types>
    static system_raw_handle construct_aux(Callable &&callable, Types &&...args)
    {
      system_raw_handle x = std::forward<Callable>(callable)(
          std::forward<Types>(args)...);
#ifdef __unix__
      if (x == -1)
      {
        int tmperrno = errno;
        errno = 0;
        throw std::system_error(tmperrno, std::system_category());
      }
      return x;
#endif
    }
    system_raw_handle m_raw_handle;
  };

  void __SPIN_EXPORT__ throw_exception_for_last_error();

}

#endif
