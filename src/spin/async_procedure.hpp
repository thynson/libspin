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

#ifndef __SPIN_EVENT_HPP_INCLUDED__
#define __SPIN_EVENT_HPP_INCLUDED__

#include "utils.hpp"

namespace spin {

  /**
   * @brief Abstract base class for async procedure
   */
  class __SPIN_EXPORT__ async_procedure : public list_node<async_procedure>
  {
  public:

    /**
     * @brief Destructor
     * Declared as virtual destucture so it's safe to inheriates
     */
    virtual ~async_procedure() noexcept {}

    /**
     * @brief Operator overload for ()
     * This is proxy method to #callback, which should be implemented by
     * inheriators
     */
    void operator () ()
    { this->callback(); }

  protected:

    /**
     * @brief Default constructor
     */
    async_procedure() = default;

    /**
     * @breif Move constructor
     */
    async_procedure(async_procedure &&tmp)
    { swap_nodes(tmp); }

    /**
     * @brief Move assign operator overload
     */
    async_procedure &operator = (async_procedure &&tmp)
    {
      if (this != &tmp)
        swap_nodes(tmp);
      return *this;
    }

    async_procedure(const async_procedure &) = delete;
    async_procedure &operator = (const async_procedure &) = delete;

  private:

    /**
     * @brief Pure virtual callback to be implemented by inheriators
     */
    virtual void callback() = 0;
  };


  /**
   * @brief Abstract base class for delayed async procedure
   */
  class __SPIN_EXPORT__ delayed_procedure : public async_procedure
                                          , public set_node<delayed_procedure>
  {
  public:

    virtual ~delayed_procedure()
    { }

    const time_point &get_time_point() const
    { return m_time_point; }


  protected:
    delayed_procedure(const time_point &tp)
      : async_procedure()
      , set_node<delayed_procedure>()
      , m_time_point(tp)
    { }

    delayed_procedure(delayed_procedure &&tmp)
      : async_procedure()
      , set_node<delayed_procedure>()
      , m_time_point(std::move(tmp.m_time_point))
    {
      async_procedure::swap_nodes(tmp);
      set_node<delayed_procedure>::swap_nodes(tmp);
    }

    delayed_procedure &operator = (delayed_procedure &&tmp)
    {
      async_procedure::swap_nodes(tmp);
      set_node<delayed_procedure>::swap_nodes(tmp);
      swap(m_time_point, tmp.m_time_point);
      return *this;
    }

    delayed_procedure(const delayed_procedure &dp) = delete;

    delayed_procedure &operator = (const delayed_procedure &tmp) = delete;

  private:
    time_point m_time_point;
  };

  inline bool operator < (const delayed_procedure &lhs,
                          const delayed_procedure &rhs)
  { return lhs.get_time_point() < rhs.get_time_point(); }

  inline bool operator > (const delayed_procedure &lhs,
                          const delayed_procedure &rhs)
  { return lhs.get_time_point() > rhs.get_time_point(); }

}
#endif
