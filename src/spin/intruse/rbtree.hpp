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
    class rbtree_node;

    template<typename Key, typename Comparer, typename Tag>
    class rbtree_set_iterator;

    template<typename Key, typename Comparer, typename Tag>
    class rbtree_set_const_iterator;

    template<typename Key, typename Comparer = less<Key>, typename Tag = void>
    class rbtree_set;

    template<typename Key, typename Comparer = less<Key>, typename Tag = void>
    class rbtree_multiset;

    /**
     * @brief rbtree_node specialization for void key type, used as base class
     * of all the other rbtree_node
     */
    template<>
    class __SPIN_EXPORT__ rbtree_node<void, void>
    {
      template<typename Key, typename Comparer, typename Tag>
      friend class rbtree_node;

      template<typename Key, typename Comparer, typename Tag>
      friend class rbtree_set;

      template<typename Key, typename Comparer, typename Tag>
      friend class rbtree_set_iterator;

    protected:

      /** @brief Auxilary class used for rbtree_set(container_tag) */
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

      /** @brief Tricky way to test if a node is container node or root node */
      bool is_container_or_root() const noexcept
      { return m_p != nullptr && m_p->m_p == this; }

      /** @brief Test if a node is the root node of rbtree_set */
      bool is_root_node() const noexcept
      { return is_container_or_root() && !this->m_is_container; }

      /** @brief Test if container is empty */
      bool is_empty_container_node() const noexcept
      { return m_p == this; }

      /** @brief Test if a node is linked into rbtree_set */
      bool is_linked() const noexcept;

      // Navigation

      /**
       * @brief Get root node from container node
       * @note User is responsible to ensure this node is container node
       * @returns the root node of this rbtree_set, or nullptr if this tree is an
       * empty tree
       */
      const rbtree_node *get_root_node_from_container_node() const noexcept;

      /**
       * @brief Get root node from container node
       * @note User is responsible to ensure this node is container node
       * @returns the root node of this rbtree_set, or nullptr if this tree is an
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
       * @brief Unlink this node from the rbtree_set
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
       * linked into a rbtree_set
       * @note However it doesn't not check if this node is container node of
       * a rbtree_set, and unlink a container node is always cause undefined
       * behaviours
       */
      rbtree_node *unlink_checked() noexcept;

      /**
       * @brief Get the boundry of a rbtree for specified key
       * @tparam Key the type indexed
       * @tparam KeyFetcher type whose instances are callable that cast an
       * rbtree_node<void, void> reference to Key type
       * @tparam Comparer type whose instances are callable that compare two
       * instances of Key type
       * @param entry The entry node of tree
       * @param key The specified key depends on which the boundry is find
       * @param caster An instance of caster
       * @param comparer An instance of Comparer
       */
      template<typename Key, typename KeyFetcher, typename Comparer>
      static std::pair<rbtree_node*, rbtree_node*>
      boundry(rbtree_node &entry, const Key &key, KeyFetcher && keyfetcher,
          Comparer && comparer)
        noexcept(noexcept(std::forward<KeyFetcher>(keyfetcher)(entry))
            && noexcept(std::forward<Comparer>(comparer)(key, key)))
      {
        assert (entry.is_linked());

        auto *p = &entry;

        if (p->m_is_container)
        {
          if (p->is_empty_container_node())
            return std::make_pair(p, p);
          else
            p = p->get_root_node_from_container_node();
        }

        auto cmper = [&] (rbtree_node *node) -> bool
        {
          return std::forward<Comparer>(comparer)(
              std::forward<KeyFetcher>(keyfetcher)(*node), key);
        };
        bool hint_result = cmper(p);

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
              return std::make_pair(p, p->m_r);
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
              return std::make_pair(p->m_l, p);
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
              return std::make_pair(p, p->m_r);
          }
          else
          {
            if (p->m_has_l)
              p = p->m_l;
            else
              return std::make_pair(p->m_l, p);
          }

          hint_result = cmper(p);
        }
      }

      /**
       * @brief Get the boundry of a rbtree for specified key
       * @tparam Key the type indexed
       * @tparam KeyFetcher type whose instances are callable that cast an
       * rbtree_node<void, void> reference to Key type
       * @tparam Comparer type whose instances are callable that compare two
       * instances of Key type
       * @param entry The entry node of tree
       * @param key The specified key depends on which the boundry is find
       * @param caster An instance of caster
       * @param comparer An instance of Comparer
       */
      template<typename Key, typename KeyFetcher, typename Comparer>
      static std::pair<const rbtree_node*, const rbtree_node*>
      boundry(const rbtree_node &entry, const Key &key, KeyFetcher && keyfetcher,
          Comparer && comparer)
        noexcept(noexcept(std::forward<KeyFetcher>(keyfetcher)(entry))
            && noexcept(std::forward<Comparer>(comparer)(key, key)))
      {
        assert (entry.is_linked());

        auto *p = &entry;

        if (p->m_is_container)
        {
          if (p->is_empty_container_node())
            return std::make_pair(p, p);
          else
            p = p->get_root_node_from_container_node();
        }

        auto cmper = [&] (rbtree_node *node) -> bool
        {
          return std::forward<Comparer>(comparer)(
              std::forward<KeyFetcher>(keyfetcher)(*node), key);
        };

        bool hint_result = cmper(p);

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
              return std::make_pair(p, p->m_r);
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
              return std::make_pair(p->m_l, p);
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
              return std::make_pair(p, p->m_r);
          }
          else
          {
            if (p->m_has_l)
              p = p->m_l;
            else
              return std::make_pair(p->m_l, p);
          }

          hint_result = cmper(p);
        }
      }


      /**
       * @brief Search and execute
       * @tparam Key the type indexed
       * @tparam KeyFetcher type whose instances are callable that cast an
       * rbtree_node<void, void> reference to Key type
       * @tparam Comparer type whose instances are callable that compare two
       * instances of Key type
       * @tparam Callable type whose instances are are callable with two
       * argument, the type of first argument is rbtree_node<void, void>, and
       * the type of second argument is boolean indicates that the compare
       * result of specified key with the first parameter pass to routine
       * @param entry The entry node for search
       * @param key The key to be searched with
       * @param caster An instance of caster
       * @param comparer An instance of Comparer
       * @param routine An instance of Callable
       */
      template<typename Key, typename KeyFetcher,
        typename Comparer, typename Callable>
      static void search_and_execute(rbtree_node &entry, const Key &key,
          KeyFetcher && keyfetcher, Comparer && comparer, Callable && routine)
        noexcept(noexcept(std::forward<KeyFetcher>(keyfetcher)(entry))
            && noexcept(std::declval<Comparer>()(key, key))
            && noexcept(std::declval<Callable>()(entry, false)))
      {
        assert(entry.is_linked());

        auto *p = &entry;

        if (p->m_is_container)
        {
          if (p->is_empty_container_node())
          {
            // false is passed as 2nd argument so, client code in the routine
            // may need not to check whether the first parameter is container
            // node
            std::forward<Callable>(routine)(*p, false);
            return ;
          }
          else
            p = p->get_root_node_from_container_node();
        }

        auto cmper = [&] (rbtree_node *node) -> bool
        {
          return std::forward<Comparer>(comparer)(
              std::forward<KeyFetcher>(keyfetcher)(*node), key);
        };

        bool hint_result = cmper(p);

        auto done = [&] () -> void
        { std::forward<Callable>(routine)(*p, hint_result); };

        // Search for youngest common parent
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

      /**
       * @brief Search and execute, const version
       * @tparam Key the type indexed
       * @tparam Caster type whose instances are callable that cast an
       * rbtree_node<void, void> reference to Key type
       * @tparam Comparer type whose instances are callable that compare two
       * instances of Key type
       * @tparam Callable type whose instances are are callable with two
       * argument, the type of first argument is rbtree_node<void, void>, and
       * the type of second argument is boolean indicates that the compare
       * result of specified key with the first parameter pass to routine
       * @param entry The entry for search
       * @param key The key to be searched with
       * @param caster An instance of caster
       * @param comparer An instance of Comparer
       * @param routine An instance of Callable
       */
      template<typename Key, typename KeyFetcher, typename Comparer, typename Callable>
      static void search_and_execute(const rbtree_node &entry, const Key &key,
          KeyFetcher && keyfetcher, Comparer && comparer, Callable && routine)
        noexcept(noexcept(std::forward<KeyFetcher>(keyfetcher)(entry))
            && noexcept(std::declval<Comparer>()(key, key))
            && noexcept(std::declval<Callable>()(entry, false)))
      {
        assert(entry.is_linked());

        auto *p = &entry;

        if (p->m_is_container)
        {
          if (p->is_empty_container_node())
          {
            // false is passed as 2nd argument so, client code in the routine
            // may need not to check whether the first parameter is container
            // node
            std::forward<Callable>(routine)(*p, false);
            return ;
          }
          else
            p = p->get_root_node_from_container_node();
        }

        auto cmper = [&] (rbtree_node *node) -> bool
        {
          return std::forward<Comparer>(comparer)(
              std::forward<KeyFetcher>(keyfetcher)(*node), key);
        };

        bool hint_result = cmper(p);

        auto done = [&] () -> void
        { std::forward<Callable>(routine)(*p, hint_result); };

        // Search for youngest common parent
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

      /** @brief Transfer link to another */
      void __SPIN_INTERNAL__ transfer_link(rbtree_node &node) noexcept;

      /**
       * @brief Swap two nodes in the tree
       * @note This will generally break the order or nodes, so it's declared
       * privately
       */
      static void __SPIN_INTERNAL__
      swap_nodes(rbtree_node &lhs, rbtree_node &rhs)
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
      friend class rbtree_set<Key, Comparer, Tag>;
      friend class rbtree_set_iterator<Key, Comparer, Tag>;
      friend class rbtree_set_const_iterator<Key, Comparer, Tag>;

      /** @brief Node for container */
      struct container_node : public rbtree_node<void, void>
      {
      public:
        container_node()
          : rbtree_node<void, void>(rbtree_node<void, void>::container)
        { }
      };

      struct front_policy {};
      struct back_policy {};
      struct nearest_policy {};
      struct unique_policy {};
      struct override_policy {};


      class duplicate_inserter
      {
      public:
        rbtree_node &node;

        duplicate_inserter(rbtree_node &node) noexcept
          : node(node)
        { }

        void operator ()
          (std::pair<rbtree_node<void, void> *, rbtree_node<void, void> *> &&pair)
          noexcept
        { (*this)(pair); }

        void operator ()
          (std::pair<rbtree_node<void, void> *, rbtree_node<void, void> *> &pair)
          noexcept
        {
          // first and second cound only be asame when the tree is empty
          if (pair.first == pair.second)
            pair.first->insert_root_node(&node);

          // Either pair.first has no right child or pair.second has no left
          // child, so it can be optimized.

          else if (pair.second->m_is_container)
            pair.first->insert_to_right(&node);
          else if (pair.first->m_is_container)
            pair.second->insert_to_left(&node);
          else if (pair.second->m_has_l)
            pair.first->insert_to_right(&node);
          else
            pair.second->insert_to_left(&node);
        }

        void operator () (rbtree_node<void, void> &n, bool result) noexcept
        {
          if (n.m_is_container)
            n.insert_root_node(&node);
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
      };

      class replace_inserter
      {
      public:
        rbtree_node *node;

        replace_inserter(rbtree_node *node) noexcept
          : node(node)
        { }

        void operator () (rbtree_node<void, void> &n, bool result) noexcept
        {
          if (n.m_is_container)
          {
            n.insert_root_node(node);
          }
          else if (result)
          {
            auto *p = internal_cast(&n);
            if (n.m_has_r)
              n.next()->insert_before(node);
            else
              n.insert_after(node);
            if (cmper(node.get_key(), p->get_key()))
              n.unlink();

          }
          else
          {
            auto *p = internal_cast(&n);
            if (n.m_has_l)
              n.prev()->insert_after(node);
            else
              n.insert_before(node);
            if (!cmper(node.get_key(), p->get_key()))
              n.unlink();
          }
        }
      };

      class unique_inserter
      {
      public:
        rbtree_node *node;

        unique_inserter(rbtree_node *node) noexcept
          : node(node)
        { }

        void operator () (rbtree_node<void, void> &n, bool result) noexcept
        {
          if (n.m_is_container)
          {
            n.insert_root_node(node);
          }
          else if (result)
          {
            auto *p = internal_cast(&n);
            if (cmper(node->get_key(), p->get_key()))
              node = p;
            else if (n.m_has_r)
              n.next()->insert_before(node);
            else
              n.insert_after(node);
          }
          else
          {
            auto *p = internal_cast(&n);
            if (!cmper(node->get_key(), p->get_key()))
              node = p;
            else if (n.m_has_l)
              n.prev()->insert_after(node);
            else
              n.insert_before(node);
          }
        }
      };
    public:

      /** @brief get key of this node */
      const Key &get_key() const noexcept
      { return m_key; }

      /** @brief update key of this node */
      template<typename Policy = unique_policy>
      Key update_key(Key key)
        noexcept(noexcept(std::swap(key, key)))
      {
        auto *p = unlink_checked();
        std::swap(key, m_key);
        if (p)
          insert(*p, *this, Policy()); // FIXME: duplicated or not
        return key;
      }

      /**
       * @brief Unlink current node from a tree
       * @note User is responsible to ensure this node is already attached
       * into a tree
       */
      static void unlink(rbtree_node &node) noexcept
      { node.rbtree_node<void, void>::unlink(); }

      /**
       * @brief Get the first node in the tree whose key is not less than
       * specified value
       * @param entry Entry node for search
       * @param key The specified value for searching the lower boundry
       */
      static rbtree_node<void, void> *
      lower_bound(rbtree_node<void, void> &entry, const Key &key)
      noexcept(noexcept(std::declval<Comparer>()(key, key)))
      { return boundry(entry, key, key_fetcher, cmper).second; }

      /**
       * @brief Get the first node in the tree whose key is not less than
       * specified value
       * @param entry Entry node for search
       * @param key The specified value for searching the lower boundry
       */
      static const rbtree_node<void, void> *
      lower_bound(const rbtree_node<void, void> &entry, const Key &key)
      noexcept(noexcept(std::declval<Comparer>()(key, key)))
      { return boundry(entry, key, key_fetcher, cmper).second; }

      /**
       * @brief Get the first node in the tree whose key is greater than
       * specified value
       * @param entry Entry node for search
       * @param key The specified value for searching the upper boundry
       */
      static rbtree_node<void, void> *
      upper_bound(rbtree_node<void, void> &entry, const Key &key)
      noexcept(noexcept(std::declval<Comparer>()(key, key)))
      {
        return boundry(entry, key, key_fetcher,
            [] (const Key &lhs, const Key &rhs) noexcept(is_comparer_noexcept)
            { return !cmper(rhs, lhs); }).second;
      }

      /**
       * @brief Get the first node in the tree whose key is greater than
       * specified value
       * @param entry Entry node for search
       * @param key The specified value for searching the upper boundry
       */
      static const rbtree_node<void, void> *
      upper_bound(const rbtree_node<void, void> &entry, const Key &key)
      noexcept(noexcept(std::declval<Comparer>()(key, key)))
      {
        return boundry(entry, key, key_fetcher,
            [] (const Key &lhs, const Key &rhs) noexcept(is_comparer_noexcept)
            { return !cmper(rhs, lhs); }).second;
      }


      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param hint_node The node which is attached into a rbtree_set for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note User is responsible to ensure hint_node is already attached to a
       * tree; and if duplicate is permitted, the node is insert after any
       * node duplicate with this node
       */
      static rbtree_node *
      insert(rbtree_node<void, void> &entry, rbtree_node &node, back_policy)
      noexcept(noexcept(std::declval<Comparer>()(node.m_key, node.m_key)))
      {
        duplicate_inserter inserter(node);
        inserter(boundry(entry, node.get_key(), key_fetcher,
            [] (const Key &lhs, const Key &rhs) noexcept // comparer
            { return !cmper(rhs, lhs); }));
        return &node;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param entry The node which is attached into a rbtree_set for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note Use is responsible to ensure hint_node is already attached to a
       * tree; and if duplicate is permitted, the node is insert before any
       * node duplicate with this node
       */
      static rbtree_node *
      insert(rbtree_node<void, void> &entry, rbtree_node &node, front_policy)
      noexcept(noexcept(std::declval<Comparer>()(node.m_key, node.m_key)))
      {
        duplicate_inserter inserter(node);
        inserter(boundry(entry, node.get_key(), key_fetcher, cmper));
        return &node;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param entry The node which is attached into a rbtree_set for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note Use is responsible to ensure hint_node is already attached to a
       * tree; and if duplicate is permitted, the node is insert with least
       * searching being done
       */
      static rbtree_node *
      insert(rbtree_node<void, void> &entry, rbtree_node &node, nearest_policy)
      noexcept(noexcept(std::declval<Comparer>()(node.m_key, node.m_key)))
      {
        search_and_execute(entry, node.get_key(), key_fetcher,

            [] (const Key &lhs, const Key &rhs) noexcept // comparer
            { return cmper(lhs, rhs) || !cmper(rhs, lhs); },

            duplicate_inserter(node) // routine
          );
        return &node;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param hint_node The node which is attached into a rbtree_set for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note User is responsible to ensure hint_node is already attached to
       * a tree; and duplicated node is not allow
       */
      static rbtree_node *
      insert(rbtree_node<void, void> &entry, rbtree_node &node, unique_policy)
      noexcept(noexcept(std::declval<Comparer>()(node.m_key, node.m_key)))
      {
        unique_inserter inserter(&node);

        search_and_execute(entry, node.get_key(),

            [] (const rbtree_node<void, void> &n) // caster
            { return internal_cast(&n)->get_key(); },

            // comparer
            [] (const Key &lhs, const Key &rhs) noexcept
            { return cmper(lhs, rhs); },

            // routine
            inserter
          );
        return inserter.node;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param hint_node The node which is attached into a rbtree_set for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note User is responsible to ensure hint_node is already attached to
       * a tree; and duplicated node will be replaced by this node
       */
      static rbtree_node *
      insert(rbtree_node<void, void> &entry, rbtree_node &node, override_policy)
      noexcept(noexcept(std::declval<Comparer>()(node.m_key, node.m_key)))
      {

        search_and_execute(entry, node.get_key(),

            // caster
            [] (rbtree_node<void, void> &n) noexcept
            { return internal_cast(&n)->get_key(); },

            // comparer
            [] (const Key &lhs, const Key &rhs)
            { return cmper(lhs, rhs); },

            // routine
            replace_inserter(node)
          );
        return &node;
      }

      /**
       * @brief Default constructor, forwarding all arguments to delegated key
       * type
       */
      template<typename ...Args>
      rbtree_node(Args && ...args)
        noexcept(noexcept(Key(std::forward<Args>(args)...)))
        : rbtree_node<void, void>()
        , m_key(std::forward<Args>(args)...)
      { }

      /** @brief Default destructor */
      ~rbtree_node() noexcept(noexcept(std::declval<Key>().~Key())) = default;

      /** @brief Move constructor */
      rbtree_node(rbtree_node &&n)
        noexcept(noexcept(Key(std::declval<Key>())))
        : rbtree_node<void, void>(std::move(n))
        , m_key(std::move(n.m_key))
      { }

      /** @brief Move assignment */
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
        noexcept(noexcept(std::declval<Comparer>()(lhs.m_key, rhs.m_key)))
      { return cmper(lhs.m_key, rhs.m_key); }

      friend bool operator > (const rbtree_node &lhs, const rbtree_node &rhs)
        noexcept(noexcept(std::declval<Comparer>()(lhs.m_key, rhs.m_key)))
      { return cmper(rhs.m_key, lhs.m_key); }

      friend bool operator <= (const rbtree_node &lhs, const rbtree_node &rhs)
        noexcept(noexcept(std::declval<Comparer>()(lhs.m_key, rhs.m_key)))
      { return !cmper(rhs.m_key, lhs.m_key); }

      friend bool operator >= (const rbtree_node &lhs, const rbtree_node &rhs)
        noexcept(noexcept(std::declval<Comparer>()(lhs.m_key, rhs.m_key)))
      { return !cmper(rhs.m_key, lhs.m_key); }

    private:

      static rbtree_node *internal_cast(rbtree_node<void, void> *x) noexcept
      { return static_cast<rbtree_node *>(x); }

      static const rbtree_node *internal_cast(const rbtree_node<void, void> *x) noexcept
      { return static_cast<const rbtree_node *>(x); }

      static class key_fetcher_t
      {
      public:
        const Key &operator () (const rbtree_node<void, void> &x) const noexcept
        { return internal_cast(&x)->get_key(); }
      } key_fetcher;

      static Comparer cmper;

      constexpr static bool is_comparer_noexcept
        = noexcept(std::declval<Comparer>()(std::declval<Key>(), std::declval<Key>()));

      Key m_key;
    };

    template<typename Key, typename Comparer, typename Tag>
    Comparer rbtree_node<Key, Comparer, Tag>::cmper;

    template<typename Key, typename Comparer, typename Tag>
    typename rbtree_node<Key, Comparer, Tag>::key_fetcher_t
    rbtree_node<Key, Comparer, Tag>::key_fetcher;

    template<typename Key, typename Comparer, typename Tag>
    class rbtree_set_iterator
    {
    public:
      // Nested type similar with STL
      using iterator_category = std::bidirectional_iterator_tag;
      using node_type         = rbtree_node<void, void>;
      using value_type        = rbtree_node<Key, Comparer, Tag>;
      using reference         = value_type &;
      using pointer           = value_type *;
      using difference_type   = std::ptrdiff_t;

      explicit rbtree_set_iterator(node_type *node) noexcept
        : m_node(node)
      { }

      rbtree_set_iterator(const rbtree_set_iterator &i) noexcept
        : m_node(i.m_node)
      { }

      rbtree_set_iterator &operator = (const rbtree_set_iterator &i) noexcept
      { m_node = i.m_node; }

      ~rbtree_set_iterator() noexcept = default;

      reference operator * () const noexcept { return *internal_cast(); }

      pointer operator -> () const noexcept { return internal_cast(); }

      rbtree_set_iterator &operator ++ () noexcept
      {
        m_node = m_node->next();
        return *this;
      }

      rbtree_set_iterator operator ++ (int) noexcept
      {
        auto ret(*this);
        ++(*this);
        return ret;
      }

      rbtree_set_iterator &operator -- () noexcept
      {
        bool has_l = m_node->m_has_l;
        m_node = m_node->prev();
        return *this;
      }

      rbtree_set_iterator operator -- (int) noexcept
      {
        auto ret(*this);
        --(*this);
        return ret;
      }

      friend bool operator == (const rbtree_set_iterator &l,
          const rbtree_set_iterator &r) noexcept
      { return l.m_node == r.m_node; }

      friend bool operator != (const rbtree_set_iterator &l,
          const rbtree_set_iterator &r) noexcept
      { return !(l.m_node == r.m_node); }

    private:

      pointer internal_cast() const noexcept
      { return static_cast<pointer>(m_node); }

      node_type *m_node;
    };

    template<typename Key, typename Comparer, typename Tag>
    class rbtree_set_const_iterator
    {
    public:
      // Nested type similar with STL
      using iterator_category = std::bidirectional_iterator_tag;
      using node_type         = const rbtree_node<void, void>;
      using value_type        = const rbtree_node<Key, Comparer, Tag>;
      using reference         = value_type &;
      using pointer           = value_type *;
      using difference_type   = std::ptrdiff_t;

      explicit rbtree_set_const_iterator(node_type *node) noexcept
        : m_node(node)
      { }

      rbtree_set_const_iterator
        (const rbtree_set_iterator<Key, Comparer, Tag> &i) noexcept
        : m_node(&*i)
      { }

      rbtree_set_const_iterator
        (const rbtree_set_const_iterator &i) noexcept
        : m_node(i.m_node)
      { }

      rbtree_set_const_iterator &
        operator = (const rbtree_set_const_iterator &i) noexcept
      { m_node = i.m_node; }

      ~rbtree_set_const_iterator() noexcept = default;

      reference operator * () const noexcept
      { return *internal_cast(); }

      pointer operator -> () const noexcept
      { return internal_cast(); }

      rbtree_set_const_iterator &operator ++ () noexcept
      {
        m_node = m_node->next();
        return *this;
      }

      rbtree_set_const_iterator operator ++ (int) noexcept
      {
        auto ret(*this);
        ++(*this);
        return ret;
      }

      rbtree_set_const_iterator &operator -- () noexcept
      {
        m_node = m_node->prev();
        return *this;
      }

      rbtree_set_const_iterator operator -- (int) noexcept
      {
        auto ret(*this);
        --(*this);
        return ret;
      }

      friend bool operator == (const rbtree_set_const_iterator &l,
          const rbtree_set_const_iterator &r) noexcept
      { return l.m_node == r.m_node; }

      friend bool operator != (const rbtree_set_const_iterator &l,
          const rbtree_set_const_iterator &r) noexcept
      { return !(l.m_node == r.m_node); }

    private:
      pointer internal_cast() const noexcept
      { return static_cast<pointer>(m_node); }

      node_type *m_node;
    };



    template<typename Key, typename Comparer, typename Tag>
    class rbtree_set
    {
    public:
      // Nested type, similar with STL
      using iterator                = rbtree_set_iterator<Key, Comparer, Tag>;
      using const_iterator          = rbtree_set_const_iterator<Key, Comparer, Tag>;
      using reverse_iterator        = std::reverse_iterator<iterator>;
      using const_reverse_iterator  = std::reverse_iterator<const_iterator>;
      using node_type               = rbtree_node<Key, Comparer, Tag>;
      using value_type              = node_type;
      using reference               = node_type &;
      using pointer                 = node_type *;
      using const_pointer           = const node_type *;
      using const_reference         = const node_type &;
      using size_type               = size_t;
      using difference_type         = std::ptrdiff_t;

      using override_policy         = typename node_type::override_policy;
      using unique_policy           = typename node_type::unique_policy;
      using front_policy            = typename node_type::front_policy;
      using back_policy             = typename node_type::back_policy;
      using nearest_policy          = typename node_type::nearest_policy;


      /** @brief Default constructor */
      rbtree_set() noexcept
        : m_container_node(node_type::container)
      { }

      template<typename InputIterator>
      rbtree_set(InputIterator b, InputIterator e) noexcept;

      rbtree_set(rbtree_set &&t) noexcept
        : m_container_node(std::move(t.m_container_node))
      { }

      rbtree_set(const rbtree_set &) = delete;

      rbtree_set &operator = (rbtree_set &&t) noexcept
      {
        if (&t != this)
        {
          this->~rbtree_set();
          new (this) rbtree_set(std::move(t));
        }
        return *this;
      }

      rbtree_set &operator = (const rbtree_set &t) = delete;

      ~rbtree_set() noexcept
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

      const_iterator find(iterator hint, const Key &key) const noexcept;

      std::pair<iterator, iterator>
      equals_range(const Key &key) noexcept;

      std::pair<iterator, iterator>
      equals_range(iterator hint, const Key &key) noexcept;

      std::pair<const_iterator, const_iterator>
      equals_range(const Key &key) const noexcept;

      std::pair<const_iterator, const_iterator>
      equals_range(const_iterator hint, const Key &key) const noexcept;

      iterator lower_bound(const Key &key)
        noexcept(noexcept(std::declval<Comparer>()(key, key)))
      { return lower_bound(end(), key); }

      const_iterator lower_bound(const Key &key) const noexcept
      { return lower_bound(end(), key); }

      iterator lower_bound(iterator hint, const Key &key)
        noexcept(noexcept(std::declval<Comparer>()(key, key)))
      { return iterator(node_type::lower_bound(*hint, key)); }

      const_iterator lower_bound(const_iterator hint, const Key &key)
        const noexcept
      { return const_iterator(node_type::lower_bound(*hint, key)); }

      iterator upper_bound(const Key &key)
      noexcept(noexcept(std::declval<Comparer>()(key, key)))
      { return upper_bound(end(), key); }

      const_iterator upper_bound(const Key &key) const noexcept
      { return upper_bound(end(), key); }

      iterator upper_bound(iterator hint, const Key &key)
      noexcept(noexcept(std::declval<Comparer>()(key, key)))
      { return iterator(node_type::upper_bound(*hint, key)); }

      const_iterator upper_bound(const_iterator hint, const Key &key)
      const noexcept
      { return const_iterator(node_type::upper_bound(*hint, key)); }

      // Modifier

      iterator insert(node_type &val) noexcept
      { return insert(end(), val, unique_policy()); }

      iterator insert(node_type &val, unique_policy p) noexcept
      { return insert(end(), val, p); }

      iterator insert(node_type &val, override_policy p) noexcept
      { return insert(end(), val, p); }

      iterator insert(node_type &val, front_policy p) noexcept
      { return insert(end(), val, p); }

      iterator insert(node_type &val, back_policy p) noexcept
      { return insert(end(), val, p); }

      iterator insert(node_type &val, nearest_policy p) noexcept
      { return insert(end(), val, p); }

      iterator insert(iterator hint, node_type &val) noexcept
      { return insert(end(), val, unique_policy()); }

      iterator insert(iterator hint, node_type &val, unique_policy p) noexcept
      { return iterator(node_type::insert(*hint, val, p)); }

      iterator insert(iterator hint, node_type &val, override_policy p) noexcept
      { return iterator(node_type::insert(*hint, val, p)); }

      iterator insert(iterator hint, node_type &val, front_policy p) noexcept
      { return iterator(node_type::insert(*hint, val, p)); }

      iterator insert(iterator hint, node_type &val, back_policy p) noexcept
      { return iterator(node_type::insert(*hint, val, p)); }

      iterator insert(iterator hint, node_type &val, nearest_policy p) noexcept
      { return iterator(node_type::insert(*hint, val, p)); }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e) noexcept
      {
        while (b != e)
          insert(*b++);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint) noexcept
      {
        while (b != e)
          insert(hint, *b++);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          front_policy p) noexcept
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          back_policy p) noexcept
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          nearest_policy p) noexcept
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          unique_policy p) noexcept
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          override_policy p) noexcept
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      void erase(iterator iter) noexcept
      { iter->unlink(); }

      void erase(iterator b, iterator e) noexcept
      {
        for (auto i = b; i != e; ++i)
          node_type::unlink(*i++);
      }

      void remove(Key &key)
      noexcept(noexcept(std::declval<Comparer>()(key, key)))
      {
        auto b = upper_bound(end(), key);
        auto e = lower_bound(b, key);
        erase(b, e);
      }

      template<typename Predicate>
      void remove(Predicate &&predicate)
      noexcept(noexcept(predicate(std::declval<Key>())))
      {
        auto b = begin(), e = end();
        while (b != e)
        {
          auto x = b++;
          if (predicate(x->get_key()))
            erase(x);
        }
      }

      void swap(rbtree_set &&t) noexcept
      { swap(t); }

      void swap(rbtree_set &t) noexcept
      { node_type::swap_nodes(m_container_node, t.m_container_node); }

      void clear() noexcept
        // Can be optimize
      { erase(begin(), end()); }

      const Comparer &key_comp() const noexcept
      { return node_type::cmper; }
    private:
      rbtree_node<void, void> m_container_node;
    };


  }
}

#endif
