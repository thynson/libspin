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

#include <spin/timer.hpp>

#include <iostream>

int main()
{
  //auto hello = [] () { std::cout << "hello world" << std::endl; };
  spin::event_loop loop;
  int i = 0;
  auto now = spin::steady_timer::clock::now();
  spin::steady_timer a(loop, [&i]()
      {
        std::cout << "this is timer A: "  << i++ << std::endl;
      }, spin::steady_timer::clock::now() + std::chrono::seconds(1),
      std::chrono::seconds(2));

  spin::steady_timer b(loop, [&b]()
      {
        std::cout << "this is timer B: " << b.reset_missed_counter() << std::endl;
      }, spin::steady_timer::clock::now() + std::chrono::seconds(3),
      std::chrono::seconds(1));

  spin::steady_timer c(loop, [&]()
      {
        std::cout << "this is timer C" << std::endl;
        a.reset(spin::steady_timer::clock::now() + std::chrono::seconds(3), std::chrono::seconds(1));
        b.reset(std::chrono::seconds(2));

      }, spin::steady_timer::clock::now() + std::chrono::seconds(8));
  spin::system_timer d(loop, [&d]()
      {
        std::cout << "this is timer D: " << d.reset_missed_counter() << std::endl;
      }, spin::system_timer::clock::now() + std::chrono::seconds(3),
      std::chrono::seconds(1));
  loop.run();
  std::cout << (spin::steady_timer::clock::now() - now).count() << std::endl;
  return 0;
}
