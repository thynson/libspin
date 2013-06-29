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

#include <spin/spin.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <memory>

std::mutex mutex;

void sync()
{
  std::unique_lock<std::mutex> uniq_lock(mutex);
}

void routine()
{
  sync();
  for (int i = 0; i < 100; i++)
    spin::event_loop loop;
}

std::list<std::unique_ptr<std::thread>> start_thread()
{
  std::unique_lock<std::mutex> uniq_lock(mutex);
  std::list<std::unique_ptr<std::thread>> list_threads;

  for (int i = 0; i < 10; i++) {
    list_threads.emplace_back(new std::thread(routine));
  }
  return list_threads;
}

int main()
{
  auto list = start_thread();
  for (auto &x : list) {
    x->join();
  }
  return 0;
}
