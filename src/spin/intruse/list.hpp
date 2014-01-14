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

#ifndef __SPIN_INTRUSE_LIST_HPP_INCLUDED__
#define __SPIN_INTRUSE_LIST_HPP_INCLUDED__

#include <spin/functional.hpp>

#include <iterator>
#include <type_traits>

namespace spin
{
  namespace intruse
  {
    /* Forward declaration */
    template<typename Inheriator, typename Tag = Inheriator> class list;
    template<typename Inheriator, typename Tag = Inheriator> class list_node;
    template<typename Inheriator, typename Tag> class list_iterator;

    template<typename Inheriator, typename Tag>
    class list_node
    {
      friend class ::spin::intruse::list<Inheriator, Tag>;
      friend class ::spin::intruse::list_iterator<Inheriator, Tag>;
    public:

      /**
       * @brief Unlink a node from container
       * @param node The node to be unlink
       * @returns Whether the unlink operation actually performed, for node
       * already unlinked, false will be returned.
       */
      static bool unlink(list_node &node) noexcept
      {
        bool ret = false;
        if (node.m_prev)
        {
          node.m_prev->m_next = node.m_next;
          ret = true;
        }
        if (node.m_next)
        {
          node.m_next->m_prev = node.m_prev;
          ret = true;
        }
        node.m_prev = nullptr;
        node.m_next = nullptr;
        return ret;
      }

      /** @breif Test a node is linked into a list */
      static bool is_linked(const list_node &node) noexcept
      { return node.m_next != nullptr && node.m_prev != nullptr; }

      list_node() noexcept
        : m_prev(nullptr)
        , m_next(nullptr)
      { }

      ~list_node() noexcept
      { unlink(*this); }

      list_node(const list_node &) = delete;

      list_node(list_node && n) noexcept
        : list_node()
      { swap(*this, n); }

      list_node &operator = (const list_node &) = delete;

      list_node &operator = (list_node &&node) noexcept
      {
        this->~list_node();
        new (this) list_node(std::move(node));
        return *this;
      }

    private:

      /** @brief Swap two node */
      static void swap(list_node &lhs, list_node &rhs)
        noexcept
      {
        if (&lhs == &rhs) return;

        if (lhs.m_prev)
        { lhs.m_prev->m_next = &rhs; }

        if (lhs.m_next)
        { lhs.m_next->m_prev = &rhs; }

        if (rhs.m_prev)
        { rhs.m_prev->m_next = &lhs; }

        if (rhs.m_next)
        { rhs.m_next->m_prev = &lhs; }

        std::swap(lhs.m_prev, rhs.m_prev);
        std::swap(lhs.m_next, rhs.m_next);
      }


      list_node(list_node *prev, list_node *next) noexcept
        : m_prev(prev)
        , m_next(next)
      { }

      list_node *m_prev;
      list_node *m_next;
    };

    template<typename Inheriator, typename Tag>
    class list_iterator
    {
    public:

      // Nested type similar with STL
      using value_type        = Inheriator;
      using reference         = Inheriator &;
      using pointer           = Inheriator *;
      using difference_type   = std::ptrdiff_t;
      using iterator_category = std::bidirectional_iterator_tag;
      using node_type         = list_node<Inheriator, Tag>;

      explicit list_iterator (node_type *node) noexcept
        : m_node (node)
      { }

      ~list_iterator() noexcept = default;

      reference operator * () const noexcept { return *internal_cast(); }

      pointer operator -> () const noexcept { return internal_cast(); }

      list_iterator &operator ++ () noexcept
      { m_node = m_node->m_next; return *this; }

      list_iterator &operator -- () noexcept
      { m_node = m_node->m_prev; return *this; }

      list_iterator operator ++ (int) noexcept
      {
        list_iterator l(m_node);
        ++(*this);
        return l;
      }

      list_iterator operator -- (int) noexcept
      {
        list_iterator l(m_node);
        --(*this);
        return l;
      }

      friend bool operator ==
      (const list_iterator &l, const list_iterator &r) noexcept
      { return l.m_node == r.m_node; }

      friend bool operator !=
      (const list_iterator &l, const list_iterator &r) noexcept
      { return !(l == r); }

