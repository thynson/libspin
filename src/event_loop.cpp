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

#include "spin/event_loop.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include "spin/event_loop.hpp"
#include <mutex>
#include <condition_variable>
#include <memory>

namespace spin {

  // @brief singleton I/O poller for all event loop
  class __SPIN_INTERNAL__ event_loop::poller
  {
  public:
    ~poller();

    // @brief singleton instance getter
    static std::shared_ptr<poller> get_instance();

    // @breif Creation timestamp
    const time_point base_timestamp;

    // @brief epoll file descripter
    const int epollfd;

    // @brief poll
    list<callable> poll(event_loop &loop);

  private:

    constexpr static size_t EVENT_BUFFER_SIZE = 128;

    // @brief Constructor
    // @note Private constructor that will only be called by get_instance()
    // with a ::unique_lock that hold the #s_mutex as argument
    poller(unique_lock &uq);

    poller(const poller &) = delete;
    poller(poller &&) = delete;
    poller &operator = (const poller &) = delete;
    poller &operator = (poller &&) = delete;

    // @brief poller thread routine
    static void routine(poller &p);

    // @brief singleton lock
    static std::mutex s_lock_singleton;

    // @brief Weak reference to poller instance
    static std::weak_ptr<poller> s_instance;

    // @brief a pair of pipe, close write-end to notify poller thread to exit
    int m_exit_notifier[2];

    // @brief poller lock
    std::mutex m_lock_poller;

    // @brief poller condition variable for notifying I/O event
    std::condition_variable m_condition_variable;

    // @brief thread id
    std::thread m_tid;

  };

  class __SPIN_INTERNAL__ event_loop::delayed_callback
    : public set_node<delayed_callback> {
  public:
    delayed_callback(callable &c, const time_point &tp) noexcept
      : m_callable(c)
      , m_time_point(tp)
    { }

    delayed_callback(callable &c, time_point &&tp) noexcept
      : m_callable(c)
      , m_time_point(std::move(tp))
    { }

    virtual ~delayed_callback() noexcept
    { }

    callable &get_callback () noexcept
    { return m_callable; }

    const time_point &get_time_point() const noexcept
    { return m_time_point; }

    friend bool operator < (const delayed_callback &lhs,
                            const delayed_callback &rhs) noexcept
    { return lhs.m_time_point < rhs.m_time_point; }

    friend bool operator > (const delayed_callback &lhs,
                            const delayed_callback &rhs) noexcept
    { return lhs.m_time_point > rhs.m_time_point; }

  private:
    delayed_callback(const delayed_callback &) = delete;
    delayed_callback(delayed_callback &&) = delete;
    delayed_callback &operator = (const delayed_callback &) = delete;
    delayed_callback &operator = (delayed_callback &&) = delete;

    callable &m_callable;
    time_point m_time_point;
  };



  std::weak_ptr<event_loop::poller>
    event_loop::poller::s_instance;

  std::mutex
    event_loop::poller::s_lock_singleton;

  //
  // Poller thread routine
  //
  // This routine polls all epoll events and dispatch them to corresponding
  // evnet loop until this thread be notified to exit.
  //
  void event_loop::poller::routine(poller &p)
  {
    int epollfd;
    int pipe_read_end;

    {
      // Wait for initialization of poller and get epollfd and pipe
      // notifier
      unique_lock guard(p.m_lock_poller);

      // Notify poller that we are ready
      p.m_condition_variable.notify_all();
      epollfd = p.epollfd;
      pipe_read_end = p.m_exit_notifier[0];
    }

    bool will_exit = false;
    while (!will_exit) {
      epoll_event events[EVENT_BUFFER_SIZE];
      int ret = epoll_wait (epollfd, events, EVENT_BUFFER_SIZE, -1);

      if (ret <= 0)
        /* XXX: What to do*/
        continue;

      for (int i = 0; i < ret; i++) {
        if (events[i].data.ptr == nullptr) {
          char dummy;
          if (read (pipe_read_end, &dummy, sizeof(dummy)) <= 0)
            // We've been notified to exit
            will_exit = true;
        } else {
          // TODO: Dispatch event to its event loop
        }
      }
    }
  }

