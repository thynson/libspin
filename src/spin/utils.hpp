/*
 * Copyright (C) 2013  LAN Xingcan
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

#ifndef __SPIN_UTILS_HPP_INCLUDED__
#define __SPIN_UTILS_HPP_INCLUDED__

#include <spin/environment.hpp>
#include <spin/time.hpp>
#include <spin/spin_lock.hpp>

#include <boost/intrusive/list.hpp>
#include <boost/intrusive/set.hpp>


namespace spin {

  using intrusive_list_node =
    boost::intrusive::list_member_hook<
    boost::intrusive::link_mode<boost::intrusive::auto_unlink>>;

  template<typename Type, intrusive_list_node Type::* PointerToMember>
    using intrusive_list = boost::intrusive::list<Type
    , boost::intrusive::member_hook<Type, intrusive_list_node, PointerToMember>
    , boost::intrusive::constant_time_size<false>>;

  using intrusive_set_node =
    boost::intrusive::set_member_hook<
    boost::intrusive::link_mode<boost::intrusive::auto_unlink>>;

  template<typename Type, intrusive_set_node Type::* PointerToMember>
    using intrusive_multiset =
    boost::intrusive::multiset<Type
    , boost::intrusive::member_hook<Type, intrusive_set_node, PointerToMember>
    , boost::intrusive::constant_time_size<false>>;

  template<typename Type, intrusive_set_node Type::* PointerToMember>
    using intrusive_set =
    boost::intrusive::set<Type
    , boost::intrusive::member_hook<Type, intrusive_set_node, PointerToMember>
    , boost::intrusive::constant_time_size<false>>;

}

#endif
