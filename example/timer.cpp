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
  int i = 0;;
  auto now = spin::timer::clock::now();
  spin::timer a(loop, [&i]()
      {
        std::cout << "this is timer A: "  << i++ << std::endl;
      }, spin::timer::clock::now() + std::chrono::seconds(1),
      std::chrono::seconds(2));

  spin::timer b(loop, []()
      {
        std::cout << "this is timer B" << std::endl;
      }, spin::timer::clock::now() + std::chrono::seconds(3),
      std::chrono::seconds(1));

  spin::timer c(loop, []()
      {
        std::cout << "this is timer C" << std::endl;
      }, spin::timer::clock::now() + std::chrono::seconds(3));
  loop.run();
  std::cout << (spin::timer::clock::now() - now).count() << std::endl;
  return 0;
}
