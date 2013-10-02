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

#include <spin/poller.hpp>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <array>

namespace spin
{

  namespace
  {
    handle setup_event_fd(const handle &epollfd)
    {
      handle x(eventfd, 0, EFD_NONBLOCK | EFD_CLOEXEC);

      // Register this eventfd with epoll so other thread may used notify the
      // polling thread to exit
      epoll_event evt;
      evt.events = EPOLLIN | EPOLLET;
      evt.data.ptr = nullptr;
      int ret = epoll_ctl (epollfd.get_handle(), EPOLL_CTL_ADD,
          x.get_handle(), &evt);

      if (ret != 0)
        handle::throw_for_last_system_error(); // XXX
      return x;
    }
  }

  poller::context::context(main_loop &loop, handle &h, poll_flag flag)
    : m_main_loop(loop)
    , m_handle(h)
    , m_poller(singleton<poller>::get_instance())
    , m_lock()
    , m_current_state()
    , m_polled_state()
    , m_poster([this]
        {
          m_main_loop.dispatch(m_dispatcher);
          std::unique_lock<spin_lock> guard(m_lock);
          m_current_state |= m_polled_state;
        })
    , m_dispatcher([this]
        {
          std::unique_lock<spin_lock> guard(m_lock);
          auto state = m_current_state;
          guard.unlock();
          on_poll_event(state);
        })
  {
    m_main_loop.ref();
    epoll_event epev;
    epev.data.ptr = this;
    epev.events = EPOLLET;
    if (flag[POLL_READABLE]) epev.events |= EPOLLIN;
    if (flag[POLL_WRITABLE]) epev.events |= EPOLLOUT;
    if (flag[POLL_ERROR]) epev.events |= EPOLLERR;
    int ret = epoll_ctl(m_poller->m_poller.get_handle(), EPOLL_CTL_ADD,
        h.get_handle(), &epev);
    if (ret == -1)
      handle::throw_for_last_system_error();
  }

  poller::context::~context()
  { m_main_loop.unref(); }

  void poller::context::context::change_poll_flag(
      poller::poll_flag ps) noexcept
  {
    ps.flip(); // Flip before lock
    std::unique_lock<spin_lock> guard(m_lock);
    m_current_state &= ps;
  }

  poller::poller()
    : m_poller(epoll_create1, O_CLOEXEC)
    , m_exit_notifier(setup_event_fd(m_poller))
    , m_thread(std::bind(&poller::poll, this))
  {

  }

  poller::~poller() noexcept
  {
    std::uint64_t value = 1;
    ::write(m_exit_notifier.get_handle(), &value, sizeof(value));
    m_thread.join();
  }

  void poller::poll() noexcept
  {
    std::array<epoll_event, 512> epevts; // May it throws std::bad_alloc?
    bool will_exit = false;
    while (!will_exit)
    {
      int ret = epoll_wait (m_poller.get_handle(), epevts.data(),
          epevts.size(), -1);

      if (ret == -1)
        continue;

      auto begin = epevts.begin(), end = begin + ret;

      auto cmper = [] (const epoll_event &lhs, const epoll_event &rhs)
      {
        context *lt = static_cast<context*>(lhs.data.ptr);
        context *rt = static_cast<context*>(lhs.data.ptr);

        // We need to ensure that the nullptr context which used for notify
        // this thread to exit, if present in evepts, will become the first
        // after sorting
        if (lt == nullptr)
          return true;
        else if (rt == nullptr)
          return false;
        return &lt->get_main_loop() < &rt->get_main_loop();
      };

      std::sort(begin, end, cmper);

      // It may have one but only one whose data.ptr is nullptr, in this
      // situation, we are going to exit this thread
      if (begin->data.ptr == nullptr)
      {
        will_exit = true;
        begin++;
      }

      while (begin != end)
      {
        main_loop::task_list tmplist;
        auto p = std::upper_bound(begin, end, *begin, cmper);

        auto get_context = [] (const epoll_event &e)
        { return static_cast<context*>(e.data.ptr); };

        auto &loop = get_context(*begin)->m_main_loop;

        while (begin != p)
        {
          epoll_event e = *begin++;
          context *t = get_context(e);
          poll_flag ps;

          if (e.events & EPOLLIN) ps.set(POLL_READABLE);
          if (e.events & EPOLLOUT) ps.set(POLL_WRITABLE);
          if (e.events & EPOLLERR) ps.set(POLL_ERROR);

          std::unique_lock<spin_lock> guard(t->m_lock);
          ps &= ~(t->m_current_state | t->m_polled_state);
          t->m_polled_state |= ps;
          if (ps.any())
            tmplist.push_back(t->m_poster);
        }
        loop.post(tmplist);
      }
    }
  }
}
