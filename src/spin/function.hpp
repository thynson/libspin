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

#ifndef __SPIN_FUNCTION_HPP_INCLUDED__
#define __SPIN_FUNCTION_HPP_INCLUDED__

#include <spin/functional.hpp>

#include <memory>
#include <cassert>

namespace spin
{

  class bad_function_call : public std::exception
  {
    const char *what() const noexcept override
    { return "Bad function call"; }
  };

  /**
   * @brief libspin implementation of function
   * @note I don't want to reinvent yet another wheel, but at least GNU C++
   * 4.8.* doesn't properly implement it any way.
   */
  template<typename T>
  class function;


  /** @brief spin::function Internal stuff */
  template<>
  class function<void>
  {
    template<typename T> friend class function;

    struct functor_padding
    {
    private:
      struct A { virtual ~A() = 0; };
      struct B : virtual public A { virtual ~B() = 0; };
      struct C : virtual public A { virtual ~C() = 0; };
      struct D : public B, public C {
        virtual ~D() = 0;
      };


      void *padding_first;
      union {
        void (A::*p1) ();
        void (B::*p2) ();
        void (D::*p3) ();
      } padding_second;
    };


    template<typename T>
    struct is_inplace_allocated
    {
      constexpr static bool value
        = sizeof (T) < sizeof (functor_padding)
        && (std::alignment_of<functor_padding>::value
          % std::alignment_of<T>::value == 0)
        && (std::is_nothrow_move_constructible<T>::value);
    };

    template<typename Result, typename ...Args>
    class base_function
    {
    public:
      using result_type = Result;
    };

    template<typename Result, typename Argument>
    class base_function<Result, Argument>
    {
    public:
      using result_type = ResultType;
      using argument_type = ArgumentType;
    };

    template<typename Result, typename FirstArgument, typename SecondArgument>
    class base_function<Result, FirstArgument, SecondArgument>
    {
    public:
      using result_type = Result;
      using first_argument_type = FirstArgument;
      using second_argument_type = SecondArgument
    };

    template<typename T>
    struct functor_type
    {
      using type = T;//decltype(make_functor(std::declval<T>()));
    };

    template<typename ResultType, typename Class, typename ...Arguments>
    struct functor_type<ResultType (Class::*)(Arguments...)>
    {
      using type = decltype(
          std::mem_fun(std::declval<ResultType (Class::*)(Arguments...)>()));
    };

    template<typename ResultType, typename Class, typename ...Arguments>
    struct functor_type<ResultType (Class::*)(Arguments...) const>
    {
      using type = decltype(
          std::mem_fun(std::declval<ResultType (Class::*)(Arguments...) const>()));
    };

    template<typename, typename>
    struct is_valid_function_argument
    {
      static const bool value = false;
    };

    template<typename Result, typename... Arguments>
    struct is_valid_function_argument<function<Result (Arguments...)>, Result (Arguments...)>
    {
      static const bool value = false;
    };

    template<typename T, typename Result, typename... Arguments>
    struct is_valid_function_argument<T, Result (Arguments...)>
    {
    private:
      template<typename U>
      static decltype(to_functor(std::declval<U>())(std::declval<Arguments>()...)) check(U *);
      template<typename>
      static std::function<void> check(...);
    public:
      static const bool value = std::is_convertible<decltype(check<T>(nullptr)), Result>::value;
    };

    template<typename T>
    static bool is_null(const T &) noexcept
    { return false; }

    template<typename Result, typename... Arguments>
    static bool is_null(Result (* const & function_pointer)(Arguments...)) noexcept
    { return function_pointer == nullptr; }

    template<typename Result, typename Class, typename... Arguments>
    static bool is_null(Result (Class::* const & function_pointer)(Arguments...)) noexcept
    { return function_pointer == nullptr; }

    template<typename Result, typename Class, typename... Arguments>
    static bool is_null(Result (Class::* const & function_pointer)(Arguments...) const) noexcept
    { return function_pointer == nullptr; }

    /**
     * @brief default value for m_invoker member of function, throws
     * bad_function_call
     */
    template<typename Result, typename ...Arguments>
    static Result empty_call(Arguments...)
    { throw bad_function_call(); }

  };

