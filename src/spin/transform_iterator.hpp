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

#ifndef __SPIN_TRANSFORM_ITERATOR_HPP__
#define __SPIN_TRANSFORM_ITERATOR_HPP__

#include <type_traits>
#include <utility>

namespace spin
{
  template<typename Adapter, typename Iterator>
  class transform_iterator : public Iterator, private Adapter
  {
  public:

    /** @brief Reference type of value Adapter returns*/
    using reference = decltype(std::declval<Adapter>()(*std::declval<Iterator>()));

    /** @brief Value type of value Adapter returns */
    using value_type = typename std::remove_reference<reference>::type;

    /** @brief Pointer type of value Adapter returns */
    using pointer = value_type *;

    // Adapter functor should return reference
    static_assert(std::is_reference<reference>::value, "adapter shoud return a refernce");

    /** @brief Whether original iterator throws no exception when dereferencing */
    constexpr static bool is_iterator_dereference_noexcept
      = noexcept(*std::declval<Iterator>());

    /** @brief Whether adapter throws no exception tranforming */
    constexpr static bool is_adapter_noexcept
      = noexcept(std::declval<Adapter>()(*std::declval<Iterator>()));

    /** @brief Whether iterator throws no exception when iterating forward */
    constexpr static bool is_iterator_forward_noexcept
      = noexcept(++std::declval<Iterator>());

    /** @brief Whether iterator throws no exception when iterating backward */
    constexpr static bool is_iterator_backward_noexcept
      = noexcept(--std::declval<Iterator>());

    transform_iterator(const Adapter &adapter, const Iterator &iter)
      noexcept(noexcept(Iterator(iter))
          && noexcept(Adapter(adapter)))
      : Iterator(iter)
      , Adapter(adapter)
    { }

    transform_iterator(const transform_iterator &) = default;
    transform_iterator(transform_iterator &&) = default;

    transform_iterator &operator = (const transform_iterator &) = default;
    transform_iterator &operator = (transform_iterator &&) = default;

    transform_iterator &operator = (const Iterator &iter)
      noexcept(noexcept(iter = iter))
    {
      Iterator &ref = *this;
      ref = iter;
      return *this;
    }

    transform_iterator &operator = (Iterator && iter)
      noexcept(noexcept(iter = std::move(iter)))
    {
      Iterator &ref = *this;
      ref = std::move(iter);
      return *this;
    }

    reference operator * ()
      noexcept(is_iterator_dereference_noexcept && is_adapter_noexcept)
    {
      Iterator &iter = *this;
      return (*this)(*iter);
    }

    pointer operator -> ()
      noexcept(is_iterator_dereference_noexcept && is_adapter_noexcept)
    {
      Iterator &iter = *this;
      return &(*this)(*iter);
    }

    transform_iterator operator ++ (int)
      noexcept(is_iterator_forward_noexcept)
    {
      transform_iterator self = *this;
      this->Iterator::operator ++ ();
      return self;
    }

    transform_iterator operator -- (int)
      noexcept(is_iterator_forward_noexcept)
    {
      transform_iterator self = *this;
      this->Iterator::operator -- ();
      return self;
    }

  };


  template<typename Adapter, typename Iterator>
  transform_iterator<
    typename std::remove_reference<Adapter>::type,
    typename std::remove_reference<Iterator>::type>
  make_transform_iterator(Adapter &&adapter, Iterator &&iter)
  noexcept(noexcept(transform_iterator<
    typename std::remove_reference<Adapter>::type,
    typename std::remove_reference<Iterator>::type>(
      std::forward<Adapter>(adapter), std::forward<Iterator>(iter))))
  {
    return transform_iterator<
    typename std::remove_reference<Adapter>::type,
    typename std::remove_reference<Iterator>::type>(
      std::forward<Adapter>(adapter), std::forward<Iterator>(iter));
  }

}

#endif
