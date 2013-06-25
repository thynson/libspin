
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

  class event_loop::poller_thread
  {
  public:
    ~poller_thread();

    static std::shared_ptr<poller_thread> get_instance();

    const time_point base_timestamp;
    const int epollfd;

    static std::mutex s_lock;
    static std::condition_variable s_condition_variable;

  private:
    poller_thread(unique_lock &uq);
    poller_thread(const poller_thread &) = delete;
    poller_thread(poller_thread &&) = delete;
    poller_thread &operator = (const poller_thread &) = delete;
    poller_thread &operator = (poller_thread &&) = delete;

    static void poller();
    static std::weak_ptr<poller_thread> s_instance;
    int m_pipe[2];
    std::thread m_tid;
  };


  std::weak_ptr<event_loop::poller_thread>
    event_loop::poller_thread::s_instance;

  std::mutex
    event_loop::poller_thread::s_lock;

  std::condition_variable
    event_loop::poller_thread::s_condition_variable;

  //
  // Poller thread routine
  //
  // This routine polls all epoll events and dispatch them to corresponding
  // evnet loop until this thread be notified to exit.
  //
  void event_loop::poller_thread::poller()
  {
    int epollfd;
    int pipe_read_end;

    {
      // Wait for initialization of poller_thread and get epollfd and pipe
      // notifier
      unique_lock guard(s_lock);
      s_condition_variable.notify_all();
      auto p = s_instance.lock();
      if (!p)
        return;
      epollfd = p->epollfd;
      pipe_read_end = p->m_pipe[0];
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
  // Constructor of poller_thread
  //
  // @note This constructor is declared as private, it should only be called
  // from poller_thread::get_instance() and poller_thread::s_lock should be
  // ensure locked.
  //
  event_loop::poller_thread::poller_thread(unique_lock &uq)
    try
    : base_timestamp(std::chrono::steady_clock::now())
    , epollfd(epoll_create1(O_CLOEXEC))
    , m_tid(poller)
    {
      if (!uq.owns_lock())
        throw std::exception();

      if (epollfd == -1) {
        // FIXME:
        throw std::exception();
      }
      int ret = pipe2(m_pipe, O_CLOEXEC | O_NONBLOCK);
      if (ret == -1) {
        throw std::exception();
      }

      epoll_event ev;
      ev.events = EPOLLIN | EPOLLET;
      ev.data.ptr = nullptr;

      ret = epoll_ctl (epollfd, EPOLL_CTL_ADD, m_pipe[0], &ev);
      if (ret == -1)
        throw std::exception();

      s_condition_variable.wait(uq);
    } catch (...) {
      // Clean up resouces that won't be automatically done by compiler
      if (epollfd != 0) {
        close (epollfd);
      }
      if (m_pipe[0] > 0)
        close (m_pipe[0]);

      if (m_pipe[1] > 0)
        close (m_pipe[1]);
      throw;
    }

  //
  // Destructor of poller_thread
  //
  // will notify the poller_thread to exit and then join it
  //
  event_loop::poller_thread::~poller_thread()
  {
    //std::lock_guard<std::mutex> lock(s_lock);
    close(m_pipe[1]);
    m_tid.join();
    close(m_pipe[0]);
    close(epollfd);
  }

  //
  // Get instance of poller_thread
  //
  // This function is a double-checked singleton helper function
  //
  std::shared_ptr<event_loop::poller_thread>
    event_loop::poller_thread::get_instance()
    {
      auto ret = s_instance.lock();
      if (!ret) {
        unique_lock lock(s_lock);
        std::atomic_thread_fence(std::memory_order_acquire);
        ret = s_instance.lock();
        if (!ret) {
          std::shared_ptr<poller_thread> p(new poller_thread(lock));
          s_instance = p;
          std::atomic_thread_fence(std::memory_order_release);
          return p;
        }
      }
      return ret;
    }

  event_loop::event_loop()
    : m_poller_thread(poller_thread::get_instance())
  { }

  event_loop::~event_loop()
  {
  }

  list<event> event_loop::wait_for_events()
  {
    list<event> tasks;
    tasks.swap(m_pending_event_list);

    if (m_timer_event_set.empty()) {
      unique_lock guard(poller_thread::s_lock);
      if (m_notified_event_list.empty()) {
        if (tasks.empty() && !m_io_event_list.empty()) {
          // No timer, just wait for other event
          do
            poller_thread::s_condition_variable.wait(guard);
          while (m_notified_event_list.empty());
          //tasks.swap(m_notified_event_list);
        }
      }
      tasks.splice(tasks.end(), m_notified_event_list);
    } else {
      auto &tp = m_timer_event_set.begin()->get_time_point();
      unique_lock guard(poller_thread::s_lock);
      if (m_notified_event_list.empty()) {
        if (tasks.empty()) {
          // There are times, wait until the latest expire time point
          do {
            std::cv_status status
              = poller_thread::s_condition_variable.wait_until(guard, tp);
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