  template<typename Result, class ...Arguments>
  class function<Result(Arguments...)>
    : public function<void>::base_function<Result, Arguments...>
  {
    using detail = function<void>;
  public:

    /** @brief Default constructor */
    function() noexcept
      : function(nullptr)
    {}

    /** @brief Constructor which accepts nullptr */
    function(std::nullptr_t)
      : function(std::allocator<void>(), nullptr)
    {}

    /** @brief Constructor which accepts raw functor */
    template<typename Callable>
    function(Callable functor)
      noexcept(detail::is_inplace_allocated<Callable>::value)
      : function(std::allocator_arg, std::allocator<void>(),
          std::move(functor))
    {  }

    /** @brief Constructor with specified allocator */
    template<typename Allocator>
    function(std::allocator_arg_t t, const Allocator &allocator) noexcept
      : function(t, allocator, nullptr)
    { }

    /** @brief Constructor accepts nullptr with specified constructor */
    template<typename Allocator>
    function(std::allocator_arg_t t, const Allocator &allocator,
        std::nullptr_t) noexcept
      : function(t, allocator, &detail::empty_call<Result, Arguments...>)
    { }

    /** @brief Constructor accepts nullptr with specified constructor */
    template<typename Callable, typename Allocator>
    function(std::allocator_arg_t, const Allocator &allocator,
        Callable callable)
      noexcept(detail::is_inplace_allocated<Callable>::value)
    {
      if (detail::is_null(callable))
        new (&get_implementation_ref())
          implementation_selector<Result(*)(Arguments...), std::allocator<void>>
          (detail::empty_call<Result, Arguments...>, std::allocator<void>());
      else
        new (&get_implementation_ref())
          implementation_selector<Callable, Allocator>
          (std::move(callable), allocator);
    }

    ~function()
    { get_implementation_ref().~base_implemetation(); }


    function(function &&other) noexcept
    {
      other.get_implementation_ref().move(m_store);
      new (&other.m_store)
        implementation_selector<Result(*)(Arguments...), std::allocator<void>>
        (detail::empty_call<Result, Arguments...>, std::allocator<void>());
    }


    Result operator () (Arguments && ... args) const
    {
      return get_implementation_ref().invoke(std::forward<Arguments>(args)...);
    }

  private:

    struct base_implemetation
    {
      virtual ~base_implemetation() {};
      virtual Result invoke (Arguments...) = 0;
      virtual void move(detail::functor_padding &) = 0;
    };

    template<typename Callable>
    struct inplace_allocated_implementation : public base_implemetation
    {
      Callable m_functor;

      template<typename Allocator>
      inplace_allocated_implementation(Callable callable, Allocator) noexcept
        : m_functor(std::move(callable))
      { }

      virtual ~inplace_allocated_implementation() override {};

      virtual Result invoke(Arguments ...args) override
      { return m_functor(std::forward<Arguments>(args)...); }

      virtual void move(detail::functor_padding &store) override
      {
        new (&store) inplace_allocated_implementation(std::move(*this));
      }
    };

    template<typename Callable, typename Allocator>
    struct outplace_allocated_implementation : public base_implemetation
    {
      using allocator_type
        = typename std::allocator_traits<Allocator>
        ::template rebind_alloc<Callable>;

      allocator_type m_allocator;
      Callable *m_functor;

      outplace_allocated_implementation(Callable callable, const Allocator &alloc)
        : m_allocator(alloc)
        , m_functor(m_allocator.allocate(1))
      { new (m_functor) Callable(std::move(callable)); }

      outplace_allocated_implementation(outplace_allocated_implementation && other) noexcept
        : m_allocator(std::move(m_allocator))
        , m_functor(nullptr)
      {
        std::swap(m_functor, other.m_functor);
      }


      ~outplace_allocated_implementation()
      {
        m_functor->~Callable();
        m_allocator.deallocate(m_functor, 1);
      }

      virtual Result invoke(Arguments ...args) override
      { return (*m_functor) (std::forward<Arguments>(args)...); }

      virtual void move(detail::functor_padding &store) override
      {
        new (&store) outplace_allocated_implementation(std::move(*this));
      }
    };

    template<typename Callable, typename Allocator>
    struct implementation_selector
      : public std::conditional<detail::is_inplace_allocated<Callable>::value,
        inplace_allocated_implementation<Callable>,
        outplace_allocated_implementation<Callable, Allocator>>::type
    {
      using base_type = typename std::conditional<detail::is_inplace_allocated<Callable>::value,
        inplace_allocated_implementation<Callable>,
        outplace_allocated_implementation<Callable, Allocator>>::type;

      implementation_selector(Callable callable, const Allocator &allocator)
        : base_type(std::forward<Callable>(callable), allocator)
      { }

    };


    mutable detail::functor_padding m_store;

    base_implemetation &get_implementation_ref() const
    {
      return reinterpret_cast<base_implemetation&>(m_store);
    }
  };
}

#endif
