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
#include <spin/timer.hpp>
#include <random>
#include <set>
#include <iostream>

constexpr unsigned N = 10000;
constexpr unsigned MIN_MILLISECONDS = 1000;
constexpr unsigned MAX_MILLISECONDS = 2000;

using namespace std;

void stress_test()
{
  spin::event_loop loop;
  std::set<spin::steady_timer> vt;
  std::mt19937 random_source;
  spin::steady_timer::time_point start = decltype(start)::clock::now();
  unsigned counter = 0;

  for (int i = 0; i < N; i++)
  {
    //spin::time::steady_time_point deadline
    spin::steady_timer::time_point deadline
      = decltype(deadline)::clock::now()
        + std::chrono::milliseconds(MIN_MILLISECONDS +
            random_source() % (MAX_MILLISECONDS - MIN_MILLISECONDS));

    vt.emplace(loop, [&]{counter++;}, std::move(deadline));
  }
  cout << "Begin of event loop" << endl;
  loop.run();
  cout << "End of event loop" << endl;
  assert (counter == N);
  assert ((decltype(start)::clock::now() - start)
      > std::chrono::milliseconds(MIN_MILLISECONDS));
}


void behaviour_test()
{
  spin::event_loop loop;
  int counter = 0;
  spin::steady_timer a(loop,
      [&] {
        if (counter == 0)
        {
          cout << "a first alarm" << endl;
          a.reset(spin::steady_timer::duration::zero());
        }
        else
        {
          cout << "a alarm again" << endl;
          a.reset(spin::steady_timer::duration::zero());
        }
      }, spin::steady_timer::clock::now() + chrono::milliseconds(100));
  spin::steady_timer b(loop,
      [&] {
        counter = 1;
        cout << "reset a" << endl;
        a.reset(spin::steady_timer::clock::now() + chrono::seconds(1));
      }, spin::steady_timer::clock::now() + chrono::milliseconds(500));
  cout << "Begin of behaviour test" << endl;
  loop.run();
  cout << "End of behaviour test" << endl;
}

int main()
{
  behaviour_test();
  stress_test();
}
