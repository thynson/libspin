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

#include <spin/event_loop.hpp>
#include <vector>
#include <iostream>

constexpr unsigned N = 10000, M = 1000000;

void test_01 ()
{
  unsigned counter = 0;
  spin::event_loop loop;

  std::vector<spin::task> vt;

  for (unsigned i = 0; i < N; i++)
  {
    vt.emplace_back([i, &vt, &counter, &loop] {
          if (counter ++ < M - N)
            loop.dispatch(vt[i]);
        });
    loop.dispatch(*vt.rbegin());

  }

  loop.run();
  assert (counter == M);
}

void test_02()
{
  unsigned counter = 0;
  spin::event_loop loop;

  std::vector<spin::task> vt;

  for (unsigned i = 0; i < M; i++)
  {
    vt.emplace_back([&counter] { counter ++; });
    loop.dispatch(*vt.rbegin());
  }

  for (unsigned i = 0; i < M; i++)
    if (i % 2)
      vt[i].cancel();

  loop.run();
  assert (counter == (M/2));
}

void test_03()
{
  spin::event_loop loop;
  bool flag = true;
  spin::task x { [] {} };
  //auto x = loop.set_task([]{});
  x.cancel();
  loop.dispatch(x);
  //x.set_proc([&]{ flag = true; });
  x.reset_procedure([&] { flag = true; });
  loop.run();
  assert (x.is_canceled());
  assert (flag);
}

int main()
{
  test_01();
  test_02();
  test_03();
}
