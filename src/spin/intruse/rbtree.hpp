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

    class policy_frontmost_t  {} policy_frontmost;
    class policy_backmost_t   {} policy_backmost;
    class policy_nearest_t    {} policy_nearest;
    class policy_override_t   {} policy_override;
    class policy_unique_t     {} policy_unique;


    template<typename Index, typename Tag = void,
      typename Comparer = less<Index>>
    class rbtree_node;

    template<typename Index, typename Type, typename Tag,
      typename Comparer>
    class rbtree_iterator;

    template<typename Index, typename Type, typename Tag,
      typename Comparer>
    class rbtree_const_iterator;

    template<typename Index, typename Type, typename Tag = void,
      typename Comparer = less<Index>>
    class rbtree;

    /**
     * @brief rbtree_node specialization for void key type, used as base class
     * of all the other rbtree_node
     */
    template<>
    class __SPIN_EXPORT__ rbtree_node<void>
    {
      template<typename Index, typename Tag, typename Comparer>
      friend class rbtree_node;

      template<typename Index, typename Type, typename Tag, typename Comparer>
      friend class rbtree;

      template<typename Index, typename Type, typename Tag, typename Comparer>
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

      /** @brief Tricky way to test if a node is container node or root node */
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

      static void insert(rbtree_node *entry, rbtree_node *node) noexcept;

      static void insert_between(rbtree_node *prev,
          rbtree_node *next, rbtree_node *node) noexcept;

      static void insert_unique(rbtree_node *prev,
          rbtree_node *next, rbtree_node *node) noexcept;

      static void insert_override(rbtree_node *prev,
          rbtree_node *next, rbtree_node *node) noexcept;

      template<typename Index, typename IndexFetcher, typename Comparer>
      static std::pair<rbtree_node*, rbtree_node*>
      search(rbtree_node &entry, const Index &key, IndexFetcher && keyfetcher,
          Comparer && comparer)
        noexcept(noexcept(std::forward<IndexFetcher>(keyfetcher)(entry))
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

        auto cmp = [&] (rbtree_node *node) -> bool
        {
          return std::forward<Comparer>(comparer)(
              std::forward<IndexFetcher>(keyfetcher)(*node), key);
        };

        auto rcmp = [&] (rbtree_node *node) -> bool
        {
          return !std::forward<Comparer>(comparer)(key,
              std::forward<IndexFetcher>(keyfetcher)(*node));
        };

        auto result = cmp(p);
        auto rresult = rcmp(p);

        if (result != rresult)
          return std::make_pair(p, p);
        else if (result)
        {
          while (!p->m_p->m_is_container)
          {
            if (p == p->m_p->m_l)
            {
              auto x = cmp(p->m_p);
              auto y = rcmp(p->m_p);
              if (x != y)
                return std::make_pair(p->m_p, p->m_p);
              else if (x)
              {
                p = p->m_p;
                continue;
              }
            }

            if (!p->m_r->m_is_container)
            {
              auto x = cmp(p->m_r);
              auto y = rcmp(p->m_r);
              if (x != y)
                return std::make_pair(p->m_r, p->m_r);
              if (x)
                p = p->m_r;
              else
                break;
            }
            else
              return std::make_pair(p, p->m_r);
          }
        }
        else
        {
          while (!p->m_p->m_is_container)
          {
            if (p == p->m_p->m_r)
            {
              auto x = cmp(p->m_p);
              auto y = rcmp(p->m_p);
              if (x != y)
                return std::make_pair(p->m_p, p->m_p);
              else if (x)
              {
                p = p->m_p;
                continue;
              }
            }

            if (!p->m_l->m_is_container)
            {
              auto x = cmp(p->m_l);
              auto y = rcmp(p->m_l);
              if (x != y)
                return std::make_pair(p->m_l, p->m_l);
              if (x)
                p = p->m_l;
              else
                break;
            }
            else
              return std::make_pair(p->m_l, p);
          }
        }

        for ( ; ; )
        {
          if (result)
            if (p->m_has_r)
              p = p->m_r;
            else
              return std::make_pair(p, p->m_r);
          else
            if (p->m_has_l)
              p = p->m_l;
            else
              return std::make_pair(p->m_l, p);

          result = cmp(p);
          rresult = rcmp(p);
          if (result != rresult)
            return std::make_pair(p, p);
        }
      }

      template<typename Index, typename IndexFetcher, typename Comparer>
      static std::pair<const rbtree_node*, const rbtree_node*>
      search(const rbtree_node &entry, const Index &key, IndexFetcher && keyfetcher,
          Comparer && comparer)
        noexcept(noexcept(std::forward<IndexFetcher>(keyfetcher)(entry))
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

        auto cmp = [&] (rbtree_node *node) -> bool
        {
          return std::forward<Comparer>(comparer)(
              std::forward<IndexFetcher>(keyfetcher)(*node), key);
        };

        auto rcmp = [&] (rbtree_node *node) -> bool
        {
          return !std::forward<Comparer>(comparer)(key,
              std::forward<IndexFetcher>(keyfetcher)(*node));
        };

        auto result = cmp(p);
        auto rresult = rcmp(p);

        if (result != rresult)
          return std::make_pair(p, p);
        else if (result)
        {
          while (!p->m_p->m_is_container)
          {
            if (p == p->m_p->m_l)
            {
              auto x = cmp(p->m_p);
              auto y = rcmp(p->m_p);
              if (x != y)
                return std::make_pair(p->m_p, p->m_p);
              else if (x)
              {
                p = p->m_p;
                continue;
              }
            }

            if (!p->m_r->m_is_container)
            {
              auto x = cmp(p->m_r);
              auto y = rcmp(p->m_r);
              if (x != y)
                return std::make_pair(p->m_r, p->m_r);
              if (x)
                p = p->m_r;
              else
                break;
            }
            else
              return std::make_pair(p, p->m_r);
          }
        }
        else
        {
          while (!p->m_p->m_is_container)
          {
            if (p == p->m_p->m_r)
            {
              auto x = cmp(p->m_p);
              auto y = rcmp(p->m_p);
              if (x != y)
                return std::make_pair(p->m_p, p->m_p);
              else if (x)
              {
                p = p->m_p;
                continue;
              }
            }

            if (!p->m_l->m_is_container)
            {
              auto x = cmp(p->m_l);
              auto y = rcmp(p->m_l);
              if (x != y)
                return std::make_pair(p->m_l, p->m_l);
              if (x)
                p = p->m_l;
              else
                break;
            }
            else
              return std::make_pair(p->m_l, p);
          }
        }

        for ( ; ; )
        {
          if (result)
            if (p->m_has_r)
              p = p->m_r;
            else
              return std::make_pair(p, p->m_r);
          else
            if (p->m_has_l)
              p = p->m_l;
            else
              return std::make_pair(p->m_l, p);

          result = cmp(p);
          rresult = rcmp(p);
          if (result != rresult)
            return std::make_pair(p, p);
        }
      }

      /**
       * @brief Get the boundry of a rbtree for specified key
       * @tparam Index the type indexed
       * @tparam IndexFetcher type whose instances are callable that cast an
       * rbtree_node<void> reference to Index type
       * @tparam Comparer type whose instances are callable that compare two
       * instances of Index type
       * @param entry The entry node of tree
       * @param key The specified key depends on which the boundry is find
       * @param caster An instance of caster
       * @param comparer An instance of Comparer
       */
      template<typename Index, typename IndexFetcher, typename Comparer>
      static std::pair<rbtree_node*, rbtree_node*>
      boundry(rbtree_node &entry, const Index &key, IndexFetcher && keyfetcher,
          Comparer && comparer)
        noexcept(noexcept(std::forward<IndexFetcher>(keyfetcher)(entry))
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
              std::forward<IndexFetcher>(keyfetcher)(*node), key);
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
       * @tparam Index the type indexed
       * @tparam IndexFetcher type whose instances are callable that cast an
       * rbtree_node<void> reference to Index type
       * @tparam Comparer type whose instances are callable that compare two
       * instances of Index type
       * @param entry The entry node of tree
       * @param key The specified key depends on which the boundry is find
       * @param caster An instance of caster
       * @param comparer An instance of Comparer
       */
      template<typename Index, typename IndexFetcher, typename Comparer>
      static std::pair<const rbtree_node*, const rbtree_node*>
      boundry(const rbtree_node &entry, const Index &key, IndexFetcher && keyfetcher,
          Comparer && comparer)
        noexcept(noexcept(std::forward<IndexFetcher>(keyfetcher)(entry))
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
              std::forward<IndexFetcher>(keyfetcher)(*node), key);
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

    template<typename Index, typename Tag, typename Comparer>
    class rbtree_node : private rbtree_node<void>
    {

      template<typename _Index, typename _Type, typename _Tag, typename _Comparer>
      friend class rbtree;

      template<typename _Index, typename _Type, typename _Tag, typename _Comparer>
      friend class rbtree_iterator;

      template<typename _Index, typename _Type, typename _Tag, typename _Comparer>
      friend class rbtree_const_iterator;

    public:

      constexpr static bool is_comparer_noexcept
        = noexcept(std::declval<Comparer>()(std::declval<Index>(), std::declval<Index>()));

      /** @brief get key of this node */
      const Index &get_key() const noexcept
      { return m_key; }

      static Index update_key(rbtree_node &node, Index key)
        noexcept(noexcept(std::swap(key, key)))
      { update_key(std::move(key), policy_unique); }

      static Index update_key(rbtree_node &node, Index key, policy_unique_t p)
        noexcept(noexcept(std::swap(key, key)))
      {
        auto *entry = node.unlink_checked();
        std::swap(key, node.m_key);
        if (entry)
          insert(*entry, &node, p);
        return key;
      }

      static Index update_key(rbtree_node &node, Index key, policy_override_t p)
        noexcept(noexcept(std::swap(key, key)))
      {
        auto *entry = node.unlink_checked();
        std::swap(key, node.m_key);
        if (entry)
          insert(*entry, &node, p);
        return key;
      }

      static Index update_key(rbtree_node &node, Index key, policy_frontmost_t p)
        noexcept(noexcept(std::swap(key, key)))
      {
        auto *entry = node.unlink_checked();
        std::swap(key, node.m_key);
        if (entry)
          insert(*entry, &node, p);
        return key;
      }

      static Index update_key(rbtree_node &node, Index key, policy_backmost_t p)
        noexcept(noexcept(std::swap(key, key)))
      {
        auto *entry = node.unlink_checked();
        std::swap(key, node.m_key);
        if (entry)
          insert(*entry, &node, p);
        return key;
      }

      static Index update_key(rbtree_node &node, Index key, policy_nearest_t p)
        noexcept(noexcept(std::swap(key, key)))
      {
        auto *entry = node.unlink_checked();
        std::swap(key, node.m_key);
        if (entry)
          insert(*entry, &node, p);
        return key;
      }

      static rbtree_node<void> *
      find(rbtree_node<void> &entry, const Index &key, policy_backmost_t)
      noexcept(is_comparer_noexcept)
      {
        auto *p = boundry(entry, key, key_fetcher,
            [](const Index &lhs, const Index &rhs)
            noexcept(is_comparer_noexcept) -> bool
            { return !cmper(rhs, lhs); }).first;

        if (cmper(key_fetcher(*p), key) != !cmper(key, key_fetcher(*p)))
          return p;
        else
          return nullptr;
      }

      static rbtree_node<void> *
      find(rbtree_node<void> &entry, const Index &key, policy_frontmost_t)
      noexcept(is_comparer_noexcept)
      {
        auto *p = boundry(entry, key, key_fetcher, cmper).second;

        if (cmper(key_fetcher(*p), key) != !cmper(key, key_fetcher(*p)))
          return p;
        else
          return nullptr;
      }

      static rbtree_node<void> *
      find(rbtree_node<void> &entry, const Index &key, policy_nearest_t)
      noexcept(is_comparer_noexcept)
      {
        auto p = search(entry, key, key_fetcher, cmper);

        if (p.first == p.second)
          return p.first;
        else
          return nullptr;
      }

      /**
       * @brief Unlink current node from a tree
       * @note User is responsible to ensure this node is already attached
       * into a tree
       */
      static void unlink(rbtree_node &node) noexcept
      { node.rbtree_node<void>::unlink(); }

      /**
       * @brief Get the first node in the tree whose key is not less than
       * specified value
       * @param entry Entry node for search
       * @param key The specified value for searching the lower boundry
       */
      static rbtree_node<void> *
      lower_bound(rbtree_node<void> &entry, const Index &key)
      noexcept(is_comparer_noexcept)
      { return boundry(entry, key, key_fetcher, cmper).second; }

      /**
       * @brief Get the first node in the tree whose key is not less than
       * specified value
       * @param entry Entry node for search
       * @param key The specified value for searching the lower boundry
       */
      static const rbtree_node<void> *
      lower_bound(const rbtree_node<void> &entry, const Index &key)
      noexcept(is_comparer_noexcept)
      { return boundry(entry, key, key_fetcher, cmper).second; }

      /**
       * @brief Get the first node in the tree whose key is greater than
       * specified value
       * @param entry Entry node for search
       * @param key The specified value for searching the upper boundry
       */
      static rbtree_node<void> *
      upper_bound(rbtree_node<void> &entry, const Index &key)
      noexcept(is_comparer_noexcept)
      {
        return boundry(entry, key, key_fetcher,
            [] (const Index &lhs, const Index &rhs) noexcept(is_comparer_noexcept)
            { return !cmper(rhs, lhs); }).second;
      }

      /**
       * @brief Get the first node in the tree whose key is greater than
       * specified value
       * @param entry Entry node for search
       * @param key The specified value for searching the upper boundry
       */
      static const rbtree_node<void> *
      upper_bound(const rbtree_node<void> &entry, const Index &key)
      noexcept(is_comparer_noexcept)
      {
        return boundry(entry, key, key_fetcher,
            [] (const Index &lhs, const Index &rhs) noexcept(is_comparer_noexcept)
            { return !cmper(rhs, lhs); }).second;
      }


      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param hint_node The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note User is responsible to ensure hint_node is already attached to a
       * tree; and if duplicate is permitted, the node is insert after any
       * node duplicate with this node
       */
      static rbtree_node<void> *
      insert(rbtree_node<void> &entry, rbtree_node &node, policy_backmost_t)
      noexcept(is_comparer_noexcept)
      {
        auto p = boundry(entry, node.get_key(), key_fetcher,
            [] (const Index &lhs, const Index &rhs) noexcept(is_comparer_noexcept)
            { return !cmper(rhs, lhs); });
        insert_between(p.first, p.second, &node);
        return &node;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param entry The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note Use is responsible to ensure hint_node is already attached to a
       * tree; and if duplicate is permitted, the node is insert before any
       * node duplicate with this node
       */
      static rbtree_node<void> *
      insert(rbtree_node<void> &entry, rbtree_node &node, policy_frontmost_t)
      noexcept(is_comparer_noexcept)
      {
        auto p = boundry(entry, node.get_key(), key_fetcher, cmper);
        insert_between(p.first, p.second, &node);
        return &node;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param entry The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note Use is responsible to ensure hint_node is already attached to a
       * tree; and if duplicate is permitted, the node is insert with least
       * searching being done
       */
      static rbtree_node<void> *
      insert(rbtree_node<void> &entry, rbtree_node &node, policy_nearest_t)
      noexcept(is_comparer_noexcept)
      {
        auto p = search(entry, node.get_key(), key_fetcher, cmper);
        insert_between(p.first, p.second, &node);
        return &node;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param hint_node The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note User is responsible to ensure hint_node is already attached to
       * a tree; and duplicated node is not allow
       */
      static rbtree_node<void> *
      insert(rbtree_node<void> &entry, rbtree_node &node, policy_unique_t)
      noexcept(is_comparer_noexcept)
      {
        auto p = search(entry, node.get_key(), key_fetcher, cmper);
        insert_unique(p.first, p.second, &node);
        return node.is_linked() ? &node : p.first;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param hint_node The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note User is responsible to ensure hint_node is already attached to
       * a tree; and duplicated node will be replaced by this node
       */
      static rbtree_node<void> *
      insert(rbtree_node<void> &entry, rbtree_node &node, policy_override_t)
      noexcept(is_comparer_noexcept)
      {
        auto p = search(entry, node.get_key(), key_fetcher, cmper);
        insert_override(p.first, p.second, &node);
        return &node;
      }

      /**
       * @brief Default constructor, forwarding all arguments to delegated key
       * type
       */
      template<typename ...Args>
      rbtree_node(Args && ...args)
        noexcept(noexcept(Index(std::forward<Args>(args)...)))
        : rbtree_node<void>()
        , m_key(std::forward<Args>(args)...)
      { }

      /** @brief Default destructor */
      ~rbtree_node() noexcept(noexcept(std::declval<Index>().~Index())) = default;

      /** @brief Move constructor */
      rbtree_node(rbtree_node &&n)
        noexcept(noexcept(Index(std::declval<Index>())))
        : rbtree_node<void>(std::move(n))
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
        noexcept(is_comparer_noexcept)
      { return cmper(lhs.m_key, rhs.m_key); }

      friend bool operator > (const rbtree_node &lhs, const rbtree_node &rhs)
        noexcept(is_comparer_noexcept)
      { return cmper(rhs.m_key, lhs.m_key); }

      friend bool operator <= (const rbtree_node &lhs, const rbtree_node &rhs)
        noexcept(is_comparer_noexcept)
      { return !cmper(rhs.m_key, lhs.m_key); }

      friend bool operator >= (const rbtree_node &lhs, const rbtree_node &rhs)
        noexcept(is_comparer_noexcept)
      { return !cmper(rhs.m_key, lhs.m_key); }

    private:

      static rbtree_node *internal_cast(rbtree_node<void> *x) noexcept
      { return static_cast<rbtree_node *>(x); }

      static const rbtree_node *internal_cast(const rbtree_node<void> *x) noexcept
      { return static_cast<const rbtree_node *>(x); }

      static class key_fetcher_t
      {
      public:
        const Index &operator () (const rbtree_node<void> &x) const noexcept
        { return internal_cast(&x)->get_key(); }
      } key_fetcher;

      static Comparer cmper;

      Index m_key;
    };

    template<typename Index, typename Tag, typename Comparer>
    Comparer rbtree_node<Index, Tag, Comparer>::cmper;

    template<typename Index, typename Tag, typename Comparer>
    typename rbtree_node<Index, Tag, Comparer>::key_fetcher_t
    rbtree_node<Index, Tag, Comparer>::key_fetcher;

    template<typename Index, typename Type, typename Tag, typename Comparer>
    class rbtree_iterator
    {
      static_assert(std::is_base_of<rbtree_node<Index>, Type>::value,
          "Type should be child class of rbtree_node<Index>");
    public:
      // Nested type similar with STL
      using iterator_category = std::bidirectional_iterator_tag;
      using node_type         = rbtree_node<void>;
      using value_type        = Type;
      using reference         = value_type &;
      using pointer           = value_type *;
      using difference_type   = std::ptrdiff_t;

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

      friend bool operator == (const rbtree_iterator &l,
          const rbtree_iterator &r) noexcept
      { return l.m_node == r.m_node; }

      friend bool operator != (const rbtree_iterator &l,
          const rbtree_iterator &r) noexcept
      { return !(l.m_node == r.m_node); }

    private:

      pointer internal_cast() const noexcept
      { return static_cast<pointer>(m_node); }

      node_type *m_node;
    };

    template<typename Index, typename Type, typename Tag, typename Comparer>
    class rbtree_const_iterator
    {
      static_assert(std::is_base_of<rbtree_node<Index>, Type>::value,
          "Type should be child class of rbtree_node<Index>");
    public:
      // Nested type similar with STL
      using iterator_category = std::bidirectional_iterator_tag;
      using node_type         = const rbtree_node<void>;
      using value_type        = const Type;
      using reference         = value_type &;
      using pointer           = value_type *;
      using difference_type   = std::ptrdiff_t;

      explicit rbtree_const_iterator(node_type *node) noexcept
        : m_node(node)
      { }

      rbtree_const_iterator
        (const rbtree_iterator<Index, Type, Tag, Comparer> &i) noexcept
        : m_node(&*i)
      { }

      rbtree_const_iterator
        (const rbtree_const_iterator &i) noexcept
        : m_node(i.m_node)
      { }

      rbtree_const_iterator &
        operator = (const rbtree_const_iterator &i) noexcept
      { m_node = i.m_node; }

      ~rbtree_const_iterator() noexcept = default;

      reference operator * () const noexcept
      { return *internal_cast(); }

      pointer operator -> () const noexcept
      { return internal_cast(); }

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

      friend bool operator == (const rbtree_const_iterator &l,
          const rbtree_const_iterator &r) noexcept
      { return l.m_node == r.m_node; }

      friend bool operator != (const rbtree_const_iterator &l,
          const rbtree_const_iterator &r) noexcept
      { return !(l.m_node == r.m_node); }

    private:
      pointer internal_cast() const noexcept
      { return static_cast<pointer>(m_node); }

      node_type *m_node;
    };



    template<typename Index, typename Type, typename Tag, typename Comparer>
    class rbtree
    {
      static_assert(std::is_base_of<rbtree_node<Index>, Type>::value,
          "Type should be child class of rbtree_node<Index>");
    public:
      // Nested type, similar with STL
      using iterator                = rbtree_iterator<Index, Type, Tag, Comparer>;
      using const_iterator          = rbtree_const_iterator<Index, Type, Tag, Comparer>;
      using reverse_iterator        = std::reverse_iterator<iterator>;
      using const_reverse_iterator  = std::reverse_iterator<const_iterator>;
      using node_type               = rbtree_node<Index, Tag, Comparer>;
      using value_type              = Type;
      using reference               = value_type &;
      using pointer                 = value_type *;
      using const_pointer           = const pointer;
      using const_reference         = const reference;
      using size_type               = size_t;
      using difference_type         = std::ptrdiff_t;

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

      iterator find(const Index &key)
      noexcept(node_type::is_comparer_noexcept)
      { return find(end(), key, policy_nearest); }

      iterator find(const Index &key, policy_nearest_t p)
      noexcept(node_type::is_comparer_noexcept)
      { return find(end(), key, p); }

      iterator find(const Index &key, policy_frontmost_t p)
      noexcept(node_type::is_comparer_noexcept)
      { return find(end(), key, p); }

      iterator find(const Index &key, policy_backmost_t p)
      noexcept(node_type::is_comparer_noexcept)
      { return find(end(), key, p); }

      iterator find(iterator hint, const Index &key)
      noexcept(node_type::is_comparer_noexcept)
      { return find(hint, key, policy_nearest); }

      iterator find(iterator hint, const Index &key, policy_nearest_t p)
      noexcept(node_type::is_comparer_noexcept)
      {
        auto *result = node_type::find(*hint, key, p);
        if (result == nullptr) return end();
        else return iterator(result);
      }

      iterator find(iterator hint, const Index &key, policy_frontmost_t p)
      noexcept(node_type::is_comparer_noexcept)
      {
        auto *result = node_type::find(*hint, key, p);
        if (result == nullptr) return end();
        else return iterator(result);
      }

      iterator find(iterator hint, const Index &key, policy_backmost_t p)
      noexcept(node_type::is_comparer_noexcept)
      {
        auto *result = node_type::find(*hint, key, p);
        if (result == nullptr) return end();
        else return iterator(result);
      }

      const_iterator
      find(const_iterator hint, const Index &key, policy_nearest_t p) const
      noexcept(node_type::is_comparer_noexcept)
      {
        auto *result = node_type::find(*hint, key, p);
        if (result == nullptr) return end();
        else return const_iterator(result);
      }

      const_iterator
      find(const_iterator hint, const Index &key, policy_frontmost_t p) const
      noexcept(node_type::is_comparer_noexcept)
      {
        auto *result = node_type::find(*hint, key, p);
        if (result == nullptr) return end();
        else return const_iterator(result);
      }

      const_iterator
      find(const_iterator hint, const Index &key, policy_backmost_t p) const
      noexcept(node_type::is_comparer_noexcept)
      {
        auto *result = node_type::find(*hint, key, p);
        if (result == nullptr) return end();
        else return const_iterator(result);
      }

      std::pair<iterator, iterator>
      equals_range(const Index &key)
      noexcept(node_type::is_comparer_noexcept)
      { return equals_range(end(), key); }

      std::pair<iterator, iterator>
      equals_range(iterator hint, const Index &key)
      noexcept(node_type::is_comparer_noexcept)
      {
        auto l = lower_bound(hint, key);
        auto u = lower_bound(l, key);
        return std::make_pair(std::move(l), std::move(u));
      }

      std::pair<const_iterator, const_iterator>
      equals_range(const Index &key) const
      noexcept(node_type::is_comparer_noexcept)
      { return equals_range(end(), key); }

      std::pair<const_iterator, const_iterator>
      equals_range(const_iterator hint, const Index &key) const
      noexcept(node_type::is_comparer_noexcept)
      {
        auto l = lower_bound(hint, key);
        auto u = lower_bound(l, key);
        return std::make_pair(std::move(l), std::move(u));
      }

      iterator lower_bound(const Index &key)
      noexcept(node_type::is_comparer_noexcept)
      { return lower_bound(end(), key); }

      const_iterator lower_bound(const Index &key) const
      noexcept(node_type::is_comparer_noexcept)
      { return lower_bound(end(), key); }

      iterator lower_bound(iterator hint, const Index &key)
      noexcept(node_type::is_comparer_noexcept)
      { return iterator(node_type::lower_bound(*hint, key)); }

      const_iterator lower_bound(const_iterator hint, const Index &key) const
      noexcept(node_type::is_comparer_noexcept)
      { return const_iterator(node_type::lower_bound(*hint, key)); }

      iterator upper_bound(const Index &key)
      noexcept(node_type::is_comparer_noexcept)
      { return upper_bound(end(), key); }

      const_iterator upper_bound(const Index &key) const
      noexcept(node_type::is_comparer_noexcept)
      { return upper_bound(end(), key); }

      iterator upper_bound(iterator hint, const Index &key)
      noexcept(node_type::is_comparer_noexcept)
      { return iterator(node_type::upper_bound(*hint, key)); }

      const_iterator
      upper_bound(const_iterator hint, const Index &key) const
      noexcept(node_type::is_comparer_noexcept)
      { return const_iterator(node_type::upper_bound(*hint, key)); }

      // Modifier

      iterator insert(node_type &val)
      noexcept(node_type::is_comparer_noexcept)
      { return insert(end(), val, policy_unique); }

      iterator insert(node_type &val, policy_unique_t p)
      noexcept(node_type::is_comparer_noexcept)
      { return insert(end(), val, p); }

      iterator insert(node_type &val, policy_override_t p)
      noexcept(node_type::is_comparer_noexcept)
      { return insert(end(), val, p); }

      iterator insert(node_type &val, policy_frontmost_t p)
      noexcept(node_type::is_comparer_noexcept)
      { return insert(end(), val, p); }

      iterator insert(node_type &val, policy_backmost_t p)
      noexcept(node_type::is_comparer_noexcept)
      { return insert(end(), val, p); }

      iterator insert(node_type &val, policy_nearest_t p)
      noexcept(node_type::is_comparer_noexcept)
      { return insert(end(), val, p); }

      iterator insert(iterator hint, node_type &val)
      noexcept(node_type::is_comparer_noexcept)
      { return insert(end(), val, policy_unique); }

      iterator insert(iterator hint, node_type &val, policy_unique_t p)
      noexcept(node_type::is_comparer_noexcept)
      { return iterator(node_type::insert(*hint, val, p)); }

      iterator insert(iterator hint, node_type &val, policy_override_t p)
      noexcept(node_type::is_comparer_noexcept)
      { return iterator(node_type::insert(*hint, val, p)); }

      iterator insert(iterator hint, node_type &val, policy_frontmost_t p)
      noexcept(node_type::is_comparer_noexcept)
      { return iterator(node_type::insert(*hint, val, p)); }

      iterator insert(iterator hint, node_type &val, policy_backmost_t p)
      noexcept(node_type::is_comparer_noexcept)
      { return iterator(node_type::insert(*hint, val, p)); }

      iterator insert(iterator hint, node_type &val, policy_nearest_t p)
      noexcept(node_type::is_comparer_noexcept)
      { return iterator(node_type::insert(*hint, val, p)); }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e)
      noexcept(node_type::is_comparer_noexcept)
      {
        while (b != e)
          insert(*b++);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint)
      noexcept(node_type::is_comparer_noexcept)
      {
        while (b != e)
          insert(hint, *b++);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          policy_unique_t p)
      noexcept(node_type::is_comparer_noexcept)
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          policy_override_t p)
      noexcept(node_type::is_comparer_noexcept)
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          policy_frontmost_t p)
      noexcept(node_type::is_comparer_noexcept)
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          policy_backmost_t p)
      noexcept(node_type::is_comparer_noexcept)
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          policy_nearest_t p)
      noexcept(node_type::is_comparer_noexcept)
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      void erase(iterator iter) noexcept
      { node_type::unlink(*iter); }

      void erase(iterator b, iterator e) noexcept
      {
        for (auto i = b; i != e; ++i)
          node_type::unlink(*i++);
      }

      void remove(Index &key)
      noexcept(node_type::is_comparer_noexcept)
      {
        auto b = upper_bound(end(), key);
        auto e = lower_bound(b, key);
        erase(b, e);
      }

      template<typename Predicate>
      void remove(Predicate &&predicate)
      noexcept(noexcept(predicate(std::declval<Index>())))
      {
        auto b = begin(), e = end();
        while (b != e)
        {
          auto x = b++;
          if (predicate(x->get_key()))
            erase(x);
        }
      }

      void swap(rbtree &&t) noexcept
      { swap(t); }

      void swap(rbtree &t) noexcept
      { node_type::swap_nodes(m_container_node, t.m_container_node); }

      void clear() noexcept
        // Can be optimize
      { erase(begin(), end()); }

      const Comparer &key_comp() const noexcept
      { return node_type::cmper; }

      const Comparer &key_comp() noexcept
      { return node_type::cmper; }

    private:
      rbtree_node<void> m_container_node;
    };
  }
}

#endif
