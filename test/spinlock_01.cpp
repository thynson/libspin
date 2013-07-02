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

#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <spin/spinlock.hpp>

using namespace std;
using namespace spin;

spinlock s_lock;
long counter = 0;
mutex guard;
condition_variable ready;

constexpr long N_THREAD = 100;
constexpr long N_ITERATION = 1000000;
constexpr long EXPECTED = N_THREAD * N_ITERATION;

void sync()
{
  std::unique_lock<mutex> uq(guard);
}

void routine()
{
  sync();
  for (int i = 0; i < N_ITERATION; i++)
  {
    std::unique_lock<mutex> uq(guard);
    counter++;
  }
}

vector<thread> start_threads()
{
  std::unique_lock<mutex> uq(guard);
  vector<thread> vt;
  for (int i = 0; i < N_THREAD; i++)
    vt.push_back(thread(routine));
  return vt;
}

int main()
{
  vector<thread> vt = start_threads();
  for (auto i = vt.begin(); i != vt.end(); ++i)
    i->join();
  cout << counter << endl;
  assert(counter == EXPECTED);
}