    private:

      Inheriator *internal_cast() const noexcept
      { return static_cast<pointer>(m_node); }
      node_type *m_node;
    };

    template<typename Inheriator, typename Tag>
    class list_const_iterator
    {
    public:

      using iterator_category = std::bidirectional_iterator_tag;
      using value_type        = const Inheriator;
      using reference         = const Inheriator &;
      using pointer           = const Inheriator *;
      using difference_type   = std::ptrdiff_t;
      using node_type         = list_node<Inheriator, Tag>;


      list_const_iterator (const node_type *node) noexcept
        : m_node (node)
      {}

      ~list_const_iterator() noexcept = default;

      reference operator * () const noexcept { return *internal_cast(); }

      pointer operator -> () const noexcept { return internal_cast(); }

      /**
       * @breif Iterate forward
       * @returns Iterator itself
       */
      list_const_iterator &operator ++ () const noexcept
      { m_node = m_node->m_next; return *this; }

      /**
       * @brief Iterate backward
       * @returns Iterator itself
       */
      list_const_iterator &operator -- () const noexcept
      { m_node = m_node->m_prev; return *this; }

      /**
       * @breif Iterate forward
       * @returns An iterator has the origin position
       */
      list_const_iterator operator ++ (int) const noexcept
      { return iterator(m_node->m_next); }

      /**
       * @breif Iterate backward
       * @returns An iterator has the origin position
       */
      list_const_iterator operator -- (int) const noexcept
      { return iterator(m_node->m_prev); }

      /** @breif Test whether the two iterator is equal */
      friend bool operator == (const list_const_iterator &l,
          const list_const_iterator &r) noexcept
      { return l->m_node == r->m_node; }

      /** @breif Test whether the two iterator is not equal */
      friend bool operator != (const list_const_iterator &l,
          const list_const_iterator &r) noexcept
      { return !(l == r); }

    private:

      /** @brief Cast to the pointer of the type that holds the node */
      pointer internal_cast() const noexcept
      { return static_cast<pointer>(m_node); }

      const node_type *m_node;
    };

    /**
     * @brief Intrusive list container
     * @tparam T the type (as well as its derived type) this list can hold
     * @tparam Tag, the tag of list_node for T, which used to avoid conflict
     * when T inheriate multriple list_node.
     *
     * Different from std::list, intrusive list dost not allocate memory
     * internally to hold elements, but require containing type, say T derived
     * from list_node<T, Tag=T>.
     */
    template<typename T, typename Tag>
    class list
    {
    public:

      // Nested type, similar with STL
      using iterator                = list_iterator<T, Tag>;
      using const_iterator          = list_const_iterator<T, Tag>;
      using reverse_iterator        = std::reverse_iterator<iterator>;
      using const_reverse_iterator  = std::reverse_iterator<const_iterator>;
      using value_type              = T;
      using pointer                 = T *;
      using reference               = T &;
      using const_pointer           = const T *;
      using const_reference         = const T &;
      using size_type               = size_t;
      using difference_type         = std::ptrdiff_t;
      using node_type               = list_node<T, Tag>;

      /** @brief Default constructor */
      list () noexcept
        : m_head(nullptr, &m_tail)
        , m_tail(&m_head, nullptr)
      { }

      /** @brief Construct list and splice elements from another list */
      list (iterator b, iterator e) noexcept
        : list()
      { splice(end(), b, e); }


      /** @brief Construct with elements from input iterator */
      template<typename InputIterator>
      list(InputIterator b, InputIterator e) noexcept(noexcept(*b) && noexcept(b++))
        : list()
      {
        while (b != e)
        {
          auto &n = *b++;
          push_back(n);
        }
      }

      /** @brief Destructor */
      ~list() noexcept
      { clear(); }

      /** @brief Move constructor */
      list(list &&other) noexcept
      { swap(other); }

      /** @brief Move assign operator */
      list &operator = (list &&other) noexcept
      { swap(other); }

      list(const list &) = delete;

      list &operator = (const list &) = delete;


      // Capacity

      bool empty() const noexcept
      { return m_head.m_next == &m_tail; }

      size_type size() const noexcept
      {
        size_type s = 0;
        iterator i = begin();
        while (i++ != end()) s++;
        return s;
      }

