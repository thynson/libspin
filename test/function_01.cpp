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

#include <spin/function.hpp>
#include <iostream>

spin::function<int()> get_function(int i)
{
  return [i]() -> int { return i; };
}

int main () {
  long a = 0;
  long b = 0;
  long c = 0;
  long d = 0;

  auto x = [&a, &b, &c, &d] () -> int {
    return a + b + c + d;
  };

  std::cout << "sizeof(x) = " << sizeof (x) << std::endl;

  static_assert(noexcept(spin::function<void()>(std::move(x))) == false,
      "x shouldn't be inplace allocated");

  auto y = [&] () {
    std::cout << a++ << std::endl;
    std::cout << b++ << std::endl;
    std::cout << c++ << std::endl;
    std::cout << d++ << std::endl;
  };
  static_assert(noexcept(spin::function<void()>(std::move(y))) == false,
      "x shouldn't be inplace allocated");

  spin::function<void()> f = std::move(y);

  spin::function<int()> g = std::move(x);

  std::cout << sizeof (f) << std::endl;

  f();
  f();
  f();
  std::cout << g() << std::endl;

  assert (get_function(5)() == 5);
}
