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

#ifndef __SPIN_TIME_HPP_INCLUDED__
#define __SPIN_TIME_HPP_INCLUDED__

#include <ratio>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <cstdint>

namespace spin
{
  namespace time
  {
    using steady_clock = std::chrono::steady_clock;

    using system_clock = std::chrono::system_clock;

    using system_time_point
      = std::chrono::time_point<std::chrono::system_clock>;

    using steady_time_point
      = std::chrono::time_point<std::chrono::steady_clock>;

  }
}

#endif
