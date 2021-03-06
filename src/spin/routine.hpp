
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

/*
  Copyright (C) Malte Skarupke

  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <http://unlicense.org/>
*/


#ifndef __SPIN_ROUTINE_HPP_INCLUDED__
#define __SPIN_ROUTINE_HPP_INCLUDED__

#include <utility>
#include <type_traits>
#include <exception>
#include <typeinfo>
#include <memory>


namespace spin
{

  template<typename>
  struct force_routine_heap_allocation
    : std::false_type
  {
  };

  template<typename...>
  class routine;

  template<>
  class routine<void>
  {
    template<typename...>
    friend class routine;

    struct manager_storage_type;
    struct functor_padding
    {
    protected:
      size_t padding_first;
      size_t padding_second;
    };

    struct empty_struct
    {
    };

    template<typename ...Arguments>
    static void empty_call(functor_padding &, Arguments...) { }

    template<typename T, typename Allocator>
    struct is_inplace_allocated
    {
      static const bool value
        // so that it fits
        = sizeof(T) <= sizeof(functor_padding)
        // so that it will be aligned
        && std::alignment_of<functor_padding>::value
          % std::alignment_of<T>::value == 0
        // so that we can offer noexcept move
        && std::is_nothrow_move_constructible<T>::value
        // so that the user can override it
        && !force_routine_heap_allocation<T>::value;
    };

    template<typename T>
    static T to_functor(T && func)
    {
      return std::forward<T>(func);
    }

    template<typename Class, typename... Arguments>
    static auto to_functor(void (Class::*func)(Arguments...))
      -> decltype(std::mem_fn(func))
    {
      return std::mem_fn(func);
    }

    template<typename Class, typename... Arguments>
    static auto to_functor(void (Class::*func)(Arguments...) const)
      -> decltype(std::mem_fn(func))
    {
      return std::mem_fn(func);
    }

    template<typename T>
    struct functor_type
    {
      typedef decltype(to_functor(std::declval<T>())) type;
    };

    template<typename T>
    static bool is_null(const T &)
    {
      return false;
    }

    template<typename... Arguments>
    static bool is_null(void (* const & routine_pointer)(Arguments...))
    {
      return routine_pointer == nullptr;
    }

    template<typename Class, typename... Arguments>
    static bool is_null(void (Class::* const & routine_pointer)(Arguments...))
    {
      return routine_pointer == nullptr;
    }

    template<typename Class, typename... Arguments>
    static bool is_null(void (Class::* const & routine_pointer)(Arguments...) const)
    {
      return routine_pointer == nullptr;
    }

    template<typename, typename>
    struct is_valid_routine_argument
    {
      static constexpr bool value = false;
    };

    template<typename... Arguments>
    struct is_valid_routine_argument<routine<void (Arguments...)>, void (Arguments...)>
    {
      static constexpr bool value = false;
    };

    template<typename T, typename... Arguments>
    struct is_valid_routine_argument<T, void (Arguments...)>
    {
    private:
      template<typename U>
      static decltype(to_functor(std::declval<U>())(std::declval<Arguments>()...)) check(U *);
      template<typename>
      static empty_struct check(...);
    public:
      static constexpr bool value = std::is_same<decltype(check<T>(nullptr)), void>::value;
    };

    enum routine_manager_calls
    {
      call_move_and_destroy,
      call_copy,
      call_copy_functor_only,
      call_destroy,
      call_type_id,
      call_target,
    };

    typedef void *(*manager_type)(void *, void *, routine_manager_calls);

    struct manager_storage_type
    {
      template<typename Allocator>
      Allocator & get_allocator() noexcept
      {
        return reinterpret_cast<Allocator &>(manager);
      }
      template<typename Allocator>
      const Allocator & get_allocator() const noexcept
      {
        return reinterpret_cast<const Allocator &>(manager);
      }

      functor_padding functor;
      manager_type manager;
    };

    template<typename T, typename Allocator, typename Enable = void>
    struct routine_manager_inplace_specialization
    {
      template<typename... Arguments>
      static void call(functor_padding & storage, Arguments... arguments)
      {
        // do not call get_functor_ref because I want this routine to be fast
        // in debug when nothing gets inlined
        return reinterpret_cast<T &>(storage)(std::forward<Arguments>(arguments)...);
      }

