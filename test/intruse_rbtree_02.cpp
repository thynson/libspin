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
#include <vector>
#include <cassert>
#include <algorithm>

using namespace std;
using namespace spin::intruse;

class X : public rbtree_node<int, X>
{
public:
  X(int x = 0)
    : rbtree_node(x)
  {}
};

int main()
{
  X vn[100];
  rbtree<int, X> tree;

  for (int i = 0; i < 100; i++)
    tree.insert(vn[i], policy_backmost);
  assert(std::is_sorted(tree.begin(), tree.end()));
  tree.clear();

  for (int i = 0; i < 100; i++)
    tree.insert(vn[i], policy_frontmost);
  assert(std::is_sorted(tree.begin(), tree.end()));
  tree.clear();

  for (int i = 0; i < 100; i++)
    tree.insert(vn[i], policy_nearest);
  assert(std::is_sorted(tree.begin(), tree.end()));
  tree.clear();

  for (int i = 0; i < 100; i++)
    tree.insert(vn[i], policy_unique);
  assert(std::is_sorted(tree.begin(), tree.end()));
  tree.clear();

  for (int i = 0; i < 100; i++)
    tree.insert(vn[i], policy_override);
  assert(std::is_sorted(tree.begin(), tree.end()));
  tree.clear();
}

