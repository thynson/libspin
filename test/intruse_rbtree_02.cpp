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

int main()
{
  rbtree_node<int> vn[100];
  rbtree<int> tree;

  for (int i = 0; i < sizeof(vn)/sizeof(vn[0]); i++)
    vn[i].update_key(i * 2); //  0 0 0 0 1 1 1 1 2 2 2 2 ... 24 24 24 24

  for (int i = 0; i < 100; i++)
    tree.insert(vn[i]);

  assert(std::is_sorted(tree.begin(), tree.end()));

  for (auto i = tree.begin(); i != tree.end(); ++i)
  {
    assert (&*tree.lower_bound(i, 35) == &vn[18]);
    assert (&*tree.lower_bound(i, 36) == &vn[18]);
    for (int k = 0; k < 100; k++)
    {
      assert (&*tree.lower_bound(i, 2 * k - 1) == &vn[k]);
      assert (&*tree.lower_bound(i, 2 * k) == &vn[k]);
    }
  }

  for (int k = 0; k < 100; k++)
  {
    assert (&*tree.lower_bound(tree.end(), 2 * k - 1) == &vn[k]);
    assert (&*tree.lower_bound(tree.end(), 2 * k) == &vn[k]);
  }

}

