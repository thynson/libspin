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

#ifndef __SPIN_SINGLETON_HELPER_HPP_INCLUDED__
#define __SPIN_SINGLETON_HELPER_HPP_INCLUDED__

#include <spin/environment.hpp>

#include <mutex>
#include <memory>
#include <type_traits>

namespace spin
{


  /**
   * @brief Singleton proxy class
   *
   * This class need constructor of proxied class be accessible, this is
   * usually done by declare friend class. Furthermore, proxied class can
   * define a constexpr static member named volatility to specify singleton
   * object life-time policy. Different from volatile keywords in C, C++, here
   * volatility means whether the object is stored with shared pointer or weak
   * pointer, for volatility is true, weak pointer is used to hold the
   * instance thus once there is no shared pointer hold a reference to this
   * instance, it
   * will be destructed immediately.
   */
  template<typename T>
  class singleton
  {
  protected:
    struct __SPIN_INTERNAL__ singleton_tag {};

  private:
    // SFINAE to check if volatility is present

    // If a T that its volatility member is a static constexpr member with
    // boolean type and accessiable for us
    template<typename X>
    static constexpr bool check_volatility(X *, bool v = T::volatility)
    { return v; }

    // Otherwise default volatility is false
    static constexpr bool check_volatility(...)
    { return false; }

    // Finish SFINAE
    static constexpr bool volatility = check_volatility(nullptr);

    static std::shared_ptr<T> make_shared()
    {
      // Construct shared pointer first to ensure exception safety
      std::shared_ptr<T> x;
      x.reset(new T(singleton_tag()));
      return x;
    }

    struct f
    {
      static std::shared_ptr<T> get_instance()
      {
        static std::shared_ptr<T> instance;
        static std::mutex lock;
        if (instance) return instance;
        std::lock_guard<decltype(lock)> guard(lock);
        if (instance) return instance;
        instance = make_shared();
        return instance;
      }
    };

    struct t
    {
      static std::shared_ptr<T> get_instance ()
      {
        static std::weak_ptr<T> instance;
        static std::mutex lock;
        auto x = instance.lock();
        if (x) return x;
        std::lock_guard<decltype(lock)> guard(lock);
        x = instance.lock();
        if (x) return x;
        x = make_shared();
        instance = x;
        return x;
      }
    };
  public:
    static std::shared_ptr<T> get_instance()
    {
      return std::conditional<volatility, t, f>::type::get_instance();
    }
  };
}

#endif
