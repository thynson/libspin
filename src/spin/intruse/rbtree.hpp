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

#ifndef __SPIN_INTRUSE_RBTREE_HPP_INCLUDED__
#define __SPIN_INTRUSE_RBTREE_HPP_INCLUDED__

#include <spin/environment.hpp>
#include <spin/functional.hpp>

#include <iterator>
#include <type_traits>
#include <cassert>

namespace spin
{
  namespace intruse
  {

    template<typename Key, typename Comparer = less<Key>, typename Tag = void>
    class rbtree;

    template<typename Key, typename Comparer = less<Key>, typename Tag = void>
    class rbtree_node;

    template<typename Key, typename Comparer, typename Tag>
    class rbtree_iterator;

    template<typename Key, typename Comparer, typename Tag>
    class rbtree_const_iterator;

    /**
     * @brief rbtree_node specialization for void key type, used as base class
     * of all the other rbtree_node
     */
    template<>
    class __SPIN_EXPORT__ rbtree_node<void, void>
    {
      template<typename Key, typename Comparer, typename Tag>
      friend class rbtree;

      template<typename Key, typename Comparer, typename Tag>
      friend class rbtree_node;

      template<typename Key, typename Comparer, typename Tag>
      friend class rbtree_iterator;

    protected:

      /** @brief Auxilary class used for rbtree(container_tag) */
      static struct container_tag {} const container;

      /** Initialize this node as container node */
      rbtree_node(container_tag) noexcept;

      /** @brief Default constructor */
      rbtree_node() noexcept;

      /** @brief Destructor */
      ~rbtree_node() noexcept;

      /** @brief Move constructor */
      rbtree_node(rbtree_node &&n) noexcept;

      /** @brief Assign operator overload for rvalue */
      rbtree_node &operator = (rbtree_node &&n) noexcept;

      rbtree_node(const rbtree_node &) = delete;

      rbtree_node &operator = (const rbtree_node &n) = delete;


      // Status

      /** @brief Test if a node is container node or root node, should be
       * faster than is_container_node and is_root_node */
      bool is_container_or_root() const noexcept
      { return m_p != nullptr && m_p->m_p == this; }

      /** @brief Test if a node is the root node of rbtree */
      bool is_root_node() const noexcept
      { return is_container_or_root() && !this->m_is_container; }

      /** @brief Test if container is empty */
      bool is_empty_container_node() const noexcept
      { return m_p == this; }

      /** @brief Test if a node is linked into rbtree */
      bool is_linked() const noexcept;

      // Navigation

      /**
       * @brief Get root node from container node
       * @note User is responsible to ensure this node is container node
       * @returns the root node of this rbtree, or nullptr if this tree is an
       * empty tree
       */
      const rbtree_node *get_root_node_from_container_node() const noexcept;

      /**
       * @brief Get root node from container node
       * @note User is responsible to ensure this node is container node
       * @returns the root node of this rbtree, or nullptr if this tree is an
       * empty tree
       */
      rbtree_node *get_root_node_from_container_node() noexcept;

      /** @brief Get root node */
      rbtree_node *get_root_node() noexcept;

      /** @brief Get root node */
      const rbtree_node *get_root_node() const noexcept;

      /**
       * @brief Return the back node of container via container node
       * @note If this node is not container node, the result is undefined
       */
      rbtree_node *back_of_container() noexcept;

      /**
       * @brief Return the back node of container via container node
       * @note If this node is not container node, the result is undefined
       */
      const rbtree_node *back_of_container() const noexcept;

      /**
       * @brief Return the front node of container via container node
       * @note If this node is not container node, the result is undefined
       */
      rbtree_node *front_of_container() noexcept;

      /**
       * @brief Return the front node of container via container node
       * @note If this node is not container node, the result is undefined
       */
      const rbtree_node *front_of_container() const noexcept;

      /**
       * @brief Get successor of this node
       * @note if this node is container node, the result is undefined
       */
      rbtree_node *next() noexcept;

      /**
       * @brief Get successor of this node
       * @note if this node is container node, the result is undefined
       */
      const rbtree_node *next() const noexcept;

