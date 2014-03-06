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

// This testcase test whether the rbtree is able to be compiled in varias usa
// case

#include <spin/intruse/rbtree.hpp>

using namespace std;
using namespace spin;
using namespace spin::intruse;


struct tag1;
struct tag2;

class X : public rbtree_node<int, X, tag1, tag2>
        , public rbtree_node<float, X, tag1, tag2, void>
{
public:
  X(int a, float b)
    : rbtree_node<int, X, tag1, tag2>(a)
    , rbtree_node<float, X, tag1, tag2, void>(b)
  {}

};

int main()
{
  rbtree<int, X, tag1> t1;
  rbtree<int, X, tag2> t2;
  rbtree<float, X, tag1> t3;
  rbtree<float, X, tag2> t4;

  X x(1, 2.0);
  X y(3, 4.0);

  t1.insert(x);
  t2.insert(x);
  t3.insert(x);
  t4.insert(x);

  t1.find(1);
  t2.find(1);
  t3.find(2.0);
  t4.find(2.0);

  t1.equals_range(1);
  t2.equals_range(1);
  t3.equals_range(2.0);
  t4.equals_range(2.0);

  t1.begin();
  t1.end();
  t1.cbegin();
  t1.cend();
  t1.rbegin();
  t1.rend();
  t1.upper_bound(1);
  t1.lower_bound(1);
  t1.size();
  t1.remove(1);

  rbtree_node<int, X, tag1, tag2>::unlink<tag2>(y);
  rbtree_node<float, X, tag1, tag2, void>::is_linked<void>(y);

}
