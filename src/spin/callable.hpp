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
   * @brief Abstract callable class
   */
  class __SPIN_EXPORT__ callable: public list_node<callable>
  {
  public:

    /**
     * @brief Destructor
     * Declared as virtual destucture so it's safe to inheriates
     */
    virtual ~callable() noexcept {}

    /**
     * @brief Operator overload for ()
     * This is proxy method to #callback, which should be implemented by
     * inheriators
     */
    void operator () ()
    { this->callback(); }
  private:

    /**
     * @brief Pure virtual callback to be implemented by inheriators
     */
    virtual void callback() = 0;
  };
}
#endif