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
  rbtree_set<int> set;

  for (int i = 0; i < 100; i++)
    set.insert(vn[i], policy_backmost);

  for (int i = 0; i < sizeof(vn)/sizeof(vn[0]); i++)
    vn[i].update_key((i / 4) * 4, policy_backmost); //  0 0 0 0 4 4 4 4 8 8 8 8 ...

  for (auto &i : set)
    std::cout << i.get_key() << ' ' << &i << std::endl;

  assert(std::is_sorted(set.begin(), set.end()));

  for (auto i = set.begin(); i != set.end(); ++i)
  {
    for (int k = 0; k <= 25; k++)
    {
      if (k != 25)
      {
        assert (&*set.lower_bound(i, 4 * k - 3) == &vn[k * 4]);
        assert (&*set.lower_bound(i, 4 * k - 2) == &vn[k * 4]);
        assert (&*set.lower_bound(i, 4 * k - 1) == &vn[k * 4]);
        assert (&*set.lower_bound(i, 4 * k - 0) == &vn[k * 4]);
        assert (&*set.upper_bound(i, 4 * (k - 1) + 3) == &vn[k * 4]);
        assert (&*set.upper_bound(i, 4 * (k - 1) + 2) == &vn[k * 4]);
        assert (&*set.upper_bound(i, 4 * (k - 1) + 1) == &vn[k * 4]);
        assert (&*set.upper_bound(i, 4 * (k - 1) + 0) == &vn[k * 4]);
      }
      else
      {
        assert (set.lower_bound(i, 4 * k - 3) == set.end());
        assert (set.lower_bound(i, 4 * k - 2) == set.end());
        assert (set.lower_bound(i, 4 * k - 1) == set.end());
        assert (set.lower_bound(i, 4 * k - 0) == set.end());
        assert (set.upper_bound(i, 4 * (k - 1) + 3) == set.end());
        assert (set.upper_bound(i, 4 * (k - 1) + 2) == set.end());
        assert (set.upper_bound(i, 4 * (k - 1) + 1) == set.end());
        assert (set.upper_bound(i, 4 * (k - 1) + 0) == set.end());
      }

    }
  }

  for (int k = 0; k <= 25; k++)
  {
    if (k != 25)
    {
      assert (&*set.lower_bound(set.end(), 4 * k - 3) == &vn[k * 4]);
      assert (&*set.lower_bound(set.end(), 4 * k - 2) == &vn[k * 4]);
      assert (&*set.lower_bound(set.end(), 4 * k - 1) == &vn[k * 4]);
      assert (&*set.lower_bound(set.end(), 4 * k - 0) == &vn[k * 4]);
      assert (&*set.upper_bound(set.end(), 4 * (k - 1) + 3) == &vn[k * 4]);
      assert (&*set.upper_bound(set.end(), 4 * (k - 1) + 2) == &vn[k * 4]);
      assert (&*set.upper_bound(set.end(), 4 * (k - 1) + 1) == &vn[k * 4]);
      assert (&*set.upper_bound(set.end(), 4 * (k - 1) + 0) == &vn[k * 4]);
    }
    else
    {
      assert (set.lower_bound(set.end(), 4 * k - 3) == set.end());
      assert (set.lower_bound(set.end(), 4 * k - 2) == set.end());
      assert (set.lower_bound(set.end(), 4 * k - 1) == set.end());
      assert (set.lower_bound(set.end(), 4 * k - 0) == set.end());
      assert (set.upper_bound(set.end(), 4 * (k - 1) + 3) == set.end());
      assert (set.upper_bound(set.end(), 4 * (k - 1) + 2) == set.end());
      assert (set.upper_bound(set.end(), 4 * (k - 1) + 1) == set.end());
      assert (set.upper_bound(set.end(), 4 * (k - 1) + 0) == set.end());
    }
  }

  cout << "Testing find()" << endl;

  for (int k = 0; k < 100; k++)
  {
    if (k % 4 == 0)
    {
      assert(&*set.find(k, policy_backmost) == vn + k + 3);
      assert(&*set.find(k, policy_frontmost) == vn + k);
      assert(&*set.find(k, policy_nearest) < vn + k + 4);
      assert(&*set.find(k, policy_nearest) >= vn + k);
    }
    else
    {
      assert(set.find(k, policy_backmost) == set.end());
      assert(set.find(k, policy_frontmost) == set.end());
      assert(set.find(k, policy_nearest) == set.end());
    }
  }

  for (int i = 0; i < sizeof(vn)/sizeof(vn[0]); i++)
    vn[i].update_key((i / 4) * 4, policy_nearest); //  0 0 0 0 4 4 4 4 8 8 8 8 ...

  for (int k = 0; k < 25; k++)
  {
    cout << (&*set.find(4 * k, policy_nearest) - vn - 4 * k) << endl;
  }
}

