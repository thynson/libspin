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

#include <spin/enable_singleton.hpp>

#include <iostream>
#include <thread>
#include <cassert>
#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>


class volatile_singleton
  : public spin::enable_singleton<volatile_singleton, true>
{
public:
  static std::atomic_long count_construct_times;
  static singleton_factory get_instance;

  bool f() { return flag; }

  ~volatile_singleton()
  {
    flag = false;
  }
protected:
  volatile_singleton()
    : flag(true)
  {
    count_construct_times++;
  }

  std::atomic_bool flag;
};

std::atomic_long volatile_singleton::count_construct_times{0};

std::mutex lock;
std::condition_variable cv;
std::chrono::steady_clock::time_point tp;
std::atomic_long count_access_times;


void mt_access()
{
  while (std::chrono::steady_clock::now() < tp)
  {
    {
      std::shared_ptr<volatile_singleton> x
        = volatile_singleton::get_instance();
      assert(x);
      assert(x->f());
      count_access_times++;
    }
    std::chrono::steady_clock::now();
  }
}


void mt_test(int n)
{
  // Clear counter
  volatile_singleton::count_construct_times = 0;
  count_access_times = 0;

  std::vector<std::thread> vt;


  tp = std::chrono::steady_clock::now() + std::chrono::seconds(1);
  for (int i = 0; i < n; i++)
  { vt.emplace_back(mt_access); }

  for (auto &i : vt)
    i.join();

  std::cout << "Multi-threading test for " << n << "concurrent thread:"
    << std::endl
    << " volatile_singleton singlethon class was constructed "
    << volatile_singleton::count_construct_times
    << " times" << std::endl
    << " while instance be accessed " << count_access_times
    << " times" << std::endl;
}

int main ()
{
  {
    std::shared_ptr<volatile_singleton> x = volatile_singleton::get_instance();
    assert (x); // assert x is not null
  }

  mt_test(4);
  mt_test(16);
  mt_test(64);

  return 0;
}
