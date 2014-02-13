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

#include <spin/singleton.hpp>

#include <iostream>
#include <thread>
#include <cassert>
#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>


class X : public spin::singleton<X>
{
public:
  static std::atomic_long count_construct_times;
  constexpr static bool volatility = true;

  X(singleton_tag)
    : flag(true)
  {
    count_construct_times++;
  }

  void f()
  {
    assert (flag);
  }

  ~X()
  {
    flag = false;
  }
protected:

  std::atomic_bool flag;
};

std::atomic_long X::count_construct_times{0};

std::mutex lock;
std::condition_variable cv;
std::chrono::steady_clock::time_point tp;
std::atomic_long count_access_times;


void mt_access()
{
  while (std::chrono::steady_clock::now() < tp)
  {
    {
      std::shared_ptr<X> x = X::get_instance();
      assert(x);
      x->f();
      count_access_times++;
    }
    std::chrono::steady_clock::now();
  }
}


void mt_test(int n)
{
  // Clear counter
  X::count_construct_times = 0;
  count_access_times = 0;

  std::vector<std::thread> vt;


  tp = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  for (int i = 0; i < n; i++)
  { vt.emplace_back(mt_access); }

  for (auto &i : vt)
    i.join();

  std::cout << "Multi-threading test for " << n << "concurrent thread:"
    << std::endl
    << " X's singlethon class was constructed "
    << X::count_construct_times
    << " times" << std::endl
    << " while instance be accessed " << count_access_times
    << " times" << std::endl;
}

int main ()
{
  {
    std::shared_ptr<X> x = X::get_instance();
    assert (x); // assert x is not null
  }

  mt_test(4);
  mt_test(16);
  mt_test(64);

  return 0;
}
