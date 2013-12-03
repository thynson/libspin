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

#ifndef __SPIN_FUNCTIONAL_HPP__
#define __SPIN_FUNCTIONAL_HPP__

#include <type_traits>
#include <functional>

namespace spin
{
  template<typename T = void>
    struct less : public std::binary_function<T, T, bool>
    {
      bool operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs < rhs))
      { return lhs < rhs; }
    };

  template<>
    struct less<void> : public std::binary_function<void, void, bool>
    {
      template<typename LeftType, typename RightType>
      bool operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs < rhs))
      { return lhs < rhs; }
    };

  template<typename T = void>
    struct greater : public std::binary_function<T, T, bool>
    {
      bool operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs > rhs))
      { return lhs > rhs; }
    };

  template<>
    struct greater<void> : public std::binary_function<void, void, bool>
    {
      template<typename LeftType, typename RightType>
      bool operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs > rhs))
      { return lhs > rhs; }
    };

  template<typename T = void>
    struct less_equal : public std::binary_function<T, T, bool>
    {
      bool operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs <= rhs))
      { return lhs <= rhs; }
    };

  template<>
    struct less_equal<void> : public std::binary_function<void, void, bool>
    {
      template<typename LeftType, typename RightType>
      bool operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs <= rhs))
      { return lhs <= rhs; }
    };

  template<typename T = void>
    struct greater_equal : public std::binary_function<T, T, bool>
    {
      bool operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs >= rhs))
      { return lhs >= rhs; }
    };

  template<>
    struct greater_equal<void> : public std::binary_function<void, void, bool>
    {
      template<typename LeftType, typename RightType>
      bool operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs >= rhs))
      { return lhs >= rhs; }
    };

  template<typename T = void>
    struct equals_to : public std::binary_function<T, T, bool>
    {
      bool operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs == rhs))
      { return lhs == rhs; }
    };

  template<>
    struct equals_to<void>
    {
      template<typename LeftType, typename RightType>
      bool operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs == rhs))
      { return lhs == rhs; }
    };

  template<typename T = void>
    struct not_equals_to : public std::binary_function<T, T, bool>
    {
      bool operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs != rhs))
      { return lhs != rhs; }
    };

  template<>
    struct not_equals_to<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      bool operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs != rhs))
      { return lhs != rhs; }
    };

  template<typename T = void>
    struct plus : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs + rhs))
      { return lhs + rhs; }
    };

  template<>
    struct plus<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs + rhs))
        -> decltype(lhs + rhs)
      { return lhs + rhs; }
    };


  template<typename T = void>
    struct minus : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs - rhs))
      { return lhs - rhs; }
    };

  template<>
    struct minus<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs - rhs))
        -> decltype(lhs - rhs)
      { return lhs - rhs; }
    };

  template<typename T = void>
    struct multiplies : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs * rhs))
      { return lhs * rhs; }
    };

  template<>
    struct multiplies<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs * rhs))
        -> decltype(lhs * rhs)
      { return lhs * rhs; }
    };

  template<typename T = void>
    struct devides : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs / rhs))
      { return lhs / rhs; }
    };

  template<>
    struct devides<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs / rhs))
        -> decltype(lhs / rhs)
      { return lhs / rhs; }
    };

  template<typename T = void>
    struct modules : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs % rhs))
      { return lhs % rhs; }
    };

  template<>
    struct modules<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs % rhs))
        -> decltype(lhs % rhs)
      { return lhs % rhs; }
    };

  template<typename T = void>
    struct negate: public std::unary_function<T, T>
    {
      T operator () (const T &x)
        noexcept(noexcept(-x))
      { return -x; }
    };

  template<>
    struct negate<void> : public std::unary_function<void, void>
    {
      template<typename T>
      auto operator () (const T &x)
        noexcept(noexcept(-x))
        -> decltype(-x)
      { return -x; }
    };

  template<typename T = void>
    struct logical_and : public std::binary_function<T, T, bool>
    {
      T operator () (const T &lhs,
                               const T &rhs)
        noexcept(noexcept(lhs && rhs))
      { return lhs && rhs; }
    };

  template<>
    struct logical_and<void> : public std::binary_function<void, void, bool>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs && rhs))
        -> decltype(lhs && rhs)
      { return lhs && rhs; }
    };

  template<typename T = void>
    struct logical_or : public std::binary_function<T, T, bool>
    {
      T operator () (const T &lhs,
                               const T &rhs)
        noexcept(noexcept(lhs || rhs))
      { return lhs || rhs; }
    };

  template<>
    struct logical_or<void> : public std::binary_function<void, void, bool>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs || rhs))
        -> decltype(lhs || rhs)
      { return lhs || rhs; }
    };

  template<typename T = void>
    struct logical_not : public std::unary_function<T, bool>
    {
      T operator () (const T &x)
        noexcept(noexcept(!x))
      { return !x; }
    };

  template<>
    struct logical_not<void> : public std::unary_function<void, bool>
    {
      template<typename T>
      auto operator () (const T &x)
        noexcept(noexcept(!x))
        -> decltype(!x)
      { return !x; }
    };

  template<typename T = void>
    struct bit_and : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs & rhs))
      { return lhs & rhs; }
    };

  template<>
    struct bit_and<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs & rhs))
        -> decltype(lhs & rhs)
      { return lhs & rhs; }
    };

  template<typename T = void>
    struct bit_or : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs | rhs))
      { return lhs | rhs; }
    };

  template<>
    struct bit_or<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs | rhs))
        -> decltype(lhs | rhs)
      { return lhs | rhs; }
    };

  template<typename T = void>
    struct bit_xor : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        noexcept(noexcept(lhs ^ rhs))
      { return lhs ^ rhs; }
    };

  template<>
    struct bit_xor<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        noexcept(noexcept(lhs ^ rhs))
        -> decltype(lhs ^ rhs)
      { return lhs ^ rhs; }
    };
}

#endif
