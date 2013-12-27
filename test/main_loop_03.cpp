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
#include <random>
#include <vector>

constexpr unsigned N = 10000;
constexpr unsigned MIN_MILLISECONDS = 1000;
constexpr unsigned MAX_MILLISECONDS = 2000;

int main()
{
  spin::event_loop loop;
  std::vector<spin::event_loop::deadline_timer> vt;
  std::mt19937 random_source;
  spin::time::steady_time_point start = decltype(start)::clock::now();
  unsigned counter = 0;

  for (int i = 0; i < N; i++)
  {
    spin::time::steady_time_point deadline
      = decltype(deadline)::clock::now()
        + std::chrono::milliseconds(MIN_MILLISECONDS +
            random_source() % (MAX_MILLISECONDS - MIN_MILLISECONDS));

    vt.push_back(
        spin::event_loop::deadline_timer(loop, [&]{counter++;},
          std::move(deadline)));
  }
  loop.run();
  assert (counter == N);
  assert ((decltype(start)::clock::now() - start)
      > std::chrono::milliseconds(MIN_MILLISECONDS));
}
