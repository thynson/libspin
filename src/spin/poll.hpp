/*
 * Copyright (C) 2014 LAN Xingcan
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

#ifndef __SPIN_POLL_HPP_INCLUDED__
#define __SPIN_POLL_HPP_INCLUDED__

#include <spin/system.hpp>
#include <spin/intruse/list.hpp>

#include <memory>

namespace spin
{
  class poller;


  /**
   * @brief Abstract pollable interface
   */
  class __SPIN_EXPORT__ basic_pollable
  {
    friend class poller;
  public:
    basic_pollable(basic_pollable&&) = delete;
    basic_pollable(const basic_pollable&) = delete;
  protected:
    basic_pollable() = default;
    virtual ~basic_pollable() = 0;
    virtual void on_readable() = 0;
    virtual void on_writable() = 0;
    virtual void on_error() = 0;
  };

  /**
   * @brief General pollable object
   */
  class __SPIN_EXPORT__ pollable : public basic_pollable
  {
  protected:

    struct poll_argument_readable_t {  } poll_argument_readable;
    struct poll_argument_writable_t {  } poll_argument_writable;
    struct poll_argument_duplex_t   {  } poll_argument_duplex;

    /** @brief Construct a read only pollable instance */
    pollable(std::shared_ptr<poller> p, system_handle handle, poll_argument_readable_t);

    /** @brief Construct a write only pollable instance */
    pollable(std::shared_ptr<poller> p, system_handle handle, poll_argument_writable_t);

    /** @brief Construct a pollable instance that is both readable and
     * writable */
    pollable(std::shared_ptr<poller> p, system_handle handle, poll_argument_duplex_t);

    pollable(pollable &&) = delete;

    pollable &operator = (pollable &&) = delete;

    ~pollable() = default;

    /**
     * @brief A noop implement the basic_pollable::on_readable
     * @note If this pollable is readable, this member function should be
     * overrided by its child class
     */
    void on_readable() override {}

    /**
     * @brief A noop implement the basic_pollable::on_writable
     * @note If this pollable is readable, this member function should be
     * overrided by its child class
     */
    void on_writable() override {}

    /**
     * @todo its behaviour is undesigned
     */
    void on_error() override {}

    const system_handle &get_handle() const noexcept
    { return m_handle; }

  private:
    std::shared_ptr<poller> m_poller;
    system_handle m_handle;
  };

  class __SPIN_EXPORT__ poller : private basic_pollable
  {
  public:

    poller();

    virtual ~poller() override;

    system_handle &get_poll_handle() noexcept
    { return m_poll_handle; }

    void interrupt();

    void poll(bool allow_blocking);

  private:

    void on_readable() override { }
    void on_writable() override { }
    void on_error() override { }

    system_handle m_poll_handle;
    system_handle m_interrupter;
  };
}

#endif
