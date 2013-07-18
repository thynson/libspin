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
#include "spin/poll_target.hpp"
#include <boost/iterator/transform_iterator.hpp>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace spin {

  // @brief singleton I/O poller for all event loop
  class __SPIN_INTERNAL__ event_loop::poller
  {
  public:
    // @brief Destructor
    ~poller();

    // @brief singleton instance getter
    static std::shared_ptr<poller> get_instance();

    // @breif Creation timestamp
    const time_point base_timestamp;

    // @brief epoll file descripter
    const int epollfd;

    // @brief poll
    callback_list poll(event_loop &loop);

  private:

    // @brief specify the size of epoll_event array to epoll_wait
    constexpr static size_t EVENT_BUFFER_SIZE = 128;

    // @brief Constructor
    // @note Private constructor that will only be called by get_instance()
    // with a ::unique_lock that hold the #s_mutex as argument
    poller(unique_lock &uq);

    poller(const poller &) = delete;
    poller(poller &&) = delete;
    poller &operator = (const poller &) = delete;
    poller &operator = (poller &&) = delete;

    static void dispatch(epoll_event *events, size_t size);

    // @brief poller thread routine
    static void routine(poller &p);

    // @brief singleton lock
    static std::mutex s_lock_singleton;

    // @brief Weak reference to poller instance
    static std::weak_ptr<poller> s_instance;

    // @brief a pair of pipe, close write-end to notify poller thread to exit
    int m_exit_notifier[2];

    std::mutex m_lock_poller;

    std::condition_variable m_condition_variable;

    // @brief thread id
    std::thread m_tid;

  };


  std::weak_ptr<event_loop::poller>
    event_loop::poller::s_instance;

  std::mutex
    event_loop::poller::s_lock_singleton;

  //
  // @brief Dispatch each events to corresponding event loop
  // @param events array of events
  // @param size sizeof events
  //
  void event_loop::poller::dispatch(epoll_event *events, size_t size)
  {
    // Helper function that cast  epoll_event to reference of poll_target
    static auto caster = [](const epoll_event &x) -> poll_target &
    { return *static_cast<poll_target*>(x.data.ptr); };

    // Helper function that compare the event_loop instance of poll_targets
    // that stored in each epoll_event for sorting
    static auto cmper = [&](const epoll_event &lhs, const epoll_event &rhs)
    {
      return &caster(lhs).get_event_loop()
        < &caster(rhs).get_event_loop();
    };

    size_t begin = 0;
    size_t end = size;

    std::sort(events, events + end, cmper);

    while (begin < end)
    {
      event_loop &loop = caster(events[begin]).get_event_loop();

      // The upper bound of events that belong to same event_loop with
      // events[begin]
      size_t bound = std::upper_bound(events + begin, events + end,
          events[begin], cmper) - events;

      static auto callback_caster = [&] (epoll_event &p) -> callback &
      { return caster(p).m_receiver; };

      typedef boost::transform_iterator<decltype(callback_caster),
              epoll_event*> transform_iterator;

      transform_iterator from(events + begin, callback_caster);
      transform_iterator to(events + bound, callback_caster);

      begin = bound;
      unique_lock guard(loop.m_notifier_lock);
      if (loop.m_notified_callbacks.empty())
        loop.m_condition_variable.notify_one();

      // Dispatch the events
      loop.m_notified_callbacks.insert(loop.m_notified_callbacks.begin(),
          from, to);
    }
  }

  //
  // @brief Poller thread routine
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
    while (!will_exit)
    {
      epoll_event events[EVENT_BUFFER_SIZE];
      int ret = epoll_wait (epollfd, events, EVENT_BUFFER_SIZE, -1);

      if (ret <= 0)
        /* XXX: What to do*/
        continue;

      for (int i = 0; i < ret; i++)
      {
        if (events[i].data.ptr == nullptr)
        {
          char dummy;
          if (read(pipe_read_end, &dummy, sizeof(dummy)) <= 0)
            // We've been notified to exit
            will_exit = true;
        }
        else
          dispatch(events, ret);
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
    : base_timestamp(time_point::clock::now())
    , epollfd(epoll_create1(O_CLOEXEC))
    , m_lock_poller()
    , m_condition_variable()
    , m_tid()
    {
      if (!uq.owns_lock())
        throw std::exception();

      if (epollfd == -1)
        // FIXME:
        throw std::exception();

      int ret = pipe2(m_exit_notifier, O_CLOEXEC | O_NONBLOCK);
      if (ret == -1)
        throw std::exception();

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
    }
  catch (...)
  {
    // Clean up resouces that won't be automatically done by compiler
    if (epollfd != 0)
      close (epollfd);

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
  std::shared_ptr<event_loop::poller> event_loop::poller::get_instance()
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
  event_loop::callback_list event_loop::poller::poll(event_loop &loop)
  {
    callback_list tasks;
    tasks.swap(loop.m_upcoming_callbacks);

    if (loop.m_timed_callbacks.empty()) {
      unique_lock guard(m_lock_poller);
      if (loop.m_notified_callbacks.empty()) {
        if (tasks.empty() && loop.m_ref_counter != 0) {
          // No timer, just wait for other event
          do
            m_condition_variable.wait(guard);
          while (loop.m_notified_callbacks.empty());
          //tasks.swap(m_notified_event_list);
        }
      }
    } else {
      auto tp = loop.m_timed_callbacks.begin()->m_time_point;
      unique_lock guard(loop.m_poller->m_lock_poller);
      if (loop.m_notified_callbacks.empty()) {
        if (tasks.empty()) {
          // There are timers, wait until the first expire time point
          do {
            std::cv_status status = m_condition_variable.wait_until(guard, tp);
            if (status == std::cv_status::timeout) {
              auto get_callback = [](timed_callback &t)->callback&
              { return t.m_callback; };

              typedef boost::transform_iterator<decltype(get_callback),
                    decltype(loop.m_timed_callbacks.begin())> tranform_iterator;
              // Insert all timer event that have same time point with tp and
              // remove them from loop.m_timer_event_set
              auto tf = loop.m_timed_callbacks.begin();
              auto te = loop.m_timed_callbacks.upper_bound(*tf);
              tranform_iterator f(tf, get_callback);
              tranform_iterator e(tf, get_callback);
              tasks.insert(tasks.end(), f, e);
              loop.m_timed_callbacks.erase(tf, te);
              return tasks;
            }
          } while (loop.m_timed_callbacks.empty());
        }
      }
    }
    tasks.splice(tasks.end(), loop.m_notified_callbacks);
    return tasks;
  }

  event_loop::event_loop()
    : m_poller(poller::get_instance())
  { }

  event_loop::~event_loop()
  { }

  void event_loop::run()
  {
    for ( ; ; )
    {
      callback_list tasks = m_poller->poll(*this);
      if (tasks.empty())
        return;
      while (!tasks.empty())
      {
        auto x = tasks.begin();
        tasks.erase(x);
        x->m_handler();
      }
    }
  }

  void event_loop::dispatch(callback &ap)
  { m_upcoming_callbacks.push_back(ap); }

  void event_loop::dispatch(timed_callback &dp)
  { m_timed_callbacks.push_back(dp); }

}
