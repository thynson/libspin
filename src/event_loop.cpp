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
#include <atomic>
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

    // @brief poller lock
    static std::mutex s_lock;

    // @brief poller condition variable for notifying I/O event
    static std::condition_variable s_condition_variable;

  private:
    // @brief Constructor
    // @note Private constructor that will only be called by get_instance()
    // with a ::unique_lock that hold the #s_mutex as argument
    poller(unique_lock &uq);

    poller(const poller &) = delete;
    poller(poller &&) = delete;
    poller &operator = (const poller &) = delete;
    poller &operator = (poller &&) = delete;

    // @brief poller thread routine
    static void routine();

    // @brief Weak reference to poller instance
    static std::weak_ptr<poller> s_instance;

    // @brief a pair of pipe, close write-end to notify poller thread to exit
    int m_exit_notifier[2];

    // @brief thread id
    std::thread m_tid;
  };


  std::weak_ptr<event_loop::poller>
    event_loop::poller::s_instance;

  std::mutex
    event_loop::poller::s_lock;

  std::condition_variable
    event_loop::poller::s_condition_variable;

  //
  // Poller thread routine
  //
  // This routine polls all epoll events and dispatch them to corresponding
  // evnet loop until this thread be notified to exit.
  //
  void event_loop::poller::routine()
  {
    int epollfd;
    int pipe_read_end;

    {
      // Wait for initialization of poller and get epollfd and pipe
      // notifier
      unique_lock guard(s_lock);

      // Notify poller that we are ready
      s_condition_variable.notify_all();
      auto p = s_instance.lock();
      if (!p)
        return;
      epollfd = p->epollfd;
      pipe_read_end = p->m_exit_notifier[0];
    }

    bool will_exit = false;
    while (!will_exit) {
      enum {EVENT_BUFFER_SIZE = 128};
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
  // from poller::get_instance() and poller::s_lock should be
  // ensure locked.
  //
  event_loop::poller::poller(unique_lock &uq)
    try
    : base_timestamp(std::chrono::steady_clock::now())
    , epollfd(epoll_create1(O_CLOEXEC))
    , m_tid(routine)
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
      s_condition_variable.wait(uq);
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
        unique_lock lock(s_lock);
        std::atomic_thread_fence(std::memory_order_acquire);
        ret = s_instance.lock();
        if (!ret) {
          std::shared_ptr<poller> p(new poller(lock));
          s_instance = p;
          std::atomic_thread_fence(std::memory_order_release);
          return p;
        }
      }
      return ret;
    }

  event_loop::event_loop()
    : m_poller(poller::get_instance())
  { }

  event_loop::~event_loop()
  {
  }

  // @brief Wait for events
  // @return list of event
  // This function will block if there are any event would be fired, or return
  // a empty list immediately to if the are no event would be fired any more
  list<event> event_loop::wait_for_events()
  {
    list<event> tasks;
    tasks.swap(m_pending_event_list);

    if (m_timer_event_set.empty()) {
      unique_lock guard(poller::s_lock);
      if (m_notified_event_list.empty()) {
        if (tasks.empty() && !m_io_event_list.empty()) {
          // No timer, just wait for other event
          do
            poller::s_condition_variable.wait(guard);
          while (m_notified_event_list.empty());
          //tasks.swap(m_notified_event_list);
        }
      }
      tasks.splice(tasks.end(), m_notified_event_list);
    } else {
      auto &tp = m_timer_event_set.begin()->get_time_point();
      unique_lock guard(poller::s_lock);
      if (m_notified_event_list.empty()) {
        if (tasks.empty()) {
          // There are times, wait until the latest expire time point
          do {
            std::cv_status status
              = poller::s_condition_variable.wait_until(guard, tp);
            if (status == std::cv_status::timeout) {
              tasks.push_back(*m_timer_event_set.begin());
              m_timer_event_set.erase(m_timer_event_set.begin());
              return tasks;
            }
          } while (m_notified_event_list.empty());
        }
      }
      tasks.splice(tasks.end(), m_notified_event_list);
    }
    return tasks;
  }

  void event_loop::run() {
    for ( ; ; ) {
      list<event> tasks = wait_for_events();
      if (tasks.empty())
        return;
      while (!tasks.empty()) {
        auto &x = *tasks.begin();
        tasks.pop_front();
        x.callback();
      }
    }
  }

}
