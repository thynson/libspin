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
#include <iostream>
#include <cassert>


void check_create_task()
{
  spin::event_loop loop;
  bool flag = false;
  auto x = loop.set_task([&flag]{ flag = true; });

  loop.run();

  assert (flag);
}


void check_create_timer()
{
  using namespace spin::time;
  using namespace std::chrono;
  steady_time_point now = decltype(now)::clock::now();
  auto deadline = now + duration_cast<decltype(now)::duration>(seconds(1));
  spin::event_loop loop;
  bool flag = false;
  spin::event_loop::deadline_timer x(loop, [&] { flag = true; },
      std::move(deadline));
  loop.run();
  assert (flag);
}

int main()
{
  check_create_task();
  check_create_timer();
}