      /**
       * @brief Get predecessor of this node
       * @note if this node is container node, the result is undefined
       */
      rbtree_node *prev() noexcept;

      /**
       * @brief Get predecesssor of this node
       * @note if this node is container node, the result is undefined
       */
      const rbtree_node *prev() const noexcept;

      /** @brief Get front node of this subtree */
      rbtree_node *front() noexcept;

      /** @brief Get back node of this subtree */
      rbtree_node *back() noexcept;

      /** @brief Get front node of this subtree */
      const rbtree_node *front() const noexcept;

      /** @brief Get back node of this subtree */
      const rbtree_node *back() const noexcept;

      // Rotation

      /** @brief Do left rotate */
      void lrotate() noexcept;

      /** @brief Do right rotate */
      void rrotate() noexcept;

      // Insertion

      /**
       * @brief Insert a node as left child of this node
       * @note User is responsible for ensure this node does not have left
       * child
       */
      void insert_to_left(rbtree_node *node) noexcept;

      /**
       * @brief Insert a node as right child of this node
       * @note User is responsible for ensure this node does not have right
       * child
       */
      void insert_to_right(rbtree_node *node) noexcept;

      /**
       * @brief Insert a node as root node
       * @note User is responible to ensure this node is container node
       */
      void insert_root_node(rbtree_node *node) noexcept;

      /** @brief Insert a node as predecessor of this node */
      void insert_before (rbtree_node *node) noexcept;

      /** @brief Insert a node as successor of this node */
      void insert_after(rbtree_node *node) noexcept;

      /** @breif Do clean up work after unlink */
      void unlink_cleanup() noexcept;

      /**
       * @brief Unlink this node from the rbtree
       * @returns the successor of this node, or container node if this node
       * is the last node in the tree
       * @note User is responsible to ensure this node is linked and is not
       * container node,
       * @see use unlink_checked
       */
      rbtree_node *unlink() noexcept;

      /**
       * @breif Unlink this node from the tree if this node is already linked
       * @returns the successor of this node, or nullptr if this node is not
       * linked into a rbtree
       * @note However it doesn't not check if this node is container node of
       * a rbtree, and unlink a container node is always cause undefined
       * behaviours
       */
      rbtree_node *unlink_checked() noexcept;

      /**
       * @brief Search and execute
       * @tparam Key the type indexed
       * @tparam Caster type whose instances are callable that cast an
       * rbtree_node<void, void> reference to Key type
       * @tparam Comparer type whose instances are callable that compare two
       * instances of Key type
       * @tparam Callable type whose instances are are callable with two
       * argument, the type of first argument is rbtree_node<void, void>, and
       * the type of second argument is boolean indicates that the compare
       * result of specified key with the first parameter pass to routine
       * @param hint The entry for search
       * @param key The key to be searched with
       * @param caster An instance of caster
       * @param comparer An instance of Comparer
       * @param routine An instance of Callable
       * @note For empty tree, routine will not be called
       */
      template<typename Key, typename Caster, typename Comparer, typename Callable>
      static void search_and_execute(rbtree_node<void, void> &hint, const Key &key,
          Caster && caster, Comparer && comparer, Callable && routine)
        noexcept(noexcept(std::forward<Caster>(caster)(hint))
            && noexcept(std::declval<Comparer>()(key, key))
            && noexcept(std::declval<Callable>()(hint, false)))
      {
        auto *p = &hint;

        if (p->m_is_container)
        {
          if (p->is_empty_container_node())
            return ;
          else
            p = p->get_root_node_from_container_node();
        }

        auto cmper = [&] (rbtree_node *node) -> bool
        {
          return std::forward<Comparer>(comparer)(
              std::forward<Caster>(caster)(*node), key);
        };

        bool hint_result = cmper(p);

        auto done = [&] () -> void
        { std::forward<Callable>(routine)(*p, hint_result); };

        // Search for younest common parent
        if (hint_result)
        {
          while (!p->m_p->m_is_container)
          {
            if (p == p->m_p->m_l)
            {
              if (cmper(p->m_p))
              {
                p = p->m_p;
                continue;
              }
            }

            if (!p->m_r->m_is_container)
            {
              if (cmper(p->m_r))
                p = p->m_r;
              else
                break;
            }
            else
            {
              done();
              return ;
            }
          }
        }
        else
        {
          while (!p->m_p->m_is_container)
          {
            if (p == p->m_p->m_r)
            {
              if (!cmper(p->m_p))
              {
                p = p->m_p;
                continue;
              }
            }

            if (!p->m_l->m_is_container)
            {
              if (!cmper(p->m_l))
                p = p->m_l;
              else
                break;
            }
            else
            {
              done();
              return ;
            }
          }
        }

        for ( ; ; )
        {
          if (hint_result)
          {
            if (p->m_has_r)
              p = p->m_r;
            else
            {
              done();
              return ;
            }
          }
          else
          {
            if (p->m_has_l)
              p = p->m_l;
            else
            {
              done();
              return ;
            }
          }

          hint_result = cmper(p);
        }

      }