      static void store_functor(manager_storage_type & storage, T to_store)
      {
        new (&get_functor_ref(storage)) T(std::forward<T>(to_store));
      }
      static void move_functor(manager_storage_type & lhs, manager_storage_type && rhs) noexcept
      {
        new (&get_functor_ref(lhs)) T(std::move(get_functor_ref(rhs)));
      }
      static void destroy_functor(Allocator &, manager_storage_type & storage) noexcept
      {
        get_functor_ref(storage).~T();
      }
      static T & get_functor_ref(manager_storage_type & storage) noexcept
      {
        return reinterpret_cast<T &>(storage.functor);
      }
    };

    template<typename T, typename Allocator>
    struct routine_manager_inplace_specialization<T, Allocator, typename std::enable_if<!is_inplace_allocated<T, Allocator>::value>::type>
    {
      template<typename... Arguments>
      static void call(functor_padding & storage, Arguments... arguments)
      {
        // do not call get_functor_ptr_ref because I want this routine to be fast
        // in debug when nothing gets inlined
        (*reinterpret_cast<typename std::allocator_traits<Allocator>::pointer &>(storage))(std::forward<Arguments>(arguments)...);
      }

      static void store_functor(manager_storage_type & self, T to_store)
      {
        Allocator & allocator = self.get_allocator<Allocator>();;
        static_assert(sizeof(typename std::allocator_traits<Allocator>::pointer) <= sizeof(self.functor), "The allocator's pointer type is too big");
        typename std::allocator_traits<Allocator>::pointer * ptr = new (&get_functor_ptr_ref(self)) typename std::allocator_traits<Allocator>::pointer(std::allocator_traits<Allocator>::allocate(allocator, 1));
        std::allocator_traits<Allocator>::construct(allocator, *ptr, std::forward<T>(to_store));
      }
      static void move_functor(manager_storage_type & lhs, manager_storage_type && rhs) noexcept
      {
        static_assert(std::is_nothrow_move_constructible<typename std::allocator_traits<Allocator>::pointer>::value, "we can't offer a noexcept swap if the pointer type is not nothrow move constructible");
        new (&get_functor_ptr_ref(lhs)) typename std::allocator_traits<Allocator>::pointer(std::move(get_functor_ptr_ref(rhs)));
        // this next assignment makes the destroy routine easier
        get_functor_ptr_ref(rhs) = nullptr;
      }
      static void destroy_functor(Allocator & allocator, manager_storage_type & storage) noexcept
      {
        typename std::allocator_traits<Allocator>::pointer & pointer = get_functor_ptr_ref(storage);
        if (!pointer) return;
        std::allocator_traits<Allocator>::destroy(allocator, pointer);
        std::allocator_traits<Allocator>::deallocate(allocator, pointer, 1);
      }
      static T & get_functor_ref(manager_storage_type & storage) noexcept
      {
        return *get_functor_ptr_ref(storage);
      }
      static typename std::allocator_traits<Allocator>::pointer & get_functor_ptr_ref(manager_storage_type & storage) noexcept
      {
        return reinterpret_cast<typename std::allocator_traits<Allocator>::pointer &>(storage.functor);
      }
    };

    template<typename T, typename Allocator>
    static void create_manager(manager_storage_type & storage, Allocator && allocator)
    {
      new (&storage.get_allocator<Allocator>()) Allocator(std::move(allocator));
      storage.manager = &routine_manager<T, Allocator>;
    }