      // Access

      reference front() noexcept
      { return *begin(); }

      const_reference front() const noexcept
      { return *begin(); }

      reference back() noexcept
      { return *rbegin(); }

      const_reference back() const noexcept
      { return *rbegin(); }

      iterator begin() noexcept
      { return iterator(m_head.m_next); }

      reverse_iterator rbegin() noexcept
      { return reverse_iterator(end()); }

      const_iterator cbegin() const noexcept
      { return const_iterator(m_head.m_next); }

      const_iterator begin() const noexcept
      { return const_iterator(m_head.m_nexgt); }

      const_reverse_iterator rbegin() const noexcept
      { return const_reverse_iterator(end()); }

      const_reverse_iterator crbegin() const noexcept
      { return const_reverse_iterator(end()); }

      iterator end() noexcept
      { return iterator(&m_tail); }

      reverse_iterator rend() noexcept
      { return reverse_iterator(begin()); }

      const_iterator end() const noexcept
      { return const_iterator(&m_tail); }

      const_iterator cend() const noexcept
      { return const_iterator(&m_tail); }

      const_reverse_iterator rend() const noexcept
      { return const_reverse_iterator(begin()); }

      const_reverse_iterator crend() const noexcept
      { return const_reverse_iterator(begin()); }


      // Modifier

      /**
       * @breif Insert given value to specified position
       * @param pos The specified position
       * @param ref Reference to the element to be inserted
       */
      void insert(iterator pos, reference ref) noexcept
      {
        node_type &nref = ref;

        // Unlink first
        node_type::unlink(nref);
        nref.m_prev = pos->m_prev;
        nref.m_prev->m_next = &nref;
        pos->m_prev = &nref;
        nref.m_next = &(*pos);
      }


      /**
       * @brief Insert elements from range to specified position
       * @tparam InputIterator The type of iterators which represent the range
       * @param pos The specified position
       * @param b The begin of the range
       * @param e The end of the range
       */
      template<typename InputIterator>
        void insert(iterator pos, InputIterator b, InputIterator e)
          noexcept(noexcept(list(b, e)))
        {
          list l(b, e);
          splice(pos, l);
        }

      /**
       * @brief Erase an elements at specified position
       * @param pos The position of the elements
       */
      void erase(iterator pos) noexcept
      {
        node_type::unlink(*pos);
      }

      /**
       * @brief Erase all elements within specified range
       * @param b The begin of the range
       * @param e The end of the range
       */
      void erase(iterator b, iterator e) noexcept
      {
        while (b != e)
          erase(b++);
      }

      /**
       * @brief Insert a elements to the front of this list
       * @param ref The refernce to the element to be inserted
       */
      void push_front(reference ref) noexcept
      {
        insert(begin(), ref);
      }

      /**
       * @brief Insert a elements to the back of this list
       * @param ref The refernce to the element to be inserted
       */
      void push_back(reference ref) noexcept
      {
        insert(end(), ref);
      }

      /**
       * @brief Remove elements equals to specified value
       * @param value The specified value
       */
      void remove(const value_type &value)
        noexcept(noexcept(value == value))
      {
        auto b = begin(), e = end();
        while (b != e)
        {
          auto i = b++;
          if (*i == value) erase(i);
        }
      }

      /**
       * @breif Remove elements accepted by specified predicate
       * @param pred The specified predicate
       */
      template<typename Predicate>
      void remove_if(Predicate &&pred)
        noexcept(noexcept(pred(std::declval<T>())))
      {
        auto b = begin(), e = end();
        while (b != e)
        {
          auto i = b++;
          if (pred(*i)) erase(i);
        }
      }

      /**
       * @brief Swap elements with another list, overload for rvalue reference
       * @param l The list to be swapped with
       */
      void swap(list &&l) noexcept
      { swap(l); }