      template<typename Key, typename Caster, typename Comparer, typename Callable>
      static void search_and_execute(const rbtree_node<void, void> &hint, const Key &key,
          Caster && caster, Comparer && comparer, Callable && routine)
        noexcept(noexcept(std::forward<Caster>(caster)(hint))
            && noexcept(std::declval<Comparer>()(key, key))
            && noexcept(std::declval<Callable>()(hint, false)))
      {
        auto *p = &hint;

        if (p->m_is_container)
        {
          if (p->is_empty_container_node())
            return ;
          else
            p = p->get_root_node_from_container_node();
        }

        auto cmper = [&] (rbtree_node *node) -> bool
        {
          return std::forward<Comparer>(comparer)(
              std::forward<Caster>(caster)(*node), key);
        };

        bool hint_result = cmper(p);

        auto done = [&] () -> void
        { std::forward<Callable>(routine)(*p, hint_result); };

        // Search for younest common parent
        if (hint_result)
        {
          while (!p->m_p->m_is_container)
          {
            if (p == p->m_p->m_l)
            {
              if (cmper(p->m_p))
              {
                p = p->m_p;
                continue;
              }
            }

            if (!p->m_r->m_is_container)
            {
              if (cmper(p->m_r))
                p = p->m_r;
              else
                break;
            }
            else
            {
              done();
              return ;
            }
          }
        }
        else
        {
          while (!p->m_p->m_is_container)
          {
            if (p == p->m_p->m_r)
            {
              if (!cmper(p->m_p))
              {
                p = p->m_p;
                continue;
              }
            }

            if (!p->m_l->m_is_container)
            {
              if (!cmper(p->m_l))
                p = p->m_l;
              else
                break;
            }
            else
            {
              done();
              return ;
            }
          }
        }

        for ( ; ; )
        {
          if (hint_result)
          {
            if (p->m_has_r)
              p = p->m_r;
            else
            {
              done();
              return ;
            }
          }
          else
          {
            if (p->m_has_l)
              p = p->m_l;
            else
            {
              done();
              return ;
            }
          }

          hint_result = cmper(p);
        }

      }
    private:

      /** @brief Rebalance a node after insertion */
      static void __SPIN_INTERNAL__
      rebalance_for_insertion(rbtree_node *node) noexcept;

      /** @brief Rebalance the tree after unlink */
      static void __SPIN_INTERNAL__
      rebalance_for_unlink(rbtree_node *node) noexcept;

      void __SPIN_INTERNAL__ transfer_link(rbtree_node &node) noexcept;

      /**
       * @brief Swap two nodes in the tree
       * @note This will generally break the order or nodes, so it's declared
       * privately
       */
      static void __SPIN_INTERNAL__
      swap_nodes(rbtree_node<void, void> &lhs, rbtree_node<void, void> &rhs)
      noexcept;

      rbtree_node *m_p;
      rbtree_node *m_l;
      rbtree_node *m_r;
      bool m_has_l;
      bool m_has_r;
      bool m_is_red;
      bool m_is_container;
    };

    template<typename Key, typename Comparer, typename Tag>
    class rbtree_node : private rbtree_node<void, void>
    {
      friend class rbtree<Key, Comparer, Tag>;
      friend class rbtree_iterator<Key, Comparer, Tag>;
      friend class rbtree_const_iterator<Key, Comparer, Tag>;