    // this routine acts as a vtable. it is an optimization to prevent
    // code-bloat from rtti. see the documentation of boost::function
    template<typename T, typename Allocator>
    static void * routine_manager(void * first_arg, void * second_arg, routine_manager_calls call_type)
    {
      typedef routine_manager_inplace_specialization<T, Allocator> specialization;
      static_assert(std::is_empty<Allocator>::value, "the allocator must be an empty class because I don't have space for it");
      switch(call_type)
      {
      case call_move_and_destroy:
      {
        manager_storage_type & lhs = *static_cast<manager_storage_type *>(first_arg);
        manager_storage_type & rhs = *static_cast<manager_storage_type *>(second_arg);
        specialization::move_functor(lhs, std::move(rhs));
        specialization::destroy_functor(rhs.get_allocator<Allocator>(), rhs);
        create_manager<T, Allocator>(lhs, std::move(rhs.get_allocator<Allocator>()));
        rhs.get_allocator<Allocator>().~Allocator();
        return nullptr;
      }
      case call_copy:
      {
        manager_storage_type & lhs = *static_cast<manager_storage_type *>(first_arg);
        manager_storage_type & rhs = *static_cast<manager_storage_type *>(second_arg);
        create_manager<T, Allocator>(lhs, Allocator(rhs.get_allocator<Allocator>()));
        specialization::store_functor(lhs, specialization::get_functor_ref(rhs));
        return nullptr;
      }
      case call_destroy:
      {
        manager_storage_type & self = *static_cast<manager_storage_type *>(first_arg);
        specialization::destroy_functor(self.get_allocator<Allocator>(), self);
        self.get_allocator<Allocator>().~Allocator();
        return nullptr;
      }
      case call_copy_functor_only:
        specialization::store_functor(*static_cast<manager_storage_type *>(first_arg), specialization::get_functor_ref(*static_cast<manager_storage_type *>(second_arg)));
        return nullptr;
      case call_type_id:
        return const_cast<std::type_info *>(&typeid(T));
      case call_target:
        if (*static_cast<const std::type_info *>(second_arg) == typeid(T))
          return &specialization::get_functor_ref(*static_cast<manager_storage_type *>(first_arg));
        else
          return nullptr;
      default:
        return nullptr;
      }
    }

    template<typename...>
    struct typedeffer
    { };

    template<typename Argument>
    struct typedeffer<Argument>
    {
      typedef Argument argument_type;
    };

    template<typename First_Argument, typename Second_Argument>
    struct typedeffer<First_Argument, Second_Argument>
    {
      typedef First_Argument first_argument_type;
      typedef Second_Argument second_argument_type;
    };
  };

