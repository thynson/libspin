/*
 * Copyright (C) 2014 LAN Xingcan
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

#include <list>
#include <random>
#include <iostream>

using namespace spin::intruse;
using namespace spin;


constexpr size_t N = 10000;
struct desending;
struct ascending;

class X : public rbtree_node<int, X, desending, spin::greater<int>, ascending, spin::less<int>>
{
public:
  explicit X(int i): rbtree_node(i) {}

};

int main()
{

  rbtree<int, X, desending, spin::greater<int>> desending_tree;
  rbtree<int, X, ascending, spin::less<int>> ascending_tree;

  std::list<X> l;
  std::mt19937 engine;

  for (size_t i = 0; i < N; ++i)
    l.emplace_back(engine());

  for (auto i = l.begin(); i != l.end(); ++i)
  {
    desending_tree.insert(*i);
    ascending_tree.insert(*i);
  }

  for (auto i = desending_tree.begin(); i != desending_tree.end(); ++i)
    std::cout << X::get_index(*i) << std::endl;

  for (auto i = ascending_tree.begin(); i != ascending_tree.end(); ++i)
    std::cout << X::get_index(*i) << std::endl;
}

