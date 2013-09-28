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

#include <mutex>
#include <memory>

namespace spin
{

  /**
   * @brief Singleton auxiliary template class
   * @tparam Inheriant the singleton class itself that use this template
   * @tparam Volatile whether the singleton is volatile (not const-volatility
   * type specifier of C++) that the instance is hold by a std::weak_ptr so
   * once refernce counter become zero, it will be destroyed immediately
   * @note We need the constructor/destructor of Inheriant be at least
   * protected to access it
   */
  template<class Inheriant, bool Volatile = false>
  class enable_singleton
  {
  protected:

    /**
     * @brief Singleton factory
     */
    class singleton_factory
    {
    public:

      /**
       * @brief Instantiate functor
       */
      std::shared_ptr<Inheriant> operator () ()
      {
        if (instance)
          return instance;
        std::unique_lock<std::mutex> guard(lock);
        if (instance)
          return instance;

        instance.reset(new target_type,
            [](Inheriant *p)
            { delete static_cast<target_type*>(p); });

        return instance;
      }
    private:
      class target_type : public Inheriant {};
    };

  private:
    // We hide the constructors, but child class need to access it
    friend Inheriant;

    enable_singleton() {}

    enable_singleton(const enable_singleton &) = delete;

    enable_singleton(enable_singleton &&) = delete;

    enable_singleton &operator = (const enable_singleton &) = delete;

    enable_singleton &operator = (enable_singleton &&) = delete;

    static std::shared_ptr<Inheriant> instance;
    static std::mutex lock;
  };


  /**
   * @brief Partial specialization of enable_singleton
   * @tparam Inheriant the singleton class itself that use this template
   */
  template<class Inheriant>
  class enable_singleton<Inheriant, true>
  {
  protected:
    /**
     * @brief Singleton factory
     */
    class singleton_factory
    {
      friend class enable_singleton<Inheriant, true>;
    public:

      /**
       * @brief Instansiate functor
       */
      std::shared_ptr<Inheriant> operator () ()
      {
        auto ret = instance.lock();

        if (ret)
        { return ret; }

        std::unique_lock<std::mutex> guard(lock);

        ret = instance.lock();

        if (ret)
        { return ret; }

        ret = std::make_shared<target_type>();
        instance = ret;
        return std::move(ret); // returns a different type
      }
    private:
      class target_type : public Inheriant { };
    };
  private:
    // We hide the constructors, but child class need to access it
    friend Inheriant;

    enable_singleton() {}

    enable_singleton(const enable_singleton &) = delete;

    enable_singleton(enable_singleton &&) = delete;

    enable_singleton &operator = (const enable_singleton &) = delete;

    enable_singleton &operator = (enable_singleton &&) = delete;

    static std::weak_ptr<typename singleton_factory::target_type> instance;
    static std::mutex lock;
  };

  template<class Inheriant, bool Volatile>
    std::shared_ptr<Inheriant> enable_singleton<Inheriant, Volatile>::instance;

  template<class Inheriant, bool Volatile>
    std::mutex enable_singleton<Inheriant, Volatile>::lock;

  template<class Inheriant>
    std::weak_ptr<typename
    enable_singleton<Inheriant, true>::singleton_factory::target_type>
    enable_singleton<Inheriant, true>::instance;

  template<class Inheriant>
    std::mutex enable_singleton<Inheriant, true>::lock;

}

#endif
