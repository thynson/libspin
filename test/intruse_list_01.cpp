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

#include <spin/intruse/list.hpp>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <random>

using namespace std;

class X : public spin::intruse::list_node<X>
{
public:
  X(int i)
    : i(i)
  {}

  friend bool operator < (const X &lhs, const X &rhs) noexcept
  { return lhs.i < rhs.i; }

  friend bool operator > (const X &lhs, const X &rhs) noexcept
  { return rhs < lhs; }

  friend bool operator >= (const X &lhs, const X &rhs) noexcept
  { return !(lhs < rhs); }

  friend bool operator <= (const X &lhs, const X &rhs) noexcept
  { return !(lhs > rhs); }

  friend bool operator == (const X &lhs, const X &rhs) noexcept
  { return lhs.i == rhs.i; }

  friend bool operator != (const X &lhs, const X &rhs) noexcept
  { return lhs.i != rhs.i; }

  using list = spin::intruse::list<X>;
  int i;
};


void test_sort()
{

  vector<X> v;
  std::mt19937 engine;
  for (int i = 0; i < 1000000; ++i)
    v.emplace_back(engine());

  X::list l;

  static_assert(noexcept(l.sort()), "noexcept deduction failed");
  static_assert(noexcept(l.merge(l)), "noexcept deduction failed");
  static_assert(noexcept(l.unique()), "noexcept deduction failed");

  for (auto i = v.begin(); i != v.end(); ++i)
    l.push_back(*i);

  assert(!l.empty());
  l.sort();
  assert(std::is_sorted(l.begin(), l.end()));
}

int main()
{

  X::list l;
  X a = 1, b = 2, c = 3, d = 4, e = 5;
  l.push_back(c);
  l.push_back(b);
  l.push_back(d);
  l.push_back(a);
  l.push_back(e);

  l.sort();

  assert(!l.empty());
  assert(l.front().i = 1);
  assert(l.back().i = 5);
  assert(std::is_sorted(l.begin(), l.end(), std::less<X>()));
  assert(std::is_sorted(l.rbegin(), l.rend(), std::greater<X>()));

  l.reverse();
  assert(std::is_sorted(l.rbegin(), l.rend(), std::less<X>()));
  assert(std::is_sorted(l.begin(), l.end(), std::greater<X>()));

  l.erase(l.begin());
  assert(l.front().i == 4);
  assert(spin::intruse::list_node<X>::is_unlinked(e));

  l.erase(--l.end());
  assert(l.back().i == 2);
  assert(spin::intruse::list_node<X>::is_unlinked(a));

  assert(std::is_sorted(l.rbegin(), l.rend(), std::less<X>()));
  assert(std::is_sorted(l.begin(), l.end(), std::greater<X>()));


  l.erase(++l.begin());
  assert(std::is_sorted(l.rbegin(), l.rend(), std::less<X>()));
  assert(spin::intruse::list_node<X>::is_unlinked(c));

  l.clear();
  assert(l.empty());
  assert(spin::intruse::list_node<X>::is_unlinked(b));
  assert(spin::intruse::list_node<X>::is_unlinked(d));


  l.push_back(a);
  l.push_back(b);
  l.push_back(c);
  l.push_back(d);
  l.push_back(e);

  X::list l2;
  l2.splice(l2.end(), l, ++l.begin(), --l.end());
  assert(l.front().i == 1);
  assert(l.back().i == 5);
  assert(l2.front().i == 2);
  assert(l2.back().i == 4);
  l.splice(++l.begin(), l2, ++l2.begin(), --l2.end());
  assert(*++l.begin() == 3);

  test_sort();
}