      /**
       * @brief Swap elements with another list
       * @param l The list to be swapped with
       */
      void swap(list &l) noexcept
      {
        list *lhs = this, *rhs = &l;

        if (lhs == rhs) return;

        if (lhs->empty())
        {
          if (rhs->empty())
            return;
          else
            std::swap(lhs, rhs);
        }

        if (rhs->empty())
        {
          lhs->m_head.m_next->m_prev = &rhs->m_head;
          lhs->m_tail.m_prev->m_next = &rhs->m_tail;
          rhs->m_head.m_next = lhs->m_head.m_next;
          rhs->m_tail.m_prev = lhs->m_tail.m_prev;
          lhs->m_head.m_next = &lhs->m_tail;
          lhs->m_tail.m_prev = &lhs->m_head;
        }
        else
        {
          std::swap(lhs->m_head.m_next->m_prev,
                    rhs->m_head.m_next->m_prev);
          std::swap(lhs->m_tail.m_prev->m_next,
                    rhs->m_tail.m_prev->m_next);
          std::swap(lhs->m_head.m_next, rhs->m_head.m_next);
          std::swap(lhs->m_tail.m_prev, rhs->m_tail.m_prev);
        }
      }

      /** @param Reverse the order of this list */
      void reverse() noexcept
      {
        list x;
        auto b = begin(), e = end();
        while (b != e)
        {
          auto &n = *b++;
          node_type::unlink(n);
          x.push_front(n);
        }
        swap(x);
      }

      /** @brief Erase all elements from this list */
      void clear() noexcept
      {
        auto *ptr = m_head.m_next;
        while (ptr != &m_tail)
        {
          auto *tmp = ptr->m_next;
          ptr->m_next = nullptr;
          ptr->m_prev = nullptr;
          ptr = tmp;
        }

        m_head.m_next = &m_tail;
        m_tail.m_prev = &m_head;
      }

      /**
       * @breif Transfer all elements from l to position pos in this list
       * @note l shall be a distinct list
       */
      void splice(iterator pos, list &l) noexcept
      {
        splice(pos, l.begin(), l.end());
      }

      /**
       * @breif Transfer all elements from l to position pos in this list
       * @note l shall be a distinct list
       */
      void splice(iterator pos, list &&l) noexcept
      {
        splice(pos, l.begin(), l.end());
      }

      /**
       * @breif Transfer all elements ranged from b to e to position pos
       * in this list
       * @note l shall in a distinct list
       */
      void splice(iterator pos, iterator b, iterator e) noexcept
      {
        if (b == e) return;

        node_type &x = *(b->m_prev), &y = *(e->m_prev);
        x.m_next = &(*e);
        e->m_prev = &x;

        b->m_prev = pos->m_prev;
        y.m_next = &(*pos);
        b->m_prev->m_next = &(*b);
        y.m_next->m_prev = &y;
      }

      /**
       * @brief Sort this list with specified strict weak ordering comparer
       * @param cmp The specified weak ordering comparer
       */
      template<typename StrictWeakOrderingComparer>
        void sort(StrictWeakOrderingComparer &&cmp)
          noexcept(noexcept(cmp(std::declval<T>(), std::declval<T>())))
        {
          if (empty() || ++begin() == end()) return;

          // Copy from libstdc++, seems it's merge sort

          list carry;
          list tmp[64];
          list *fill = &tmp[0];
          list *counter;

          do
          {
            T &n = *begin();
            node_type::unlink(n);
            carry.insert(carry.begin(), n);

            for (counter = &tmp[0];
                counter != fill && !counter->empty();
                ++counter)
            {
              counter->merge(carry,
                  std::forward<StrictWeakOrderingComparer>(cmp));
              counter->swap(carry);
            }

            carry.swap(*counter);
            if (counter == fill)
              ++fill;
          }
          while(!empty());

          for (counter = &tmp[1]; counter != fill; ++counter)
            counter->merge(*(counter - 1),
                std::forward<StrictWeakOrderingComparer>(cmp));
          swap(*(fill - 1));
        }

      /** @brief Sort elements in this list with spin::less */
      void sort()
        noexcept(noexcept(std::declval<list>().sort(less<T>())))
      { sort(less<T>()); }

      /**
       * @breif Merge sorted list
       * @param l The list to be merged
       */
      void merge(list &l)
        noexcept(noexcept(l.merge(l, less<T>())))
      { merge(l, less<T>()); }

