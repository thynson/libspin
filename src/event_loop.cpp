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

#include <spin/event_loop.hpp>
#include <spin/transform_iterator.hpp>

#include <mutex>

#include <sys/epoll.h>
#include <sys/eventfd.h>

namespace spin
{

  namespace
  {
    system_handle setup_eventfd (const system_handle &epollfd)
    {
      system_handle x(eventfd, 0, EFD_NONBLOCK | EFD_CLOEXEC);

      epoll_event evt;

      evt.events = EPOLLIN | EPOLLET;
      evt.data.ptr = nullptr;

      int ret = epoll_ctl (epollfd.get_raw_handle(), EPOLL_CTL_ADD,
          x.get_raw_handle(), &evt);

      if (ret != 0)
        throw std::system_error(errno, std::system_category());
      return x;
    }


    task::queue_type
    unqueue_posted_task(spin_lock &lock, task::queue_type &q) noexcept
    {
      std::lock_guard<spin_lock> guard(lock);
      return std::move(q);
    }
  }

  event_loop::event_loop()
    : m_epoll_handle(epoll_create1, EPOLL_CLOEXEC)
    , m_interrupter(setup_eventfd(m_epoll_handle))
  { }

  void event_loop::run()
  {
    std::array<epoll_event, 512> evarray;

    for ( ; ; )
    {
      task::queue_type q(std::move(m_dispatched_queue));
      q.splice(q.end(), unqueue_posted_task(m_lock, m_posted_queue));

      if (!m_event_sources.empty())
      {
        int timeout = q.empty() ? -1 : 0;
        int nfds = epoll_wait(m_epoll_handle.get_raw_handle(),
            evarray.data(), evarray.size(), timeout);

        if (nfds == -1)
        {
          assert (errno != EBADF || errno != EINVAL || errno != EFAULT);
          errno = 0;
          nfds = 0;
        }
        else
        {
          assert((decltype(evarray)::size_type) nfds <= evarray.size());

          for (auto i = evarray.begin(); i != evarray.begin() + nfds; ++i)
          {
            event_source *s = reinterpret_cast<event_source *>(i->data.ptr);
            s->on_active(*this);
          }

          q.splice(q.end(), m_dispatched_queue);
        }
      }

      if (q.empty())
        return;

      for (auto i = q.begin(); i != q.end(); )
      {
        auto &t = *i++;
        t.cancel();
        t();
      }
    }

  }

}
