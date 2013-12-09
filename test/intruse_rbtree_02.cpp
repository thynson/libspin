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

  for (int i = 0; i < 100; i++)
    tree.insert(vn[i]);

  for (int i = 0; i < sizeof(vn)/sizeof(vn[0]); i++)
    vn[i].update_key((i / 4) * 4); //  0 0 0 0 4 4 4 4 8 8 8 8 ...

  for (auto &i : tree)
    std::cout << i.get_key() << ' ' << &i << std::endl;

  assert(std::is_sorted(tree.begin(), tree.end()));

  for (auto i = tree.begin(); i != tree.end(); ++i)
  {
    for (int k = 0; k <= 25; k++)
    {
      if (k != 25)
      {
        assert (&*tree.lower_bound(i, 4 * k - 3) == &vn[k * 4]);
        assert (&*tree.lower_bound(i, 4 * k - 2) == &vn[k * 4]);
        assert (&*tree.lower_bound(i, 4 * k - 1) == &vn[k * 4]);
        assert (&*tree.lower_bound(i, 4 * k - 0) == &vn[k * 4]);
        assert (&*tree.upper_bound(i, 4 * (k - 1) + 3) == &vn[k * 4]);
        assert (&*tree.upper_bound(i, 4 * (k - 1) + 2) == &vn[k * 4]);
        assert (&*tree.upper_bound(i, 4 * (k - 1) + 1) == &vn[k * 4]);
        assert (&*tree.upper_bound(i, 4 * (k - 1) + 0) == &vn[k * 4]);
      }
      else
      {
        assert (tree.lower_bound(i, 4 * k - 3) == tree.end());
        assert (tree.lower_bound(i, 4 * k - 2) == tree.end());
        assert (tree.lower_bound(i, 4 * k - 1) == tree.end());
        assert (tree.lower_bound(i, 4 * k - 0) == tree.end());
        assert (tree.upper_bound(i, 4 * (k - 1) + 3) == tree.end());
        assert (tree.upper_bound(i, 4 * (k - 1) + 2) == tree.end());
        assert (tree.upper_bound(i, 4 * (k - 1) + 1) == tree.end());
        assert (tree.upper_bound(i, 4 * (k - 1) + 0) == tree.end());
      }

    }
  }

  for (int k = 0; k <= 25; k++)
  {
    if (k != 25)
    {
      assert (&*tree.lower_bound(tree.end(), 4 * k - 3) == &vn[k * 4]);
      assert (&*tree.lower_bound(tree.end(), 4 * k - 2) == &vn[k * 4]);
      assert (&*tree.lower_bound(tree.end(), 4 * k - 1) == &vn[k * 4]);
      assert (&*tree.lower_bound(tree.end(), 4 * k - 0) == &vn[k * 4]);
      assert (&*tree.upper_bound(tree.end(), 4 * (k - 1) + 3) == &vn[k * 4]);
      assert (&*tree.upper_bound(tree.end(), 4 * (k - 1) + 2) == &vn[k * 4]);
      assert (&*tree.upper_bound(tree.end(), 4 * (k - 1) + 1) == &vn[k * 4]);
      assert (&*tree.upper_bound(tree.end(), 4 * (k - 1) + 0) == &vn[k * 4]);
    }
    else
    {
      assert (tree.lower_bound(tree.end(), 4 * k - 3) == tree.end());
      assert (tree.lower_bound(tree.end(), 4 * k - 2) == tree.end());
      assert (tree.lower_bound(tree.end(), 4 * k - 1) == tree.end());
      assert (tree.lower_bound(tree.end(), 4 * k - 0) == tree.end());
      assert (tree.upper_bound(tree.end(), 4 * (k - 1) + 3) == tree.end());
      assert (tree.upper_bound(tree.end(), 4 * (k - 1) + 2) == tree.end());
      assert (tree.upper_bound(tree.end(), 4 * (k - 1) + 1) == tree.end());
      assert (tree.upper_bound(tree.end(), 4 * (k - 1) + 0) == tree.end());
    }
  }

}