  //
  // Constructor of poller
  //
  // @note This constructor is declared as private, it should only be called
  // from poller::get_instance() and poller::s_lock_singleton should be
  // ensure locked.
  //
  event_loop::poller::poller(unique_lock &uq)
    try
    : base_timestamp(std::chrono::steady_clock::now())
    , epollfd(epoll_create1(O_CLOEXEC))
    , m_lock_poller()
    , m_condition_variable()
    , m_tid()
    {
      if (!uq.owns_lock())
        throw std::exception();

      if (epollfd == -1) {
        // FIXME:
        throw std::exception();
      }
      int ret = pipe2(m_exit_notifier, O_CLOEXEC | O_NONBLOCK);
      if (ret == -1) {
        throw std::exception();
      }

      unique_lock guard(m_lock_poller);
      m_tid = std::thread(routine, std::ref(*this));

      // Register EPOLLIN for read-end of m_exit_notifier, so that when
      // write-end be closed, the poller will be noticed and know it's time to
      // exit
      epoll_event ev;
      ev.events = EPOLLIN | EPOLLET;
      ev.data.ptr = nullptr;

      ret = epoll_ctl (epollfd, EPOLL_CTL_ADD, m_exit_notifier[0], &ev);
      if (ret == -1)
        throw std::exception();

      // Wait till poller thread is ready
      m_condition_variable.wait(guard);
    } catch (...) {
      // Clean up resouces that won't be automatically done by compiler
      if (epollfd != 0) {
        close (epollfd);
      }
      if (m_exit_notifier[0] > 0)
        close (m_exit_notifier[0]);

      if (m_exit_notifier[1] > 0)
        close (m_exit_notifier[1]);
      throw;
    }

  //
  // Destructor of poller
  //
  // will notify the poller to exit and then join it
  //
  event_loop::poller::~poller()
  {
    // Close the write-end of this pipe to trigger an EPOLLIN event to notify
    // the thread to exit
    close(m_exit_notifier[1]);

    // Now wait the poler thread to join
    m_tid.join();

    // Finally close all the other resources
    close(m_exit_notifier[0]);
    close(epollfd);
  }

  //
  // Get instance of poller
  //
  // This function is a double-checked singleton helper function
  //
  std::shared_ptr<event_loop::poller>
    event_loop::poller::get_instance()
    {
      auto ret = s_instance.lock();
      if (!ret) {
        unique_lock lock(s_lock_singleton);
        if (!ret) {
          ret.reset(new poller(lock));
          s_instance = ret;
          return ret;
        }
      }
      return ret;
    }

  // @brief Poll event for an event loop
  // @param loop The event loop
  //
  // This function will block until an event is fired or return an empty list
  // immediately if there is no event will be fired
  list<callable> event_loop::poller::poll(event_loop &loop)
  {
    list<callable> tasks;
    tasks.swap(loop.m_pending_callback_list);

    if (loop.m_delayed_callback_list.empty()) {
      unique_lock guard(m_lock_poller);
      if (loop.m_notified_callback_list.empty()) {
        if (tasks.empty() && !loop.m_io_event_list.empty()) {
          // No timer, just wait for other event
          do
            m_condition_variable.wait(guard);
          while (loop.m_notified_callback_list.empty());
          //tasks.swap(m_notified_event_list);
        }
      }
    } else {
      auto &tp = loop.m_delayed_callback_list.begin()->get_time_point();
      unique_lock guard(loop.m_poller->m_lock_poller);
      if (loop.m_notified_callback_list.empty()) {
        if (tasks.empty()) {
          // There are timers, wait until the first expire time point
          do {
            std::cv_status status = m_condition_variable.wait_until(guard, tp);
            if (status == std::cv_status::timeout) {
              // Insert all timer event that have same time point with tp and
              // remove them from loop.m_timer_event_set
              auto f = loop.m_delayed_callback_list.begin();
              auto e = loop.m_delayed_callback_list.upper_bound(*f);
              for (auto i = f; i != e; ++i)
                tasks.push_back(i->get_callback());
              // XXX: Avoid using new/delete
              loop.m_delayed_callback_list.erase_and_dispose(f, e,
                  [](delayed_callback *x){ delete x; });
              return tasks;
            }
          } while (loop.m_delayed_callback_list.empty());
        }
      }
    }
    tasks.splice(tasks.end(), loop.m_notified_callback_list);
    return tasks;
  }

  event_loop::event_loop()
    : m_poller(poller::get_instance())
  { }

  event_loop::~event_loop()
  {
  }

  void event_loop::run()
  {
    for ( ; ; ) {
      list<callable> tasks = m_poller->poll(*this);
      if (tasks.empty())
        return;
      while (!tasks.empty()) {
        (*tasks.begin())();
        tasks.pop_front();
      }
    }
  }

  void event_loop::post(callable &c)
  { m_pending_callback_list.push_back(c); }

  void event_loop::post_delayed(callable &c, const time_point &tp)
  {
    if (tp < m_poller->base_timestamp)
      m_pending_callback_list.push_back(c);
    else {
      // XXX: Avoid using new/delete
      auto ptr = new delayed_callback(c, tp);
      m_delayed_callback_list.push_back(*ptr);
    }
  }

  void event_loop::post_delayed(callable &c, time_point &&tp)
  {
    if (tp < m_poller->base_timestamp)
      m_pending_callback_list.push_back(c);
    else {
      // XXX: Avoid using new/delete
      auto ptr = new delayed_callback(c, std::move(tp));
      m_delayed_callback_list.push_back(*ptr);
    }
  }

}
