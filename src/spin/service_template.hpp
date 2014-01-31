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

#ifndef __SPIN_SERVICE_TEMPLATE_HPP__
#define __SPIN_SERVICE_TEMPLATE_HPP__

#include <spin/intruse/rbtree.hpp>
#include <memory>


namespace spin
{

  template<typename Service, typename Identity>
  class service_template : public intruse::rbtree_node<Identity, Service>
                         , public std::enable_shared_from_this<Service>
  {
  protected:
    struct __SPIN_INTERNAL__ service_tag {};

    static intruse::rbtree<Identity, Service> instance_table;

  protected:
    service_template(Identity && identity)
      : intruse::rbtree_node<Identity, Service>(std::forward<Identity>(identity))
      , std::enable_shared_from_this<Service>()
    { }

    Identity &get_identity() noexcept
    { return intruse::rbtree_node<Identity, Service>::get_index(*this); }

    const Identity &get_identity() const noexcept
    { return intruse::rbtree_node<Identity, Service>::get_index(*this); }

  public:

    ~service_template() = default;

    service_template(const service_template &) = delete;

    service_template(service_template &&) = delete;

    service_template& operator = (const service_template &) = delete;

    service_template& operator = (service_template &&) = delete;

    static std::shared_ptr<Service> get(event_loop &el)
    {
      auto i = instance_table.find(&el);
      if (i == instance_table.end())
      {
        auto s = std::make_shared<Service>(el, service_tag());
        instance_table.insert(*s);
        return s;
      }
      else
        return i->shared_from_this();

    }

  };

  template<typename Service, typename Identity>
  intruse::rbtree<Identity, Service>
  service_template<Service, Identity>::instance_table;
}
#endif