      /** @brief Node for container */
      struct container_node : public rbtree_node<void, void>
      {
      public:
        container_node()
          : rbtree_node<void, void>(rbtree_node<void, void>::container)
        { }
      };

    public:

      /* @brief get key of this node */
      const Key &get_key() const noexcept
      { return m_key; }

      /* @brief update key of this node */
      Key update_key(Key key)
        noexcept(noexcept(std::swap(key, key)))
      {
        auto *p = unlink_checked();
        std::swap(key, m_key);
        if (p)
          insert_after(*p, *this); // FIXME: duplicated or not
        return key;
      }

      static void unlink(rbtree_node &node) noexcept
      { node.rbtree_node<void, void>::unlink(); }

      static rbtree_node<void, void> *
      lower_bound(rbtree_node<void, void> &hint, const Key &key)
      noexcept(noexcept(std::declval<Comparer>()(key, key)))
      {
        auto *p = &hint;
        search_and_execute(hint, key,
            // caster
            [] (rbtree_node<void, void> &n)
            { return internal_cast(&n)->get_key(); },

            // comparer
            [] (const Key &lhs, const Key &rhs)
            { return cmper(lhs, rhs); },

            [&] (rbtree_node<void, void> &node, bool result)
            {
              if (result)
                p = node.m_r;
              else
                p = &node;
            }
          );
        return p;
      }