  template<typename... Arguments>
  class routine
    : public routine<void>::typedeffer<Arguments...>
  {
    using result_type = void;
    using detail = routine<void>;
  public:
    routine() noexcept
    {
      initialize_empty();
    }

    routine(routine && other) noexcept
    {
      initialize_empty();
      swap(other);
    }

    routine(const routine & other)
      : call(other.call)
    {
      other.manager_storage.manager(&manager_storage, &other.manager_storage, detail::call_copy);
    }

    template<typename T>
    routine(T functor)
      noexcept(detail::is_inplace_allocated<T, std::allocator<typename detail::functor_type<T>::type>>::value)
    {
      static_assert(detail::is_valid_routine_argument<T, void(Arguments...)>::value,
          "T is not valid functor type");
      if (detail::is_null(functor))
      {
        initialize_empty();
      }
      else
      {
        typedef typename detail::functor_type<T>::type functor_type;
        initialize(detail::to_functor(std::forward<T>(functor)), std::allocator<functor_type>());
      }
    }
    template<typename Allocator>
    routine(std::allocator_arg_t, const Allocator &)
    {
      // ignore the allocator because I don't allocate
      initialize_empty();
    }
    template<typename Allocator>
    routine(std::allocator_arg_t, const Allocator &, std::nullptr_t)
    {
      // ignore the allocator because I don't allocate
      initialize_empty();
    }
    template<typename Allocator, typename T>
    routine(std::allocator_arg_t, const Allocator & allocator, T functor)
      noexcept(detail::is_inplace_allocated<T, std::allocator<typename detail::functor_type<T>::type>>::value)
    {
      static_assert(detail::is_valid_routine_argument<T, void(Arguments...)>::value,
          "T is not valid functor type");
      if (detail::is_null(functor))
      {
        initialize_empty();
      }
      else
      {
        initialize(detail::to_functor(std::forward<T>(functor)), Allocator(allocator));
      }
    }
    template<typename Allocator>
    routine(std::allocator_arg_t, const Allocator & allocator, const routine & other)
      : call(other.call)
    {
      typedef typename std::allocator_traits<Allocator>::template rebind_alloc<routine>::other MyAllocator;

      // first try to see if the allocator matches the target type
      detail::manager_type manager_for_allocator = &detail::routine_manager<typename std::allocator_traits<Allocator>::value_type, Allocator>;
      if (other.manager_storage.manager == manager_for_allocator)
      {
        detail::create_manager<typename std::allocator_traits<Allocator>::value_type, Allocator>(manager_storage, Allocator(allocator));
        manager_for_allocator(&manager_storage, const_cast<detail::manager_storage_type *>(&other.manager_storage), detail::call_copy_functor_only);
      }
      // if it does not, try to see if the target contains my type. this
      // breaks the recursion of the last case. otherwise repeated copies
      // would allocate more and more memory
      else if (other.manager_storage.manager == &detail::routine_manager<routine, MyAllocator>)
      {
        detail::create_manager<routine, MyAllocator>(manager_storage, MyAllocator(allocator));
        detail::routine_manager<routine, MyAllocator>(&manager_storage, const_cast<detail::manager_storage_type *>(&other.manager_storage), detail::call_copy_functor_only);
      }
      else
      {
        // else store the other routine as my target
        initialize(other, MyAllocator(allocator));
      }
    }
    template<typename Allocator>
    routine(std::allocator_arg_t, const Allocator &, routine && other) noexcept
    {
      // ignore the allocator because I don't allocate
      initialize_empty();
      swap(other);
    }

    routine & operator=(routine other) noexcept
    {
      swap(other);
      return *this;
    }

    ~routine() noexcept
    {
      manager_storage.manager(&manager_storage, nullptr, detail::call_destroy);
    }

    void operator()(Arguments... arguments) const
    {
      call(manager_storage.functor, std::forward<Arguments>(arguments)...);
    }

    template<typename T, typename Allocator>
    void assign(T && functor, const Allocator & allocator)
      noexcept(detail::is_inplace_allocated<T, std::allocator<typename detail::functor_type<T>::type>>::value)
    {
      routine(std::allocator_arg, allocator, functor).swap(*this);
    }

    void swap(routine & other) noexcept
    {
      detail::manager_storage_type temp_storage;
      other.manager_storage.manager(&temp_storage, &other.manager_storage, detail::call_move_and_destroy);
      manager_storage.manager(&other.manager_storage, &manager_storage, detail::call_move_and_destroy);
      temp_storage.manager(&manager_storage, &temp_storage, detail::call_move_and_destroy);

      std::swap(call, other.call);
    }


    const std::type_info & target_type() const noexcept
    {
      return *static_cast<std::type_info *>(manager_storage.manager(nullptr, nullptr, detail::call_type_id));
    }

    template<typename T>
    T * target() noexcept
    {
      return static_cast<T *>(manager_storage.manager(&manager_storage, const_cast<std::type_info *>(&typeid(T)), detail::call_target));
    }

    template<typename T>
    const T * target() const noexcept
    {
      return static_cast<const T *>(manager_storage.manager(const_cast<detail::manager_storage_type *>(&manager_storage), const_cast<std::type_info *>(&typeid(T)), detail::call_target));
    }

  private:
    mutable detail::manager_storage_type manager_storage;
    void (*call)(detail::functor_padding &, Arguments...);

    template<typename T, typename Allocator>
    void initialize(T functor, Allocator && allocator)
    {
      call = &detail::routine_manager_inplace_specialization<T, Allocator>::template call<Arguments...>;
      detail::create_manager<T, Allocator>(manager_storage, std::forward<Allocator>(allocator));
      detail::routine_manager_inplace_specialization<T, Allocator>::store_functor(manager_storage, std::forward<T>(functor));
    }

    typedef void (*Empty_Function_Type)(Arguments...);
    void initialize_empty() noexcept
    {
      typedef std::allocator<Empty_Function_Type> Allocator;
      static_assert(detail::is_inplace_allocated<Empty_Function_Type, Allocator>::value, "The empty routine should benefit from small functor optimization");

      detail::create_manager<Empty_Function_Type, Allocator>(manager_storage, Allocator());
      detail::routine_manager_inplace_specialization<Empty_Function_Type, Allocator>::store_functor(manager_storage, nullptr);
      call = &detail::empty_call<Arguments...>;
    }
  };

  template<typename T>
  void swap(routine<T> & lhs, routine<T> & rhs)
  {
    lhs.swap(rhs);
  }

} // end namespace func

namespace std
{
  template<typename... Arguments, typename Allocator>
  struct uses_allocator<spin::routine<Arguments...>, Allocator>
    : std::true_type
  { };
}

#endif
