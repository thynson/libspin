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

    using reference = decltype(std::declval<Adapter>()(*std::declval<Iterator>()));
    static_assert(std::is_reference<reference>::value, "adapter shoud return a refernce");
    using value_type = typename std::remove_reference<reference>::type;
    using pointer = value_type *;

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

    reference operator * ()
    {
      Iterator &iter = *this;
      return (*this)(*iter);
    }

    pointer operator -> ()
    {
      Iterator &iter = *this;
      return &(*this)(*iter);
    }

    transform_iterator operator ++ (int)
    {
      transform_iterator self = *this;
      this->Iterator::operator ++ ();
      return self;
    }

  };
}

#endif
