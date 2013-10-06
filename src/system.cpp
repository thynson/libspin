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

#include <spin/system.hpp>
#ifdef __unix__
#include <unistd.h>
#include <errno.h>
#include <string.h>
#endif

namespace spin
{

#ifdef __unix__
  handle::handle(os_handle_t x) noexcept
    : m_os_handle(x)
  {  }

  handle::~handle() noexcept
  {
    close();
  }

  void handle::close() noexcept
  {
    if (this->operator bool())
      ::close(m_os_handle);
    m_os_handle = 0;
  }

  handle::operator bool() const noexcept
  {
    return m_os_handle > 0;
  }

#endif
}