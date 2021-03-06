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

#include <spin/intruse/rbtree.hpp>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <random>
#include <cstdint>
#include <climits>

using namespace std;

class X : public spin::intruse::rbtree_node<int, X>
{
public:
  X(int x)
    : rbtree_node(x)
  { }

};

void test_erase(spin::intruse::rbtree<int, X> &t)
{

  while (!t.empty())
  {
    auto b = t.begin();
    auto e = t.upper_bound(b, *b);
    t.erase(b, e);
  }
}

int main()
{
  vector<X> vx;
  std::mt19937 engine;
  vector<int> v;
  for (int i = 0; i < 1000000; i++)
  {
    vx.emplace_back(engine());
    v.push_back(i);
  }
  spin::intruse::rbtree<int, X> t;

  auto last = t.end();
  for (auto i = vx.begin(); i != vx.end(); ++i)
    last = t.insert(last, *i, spin::intruse::policy_backmost); // test with hint

  for (auto &i : t)
    cout << spin::intruse::rbtree_node<int, X>::get_index(i) << endl;

  std::shuffle(v.begin(), v.end(), engine);
  cout << spin::intruse::rbtree_node<int, X>::get_index(t.front()) << endl;
  cout << spin::intruse::rbtree_node<int, X>::get_index(t.back()) << endl;
  auto e = t.upper_bound(INT_MAX);
  test_erase(t);
  assert(t.empty());

}

