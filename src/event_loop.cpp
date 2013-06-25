
#include "spin/event_loop.hpp"
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

namespace spin {

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