      /**
       * @breif Merge a sorted list, overload for rvalue refernece
       * @param l The list to be merged
       */
      void merge(list &&l)
        noexcept(noexcept(l.merge(l)))
      { merge(l); }

      /**
       * @breif Merge sorted list l with specified strict weak ordering
       * comparer cmper
       * @param l List to be merged
       * @param cmper The specified strict weak ordering comparer
       */
      template<typename Comparer>
      void merge(list &l, Comparer &&cmper)
        noexcept(noexcept(l.merge(l, l.begin(), l.end(),
                std::forward<Comparer>(cmper))))
      {
        merge(l, l.begin(), l.end(),
            std::forward<Comparer>(cmper));
      }

      /**
       * @breif Merge a sorted part of list l from b to e with specified
       * strict weak ordering comparer cmper
       * @param l List to be merged
       * @param b List to be merged
       * @param cmper The specified strict weak ordering comparer
       */
      template<typename Comparer>
      void merge(list &l, iterator b, iterator e,
          Comparer &&cmper)
        noexcept(noexcept(cmper(*b, *e)))
      {
        if (this == &l) return;

        auto p = begin(), q = end();

        while (p != q && b != e)
          if (std::forward<Comparer>(cmper)(*b, *p))
          {
            auto &n = *b++;
            node_type::unlink(n);
            insert(p, n);
          }
          else
          { ++p; }

        if (b != e)
          splice(q, b, e);
      }

      /** @breif Erase duplicate elements */
      void unique()
        noexcept(noexcept(std::declval<list>().unique(less<T>())))
      { unique(less<T>()); }

      /**
       * @breif Erase duplicate elements in range [b,e)
       * @param b Begin of the range
       * @param e End of the range
       */
      void unique(iterator b, iterator e)
        noexcept(noexcept(std::declval<list>().unique(b, e, less<T>())))
      { unique(b, e, less<T>()); }

      /**
       * @breif Erase duplicate elements with specified binary predicate
       * @param binpred The specified binary predicate
       */
      template<typename BinaryPredicate>
      void unique(BinaryPredicate && binpred)
        noexcept(noexcept(std::declval<list>().unique
              (std::declval<iterator>(), std::declval<iterator>(),
               std::forward<BinaryPredicate>(binpred))))
      {
        unique(begin(), end(),
            std::forward<BinaryPredicate>(binpred));
      }

      /**
       * @breif Erase duplicate elements with specified binary predicate in
       * rnage [b, e)
       * @param b Begin of the range
       * @param e End of the range
       * @param binpred The specified binary predicate
       */
      template<typename BinaryPredicate>
      void unique(iterator b, iterator e, BinaryPredicate &&binpred)
        noexcept(noexcept(binpred(*b, *e)))
      {
        if (b == e) return;
        auto n = b;
        while(++n != b)
        {
          if (binpred(*b, *n))
            erase(n);
          else
            b = n;
          n = b;
        }
      }


    private:
      node_type m_head;
      node_type m_tail;
    };


    template<typename T, typename Tag>
      inline bool operator == (const list<T, Tag> &x,
          const list<T, Tag> &y) noexcept
      {
        auto i = x.begin(), j = y.begin();
        auto m = x.end(), n = y.end();

        while (i != m && j != n && *i == *j)
        { ++i; ++j; }

        return i == m && j == n;
      }

    template<typename T, typename Tag>
      inline bool operator != (const list<T, Tag> &x,
          const list<T, Tag> &y) noexcept
      { return !(x == y); }

    template<typename T, typename Tag>
      inline bool operator < (const list<T, Tag> &x,
          const list<T, Tag> &y) noexcept
      {
        if (&x == &y) return false;
        return std::lexicographical_compare(x.begin(), x.end(),
            y.begin(), y.end());
      }

    template<typename T, typename Tag>
      inline bool operator <= (const list<T, Tag> &x,
          const list<T, Tag> &y) noexcept
      { return !(y < x); }

    template<typename T, typename Tag>
      inline bool operator > (const list<T, Tag> &x,
          const list<T, Tag> &y) noexcept
      { return y < x; }

    template<typename T, typename Tag>
      inline bool operator >= (const list<T, Tag> &x,
          const list<T, Tag> &y) noexcept
      { return !(y > x); }

  }
}

#endif