      static rbtree_node<void, void> *
      upper_bound(rbtree_node<void, void> &hint, const Key &key)
      noexcept(noexcept(std::declval<Comparer>()(key, key)))
      {
        auto *p = &hint;
        search_and_execute(hint, key,
            // caster
            [] (rbtree_node<void, void> &n)
            { return internal_cast(&n)->get_key(); },

            // comparer
            [] (const Key &lhs, const Key &rhs)
            { return !cmper(rhs, lhs); },

            [&] (rbtree_node<void, void> &node, bool result)
            {
              if (result)
                p = node.m_r;
              else
                p = &node;
            }
          );
        return p;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param hint_node The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note Use is responsible to ensure hint_node is already attached to a
       * tree; and if duplicate is permitted, the node is insert after any
       * node duplicate with this node
       */
      static void insert_after(rbtree_node<void, void> &hint_node,
          rbtree_node &node)
      noexcept(
          noexcept(std::declval<Comparer>()(node.get_key(), node.get_key())))
      {
        if (hint_node.m_is_container && hint_node.is_empty_container_node())
        {
          hint_node.insert_root_node(&node);
          return ;
        }

        search_and_execute(hint_node, node.get_key(),

            // caster
            [] (rbtree_node<void, void> &n)
            { return internal_cast(&n)->get_key(); },

            // comparer
            [] (const Key &lhs, const Key &rhs)
            { return !cmper(rhs, lhs); },

            // routine
            [&] (rbtree_node<void, void> &n, bool result)
            {
              if (n.m_is_container)
              {
                n.insert_root_node(&node);
              }
              else if (result)
              {
                if (n.m_has_r)
                  n.next()->insert_before(&node);
                else
                  n.insert_after(&node);
              }
              else
              {
                if (n.m_has_l)
                  n.prev()->insert_after(&node);
                else
                  n.insert_before(&node);
              }
            }
          );
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param hint_node The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note Use is responsible to ensure hint_node is already attached to a
       * tree; and if duplicate is permitted, the node is insert before any
       * node duplicate with this node
       */
      static void insert_before(rbtree_node<void, void> &hint_node,
          rbtree_node &node)
      noexcept(noexcept(std::declval<Comparer>()(node.get_key(), node.get_key())))
      {
        if (hint_node.m_is_container && hint_node.is_empty_container_node())
        {
          hint_node.insert_root_node(&node);
          return ;
        }

        search_and_execute(hint_node, node.get_key(),

            // caster
            [] (rbtree_node<void, void> &n)
            { return internal_cast(&n)->get_key(); },

            // comparer
            [] (const Key &lhs, const Key &rhs)
            { return cmper(lhs, rhs); },

            // routine
            [&] (rbtree_node<void, void> &n, bool result)
            {
              if (n.m_is_container)
              {
                n.insert_root_node(&node);
              }
              else if (result)
              {
                if (n.m_has_r)
                  n.next()->insert_before(&node);
                else
                  n.insert_after(&node);
              }
              else
              {
                if (n.m_has_l)
                  n.prev()->insert_after(&node);
                else
                  n.insert_before(&node);
              }
            }
          );
      }

      template<typename ...Args>
      rbtree_node(Args && ...args)
        noexcept(noexcept(Key(std::forward<Args>(args)...)))
        : rbtree_node<void, void>()
        , m_key(std::forward<Args>(args)...)
      { }

      ~rbtree_node() noexcept(noexcept(std::declval<Key>().~Key())) = default;

      rbtree_node(rbtree_node &&n)
        noexcept(noexcept(Key(std::declval<Key>())))
        : rbtree_node<void, void>(std::move(n))
        , m_key(std::move(n.m_key))
      { }

      rbtree_node &operator = (rbtree_node &&n)
        noexcept(noexcept(std::swap(n.m_key, n.m_key)))
      {
        std::swap(m_key, n.m_key);
        swap(*this, n);
        return *this;
      }

      rbtree_node(const rbtree_node &) = delete;
      rbtree_node operator = (const rbtree_node &) = delete;

      friend bool operator < (const rbtree_node &lhs, const rbtree_node &rhs)
        noexcept(noexcept(std::declval<Comparer>()(std::declval<Key>(), std::declval<Key>())))
      { return cmper(lhs.m_key, rhs.m_key); }

      friend bool operator > (const rbtree_node &lhs, const rbtree_node &rhs)
        noexcept(noexcept(std::declval<Comparer>()(std::declval<Key>(), std::declval<Key>())))
      { return cmper(rhs.m_key, lhs.m_key); }

      friend bool operator <= (const rbtree_node &lhs, const rbtree_node &rhs)
        noexcept(noexcept(std::declval<Comparer>()(std::declval<Key>(), std::declval<Key>())))
      { return !cmper(rhs.m_key, lhs.m_key); }

      friend bool operator >= (const rbtree_node &lhs, const rbtree_node &rhs)
        noexcept(noexcept(std::declval<Comparer>()(std::declval<Key>(), std::declval<Key>())))
      { return !cmper(rhs.m_key, lhs.m_key); }

    private:

      static rbtree_node *internal_cast(rbtree_node<void, void> *x)
      { return static_cast<rbtree_node *>(x); }

      static const rbtree_node *internal_cast(const rbtree_node<void, void> *x)
      { return static_cast<const rbtree_node *>(x); }

      static Comparer cmper;

      Key m_key;
    };

    template<typename Key, typename Comparer, typename Tag>
    Comparer rbtree_node<Key, Comparer, Tag>::cmper;


    template<typename Key, typename Comparer, typename Tag>
    class rbtree_iterator
    {
    public:
      // Nested type similar with STL
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type        = rbtree_node<Key, Comparer, Tag>;
      using reference         = rbtree_node<Key, Comparer, Tag> &;
      using pointer           = rbtree_node<Key, Comparer, Tag> *;
      using difference_type   = std::ptrdiff_t;
      using node_type         = rbtree_node<void, void>;

      explicit rbtree_iterator(node_type *node) noexcept
        : m_node(node)
      { }

      rbtree_iterator(const rbtree_iterator &i) noexcept
        : m_node(i.m_node)
      { }

      rbtree_iterator &operator = (const rbtree_iterator &i) noexcept
      { m_node = i.m_node; }

      ~rbtree_iterator() noexcept = default;

      reference operator * () const noexcept { return *internal_cast(); }

      pointer operator -> () const noexcept { return internal_cast(); }

      rbtree_iterator &operator ++ () noexcept
      {
        m_node = m_node->next();
        return *this;
      }

      rbtree_iterator operator ++ (int) noexcept
      {
        auto ret(*this);
        ++(*this);
        return ret;
      }

      rbtree_iterator &operator -- () noexcept
      {
        bool has_l = m_node->m_has_l;
        m_node = m_node->prev();
        return *this;
      }

      rbtree_iterator operator -- (int) noexcept
      {
        auto ret(*this);
        --(*this);
        return ret;
      }

      friend bool operator == (const rbtree_iterator &l, const rbtree_iterator &r) noexcept
      { return l.m_node == r.m_node; }

      friend bool
        operator != (const rbtree_iterator &l, const rbtree_iterator &r) noexcept
      { return !(l.m_node == r.m_node); }

    private:

      pointer internal_cast() const noexcept
      { return static_cast<pointer>(m_node); }

      node_type *m_node;
    };

    template<typename Key, typename Comparer, typename Tag>
    class rbtree_const_iterator
    {
    public:
      // Nested type similar with STL
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type        = const rbtree_node<Key, Comparer, Tag>;
      using reference         = const rbtree_node<Key, Comparer, Tag> &;
      using pointer           = const rbtree_node<Key, Comparer, Tag> *;
      using difference_type   = std::ptrdiff_t;
      using node_type         = const rbtree_node<void, void>;

      explicit rbtree_const_iterator(node_type *node) noexcept
        : m_node(node)
      { }

      rbtree_const_iterator(const rbtree_iterator<Key, Comparer, Tag> &i) noexcept
        : m_node(&*i)
      { }

      rbtree_const_iterator(const rbtree_const_iterator &i) noexcept
        : m_node(i.m_node)
      { }

      rbtree_const_iterator &operator = (const rbtree_const_iterator &i) noexcept
      { m_node = i.m_node; }

      ~rbtree_const_iterator() noexcept = default;

      reference operator * () const noexcept { return *internal_cast(); }

      pointer operator -> () const noexcept { return internal_cast(); }

      rbtree_const_iterator &operator ++ () noexcept
      {
        m_node = m_node->next();
        return *this;
      }

      rbtree_const_iterator operator ++ (int) noexcept
      {
        auto ret(*this);
        ++(*this);
        return ret;
      }

      rbtree_const_iterator &operator -- () noexcept
      {
        m_node = m_node->prev();
        return *this;
      }

      rbtree_const_iterator operator -- (int) noexcept
      {
        auto ret(*this);
        --(*this);
        return ret;
      }

      friend bool operator == (const rbtree_const_iterator &l, const rbtree_const_iterator &r) noexcept
      { return l.m_node == r.m_node; }

      friend bool operator != (const rbtree_const_iterator &l, const rbtree_const_iterator &r) noexcept
      { return !(l.m_node == r.m_node); }

    private:
      pointer internal_cast() const noexcept
      { return static_cast<pointer>(m_node); }

      node_type *m_node;
    };



    template<typename Key, typename Comparer, typename Tag>
    class rbtree
    {
    public:
      // Nested type, similar with STL
      using iterator                = rbtree_iterator<Key, Comparer, Tag>;
      using const_iterator          = rbtree_const_iterator<Key, Comparer, Tag>;
      using reverse_iterator        = std::reverse_iterator<iterator>;
      using const_reverse_iterator  = std::reverse_iterator<const_iterator>;
      using value_type              = rbtree_node<Key, Comparer, Tag>;
      using pointer                 = rbtree_node<Key, Comparer, Tag> *;
      using reference               = rbtree_node<Key, Comparer, Tag>  &;
      using const_pointer           = const rbtree_node<Key, Comparer, Tag>  *;
      using const_reference         = const rbtree_node<Key, Comparer, Tag>  &;
      using size_type               = size_t;
      using difference_type         = std::ptrdiff_t;
      using node_type               = rbtree_node<Key, Comparer, Tag>;

      /** @brief Default constructor */
      rbtree() noexcept
        : m_container_node(node_type::container)
      { }

      template<typename InputIterator>
      rbtree(InputIterator b, InputIterator e) noexcept;

      rbtree(rbtree &&t) noexcept
        : m_container_node(std::move(t.m_container_node))
      { }

      rbtree(const rbtree &) = delete;

      rbtree &operator = (rbtree &&t) noexcept
      {
        if (&t != this)
        {
          this->~rbtree();
          new (this) rbtree(std::move(t));
        }
        return *this;
      }

      rbtree &operator = (const rbtree &t) = delete;

      ~rbtree() noexcept
      { clear(); }


      // Capacity
      bool empty() const noexcept
      { return begin() == end(); }

      size_type size() const noexcept
      {
        size_type s = 0;
        auto b = begin(), e = end();
        while (++b != e) ++s;
        return s;
      }

      // Access

      iterator begin() noexcept
      { return iterator(m_container_node.front_of_container()); }

      reverse_iterator rbegin() noexcept
      { return reverse_iterator(end()); }

      const_iterator begin() const noexcept
      { return const_reverse_iterator(end()); }

      const_reverse_iterator rbegin() const noexcept
      { return const_reverse_iterator(end()); }

      const_iterator cbegin() const noexcept
      { return const_iterator(m_container_node.front_of_container()); }

      const_reverse_iterator crbegin() const noexcept
      { return const_reverse_iterator(end()); }

      iterator end() noexcept
      { return iterator(&m_container_node); }

      reverse_iterator rend() noexcept
      { return reverse_iterator(begin()); }

      const_iterator end() const noexcept
      { return const_iterator(&m_container_node); }

      const_reverse_iterator rend() const noexcept
      { return const_reverse_iterator(begin()); }

      const_iterator cend() const noexcept
      { return const_iterator(m_container_node); }

      const_reverse_iterator crend() const noexcept
      { return const_reverse_iterator(begin()); }

      reference front() noexcept
      { return *begin(); }

      const_reference front() const noexcept
      { return *begin(); }

      reference back() noexcept
      { return *rbegin(); }

      const_reference back() const noexcept
      { return *rbegin(); }

      iterator find(const Key &key) noexcept;

      const_iterator find(const Key &key) const noexcept;

      iterator find(const_iterator hint, const Key &key) noexcept;

      const_iterator find(const_iterator hint, const Key &key) const noexcept;

      iterator lower_bound(const Key &key)
        noexcept(noexcept(std::declval<Comparer>()(key, key)))
      { return lower_bound(end(), key); }

      const_iterator lower_bound(const Key &key) const noexcept;

      iterator lower_bound(iterator hint, const Key &key)
        noexcept(noexcept(std::declval<Comparer>()(key, key)))
      { return iterator(node_type::lower_bound(*hint, key)); }

      const_iterator lower_bound(const_iterator hint, const Key &key) const noexcept;

      iterator upper_bound(const Key &key)
      noexcept(noexcept(std::declval<Comparer>()(key, key)))
      { return upper_bound(end(), key); }

      //const_iterator upper_bound(const Key &key) const noexcept;

      iterator upper_bound(iterator hint, const Key &key)
      noexcept(noexcept(std::declval<Comparer>()(key, key)))
      { return iterator(node_type::upper_bound(*hint, key)); }

      //const_iterator upper_bound(const_iterator hint, const Key &key) const noexcept;

      // Modifier
      iterator insert(value_type &val) noexcept
      {
        return insert(end(), val);
      }

      iterator insert(iterator hint, value_type &val) noexcept
      {
        node_type::insert_after(*hint, val);
        return iterator(&val);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e) noexcept
      {
        while (b != e)
          insert(*b++);
      }

      void erase(iterator iter) noexcept
      { iter->unlink(); }

      void erase(iterator b, iterator e) noexcept
      {
        for (auto i = b; i != e; ++i)
          node_type::unlink(*i++);
      }

      void remove(Key &key) noexcept(noexcept(key == key)); // TODO

      template<typename Predicate>
      void remove(Predicate &&predicate) noexcept(noexcept(pred(std::declval<Key>())));

      void swap(rbtree &&t) noexcept
      { swap(t); }

      void swap(rbtree &t) noexcept
      { node_type::swap_nodes(m_container_node, t.m_container_node); }

      void clear() noexcept
      { erase(begin(), end()); }

      const Comparer &key_comp() const noexcept;
    private:
      rbtree_node<void, void> m_container_node;
    };
  }
}

#endif
