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
#include <spin/poll_target.hpp>
#include <thread>

class spin_target : public spin::poll_target
{
  int n;
public:
  spin_target(spin::event_loop &l)
    : poll_target(l)
    , n(10000)
  {}

  virtual bitset on_state_changed(bitset event) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
    if (n--) {
      assert(event.any());
      return event;
    }
    event.reset();
    return event;
  }
};

int main ()
{
  spin::event_loop loop;
  spin_target pt(loop);

  spin_target::bitset flag;
  flag.set(spin_target::POLL_IN);
  pt.notify(flag);
  flag.set(spin_target::POLL_OUT);
  pt.notify(flag);
  flag.set(spin_target::POLL_OUT);
  pt.notify(flag);

  flag.reset();
  pt.notify(flag);

  loop.run();
}
