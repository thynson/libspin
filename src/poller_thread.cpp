#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include "spin/event_loop.hpp"

namespace spin {

  std::weak_ptr<poller_thread> poller_thread::s_instance;
  std::mutex poller_thread::s_lock;
  std::condition_variable poller_thread::s_condition_variable;

  //
  // Poller thread routine
  //
  // This routine polls all epoll events and dispatch them to corresponding
  // evnet loop until this thread be notified to exit.
  //
  void poller_thread::poller()
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
  poller_thread::poller_thread(unique_lock &uq)
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
  poller_thread::~poller_thread()
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
  std::shared_ptr<poller_thread> poller_thread::get_instance()
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

}
