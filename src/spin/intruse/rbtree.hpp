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

    extern class policy_frontmost_t  {} policy_frontmost;
    extern class policy_backmost_t   {} policy_backmost;
    extern class policy_nearest_t    {} policy_nearest;
    extern class policy_override_t   {} policy_override;
    extern class policy_unique_t     {} policy_unique;


    template<typename Index, typename Type, typename ...Arguments>
    class rbtree_node;

    template<typename Index, typename Type, typename Tag, typename Comparator>
    class rbtree_iterator;

    template<typename Index, typename Type, typename Tag, typename Comparator>
    class rbtree_const_iterator;

    template<typename Index, typename Type,
      typename Tag = void, typename Comparator = less<Index>>
    class rbtree;

    /**
     * @brief rbtree_node specialization for void index type, used as base class
     * of all the other rbtree_node, and hide the implementation detail
     */
    template<>
    class __SPIN_EXPORT__ rbtree_node<void, void>
    {

      // Friend classes

      template<typename, typename, typename...>
      friend class rbtree_node;

      template<typename, typename, typename, typename>
      friend class rbtree;

      template<typename, typename, typename, typename>
      friend class rbtree_iterator;

      template<typename, typename, typename, typename>
      friend class rbtree_const_iterator;

    protected:

      /** @brief Auxilary class used for @a rbtree_node(container_tag) */
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

      /** @brief Do clean up work after unlink */
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
       * @brief Unlink this node from the tree if this node is already linked
       * @returns the successor of this node, or nullptr if this node is not
       * linked into a rbtree
       * @note However it doesn't not check if this node is container node of
       * a rbtree, and unlink a container node is always cause undefined
       * behaviours
       */
      rbtree_node *unlink_checked() noexcept;

      /**
       * @brief Insert node as parent's chlid, left child or right chlid are
       * all possible.
       * @param parent The node want child
       * @param node The node want to be inserted
       */
      static void insert(rbtree_node *parent, rbtree_node *node) noexcept;

      /**
       * @brief Insert a node between prev and next
       * @param prev The node will become predecessor of node after insertion
       * is done
       * @param next The node will be successor of node after insertion is
       * done
       * @param node The node want to be inserted
       * @note User code should ensure that #prev is predecessor of #next
       */
      static void insert_between(rbtree_node *prev,
          rbtree_node *next, rbtree_node *node) noexcept;

      /**
       * @brief Insert a node between prev and next in condition that no index
       * will become duplicated
       * @param prev The node will become predecessor of the node after insertion
       * is done
       * @param next The node will be successor of the node after insertion is
       * done
       * @param node The node want to be inserted
       * @note User code should ensure that prev is predecessor of next
       */
      static void insert_unique(rbtree_node *prev,
          rbtree_node *next, rbtree_node *node) noexcept;

      /**
       * @brief Insert a node between prev and next and another node with same index
       * will be overrided (unlinked from tree)
       * @param prev The node will become predecessor of the node after insertion
       * is done
       * @param next The node will be successor of the node after insertion is
       * done
       * @param node The node want to be inserted
       * @note User code should ensure that prev is predecessor of #next
       */
      static void insert_override(rbtree_node *prev,
          rbtree_node *next, rbtree_node *node) noexcept;

      /**
       * @brief Search the position where specified index is suitable to be
       * @tparam Index the Index type of rbtree
       * @tparam IndexFetcher the type of indexfetcher functor
       * @tparam Comparator the type of comparator functor
       * @param entry A node in the tree which we want to search in
       * @param index The index we want to search for
       * @param indexfetcher Functor which cast rbtree_node to Index
       * type
       * @param comparator Functor which compares index
       * @returns a pair of pointer to rbtree_node, if the pair is the same,
       * then the index is equals to the index of node which the two pointer
       * point to, else if the pair is not the same, the index is greater than
       * the first of the pair and less than the second of the pair
       */
      template<typename Index, typename IndexFetcher, typename Comparator>
      static std::pair<rbtree_node*, rbtree_node*> search(rbtree_node &entry,
          const Index &index, IndexFetcher && indexfetcher, Comparator && comparator)
          noexcept(noexcept(std::forward<IndexFetcher>(indexfetcher)(entry))
            && noexcept(std::forward<Comparator>(comparator)(index, index)))
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
          return std::forward<Comparator>(comparator)(
              std::forward<IndexFetcher>(indexfetcher)(*node), index);
        };

        auto rcmp = [&] (rbtree_node *node) -> bool
        {
          return !std::forward<Comparator>(comparator)(index,
              std::forward<IndexFetcher>(indexfetcher)(*node));
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
       * @brief Search the position where specified index is suitable to be
       * @tparam Index the Index type of rbtree
       * @tparam IndexFetcher the type of indexfetcher functor
       * @tparam Comparator the type of comparator functor
       * @param entry A node in the tree which we want to search in
       * @param index The index we want to search for
       * @param indexfetcher Functor which cast rbtree_node to Index
       * type
       * @param comparator Functor which compares index
       * @returns a pair of pointer to rbtree_node, if the pair is the same,
       * then the index is equals to the index of node which the two pointer
       * point to, else if the pair is not the same, the index is greater than
       * the first of the pair and less than the second of the pair
       */
      template<typename Index, typename IndexFetcher, typename Comparator>
      static std::pair<const rbtree_node*, const rbtree_node*> search(
          const rbtree_node &entry, const Index &index,
          IndexFetcher && indexfetcher, Comparator && comparator)
          noexcept(noexcept(std::forward<IndexFetcher>(indexfetcher)(entry))
            && noexcept(std::forward<Comparator>(comparator)(index, index)))
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
          return std::forward<Comparator>(comparator)(
              std::forward<IndexFetcher>(indexfetcher)(*node), index);
        };

        auto rcmp = [&] (rbtree_node *node) -> bool
        {
          return !std::forward<Comparator>(comparator)(index,
              std::forward<IndexFetcher>(indexfetcher)(*node));
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
              if (x != y) return std::make_pair(p->m_p, p->m_p);
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
              if (x != y) return std::make_pair(p->m_r, p->m_r);
              if (x) p = p->m_r;
              else break;
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
              if (x != y) return std::make_pair(p->m_p, p->m_p);
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
              if (x != y) return std::make_pair(p->m_l, p->m_l);
              if (x) p = p->m_l;
              else break;
            }
            else return std::make_pair(p->m_l, p);
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
       * @brief Get the boundry of a rbtree for specified index
       * @tparam Index the type indexed
       * @tparam IndexFetcher type whose instances are callable that cast an
       * rbtree_node<void, void> reference to Index type
       * @tparam Comparator type whose instances are callable that compare two
       * instances of Index type
       * @param entry The entry node of tree
       * @param index The specified index depends on which the boundry is find
       * @param indexfetcher An instance of IndexFetcher
       * @param comparator An instance of Comparator
       */
      template<typename Index, typename IndexFetcher, typename Comparator>
      static std::pair<rbtree_node*, rbtree_node*> boundry(rbtree_node &entry,
          const Index &index, IndexFetcher && indexfetcher, Comparator && comparator)
        noexcept(noexcept(std::forward<IndexFetcher>(indexfetcher)(entry))
            && noexcept(std::forward<Comparator>(comparator)(index, index)))
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
          return std::forward<Comparator>(comparator)(
              std::forward<IndexFetcher>(indexfetcher)(*node), index);
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
              if (cmper(p->m_r)) p = p->m_r;
              else break;
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
              if (!cmper(p->m_l)) p = p->m_l;
              else break;
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
            if (p->m_has_r) p = p->m_r;
            else return std::make_pair(p, p->m_r);
          }
          else
          {
            if (p->m_has_l) p = p->m_l;
            else return std::make_pair(p->m_l, p);
          }

          hint_result = cmper(p);
        }
      }

      /**
       * @brief Get the boundry of a rbtree for specified index
       * @tparam Index the type indexed
       * @tparam IndexFetcher type whose instances are callable that cast an
       * rbtree_node<void, void> reference to Index type
       * @tparam Comparator type whose instances are callable that compare two
       * instances of Index type
       * @param entry The entry node of tree
       * @param index The specified index depends on which the boundry is find
       * @param indexfetcher An instance of IndexFetcher
       * @param comparator An instance of Comparator
       */
      template<typename Index, typename IndexFetcher, typename Comparator>
      static std::pair<const rbtree_node*, const rbtree_node*> boundry(
          const rbtree_node &entry, const Index &index, IndexFetcher && indexfetcher,
          Comparator && comparator)
        noexcept(noexcept(std::forward<IndexFetcher>(indexfetcher)(entry))
            && noexcept(std::forward<Comparator>(comparator)(index, index)))
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
          return std::forward<Comparator>(comparator)(
              std::forward<IndexFetcher>(indexfetcher)(*node), index);
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
              if (cmper(p->m_r)) p = p->m_r;
              else break;
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
              if (!cmper(p->m_l)) p = p->m_l;
              else break;
            }
            else
              return std::make_pair(p->m_l, p);
          }
        }

        for ( ; ; )
        {
          if (hint_result)
          {
            if (p->m_has_r) p = p->m_r;
            else return std::make_pair(p, p->m_r);
          }
          else
          {
            if (p->m_has_l) p = p->m_l;
            else return std::make_pair(p->m_l, p);
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
      swap_nodes(rbtree_node &lhs, rbtree_node &rhs) noexcept;

      rbtree_node *m_p;
      rbtree_node *m_l;
      rbtree_node *m_r;
      bool m_has_l;
      bool m_has_r;
      bool m_is_red;
      bool m_is_container;

      /********************* Meta programming stuff **********************/
    public:

      template<typename Tag, typename Comparator>
      struct args
      {
        using tag = Tag;
        using comparator = Comparator;
      };

      template<typename ...Arguments>
      struct args_list;

      template<typename Tag, typename ArgsList>
      struct select_argument;

      template<typename Tag>
      struct select_argument<Tag, args_list<>>
      {
        static constexpr bool found = false;
        using argument = void;
      };

      template<typename Tag, typename Current, typename ...ArgsList>
      struct select_argument<Tag, args_list<Current, ArgsList...>>
      {
        static constexpr bool found = select_argument<Tag, args_list<ArgsList...>>::found;
        using argument = typename select_argument<Tag, args_list<ArgsList...>>::argument;
      };

      template<typename Tag, typename Comparator, typename ...ArgsList>
      struct select_argument<Tag, args_list<args<Tag, Comparator>, ArgsList...>>
      {
        static constexpr bool found = true;
        using argument = args<Tag, Comparator>;
      };

      // Test select_argument
      static_assert(select_argument<void, args_list<args<void, spin::less<int>>>>::found, "");
      static_assert(!select_argument<int, args_list<args<void, void>>>::found, "");
      static_assert(select_argument<int, args_list<args<void, void>, args<int, void>>>::found, "");
      static_assert(select_argument<void, args_list<args<void, spin::less<int>>, args<int, spin::less<int>>>>::found, "");
      static_assert(!select_argument<float, args_list<args<void, void>, args<int, void>>>::found, "");

      template<typename A, typename B>
      struct concat_args_list;

      template<typename A, typename C, typename ...B>
      struct concat_args_list<args<A, C>, args_list<B...>>
      {
        static_assert(select_argument<A, args_list<B...>>::found == false,
            "Tag conflict");
        using type = args_list<args<A, C>, B...>;
      };


      template<typename DefaultComparator, typename ...Arguments>
      struct make_args_list;

      template<typename DefaultComparator>
      struct make_args_list<DefaultComparator>
      {
        using type = args_list<args<void, DefaultComparator>>;
      };

      template<typename DefaultComparator, typename Tag>
      struct make_args_list<DefaultComparator, Tag>
      {
        using type = args_list<args<Tag, DefaultComparator>>;
      };

      template<typename DefaultComparator, typename Tag, typename PendingArgument>
      struct make_args_list<DefaultComparator, Tag, PendingArgument>
      {
        static constexpr bool pending_argument_is_comparator
          = std::is_default_constructible<PendingArgument>::value;
        using type = typename std::conditional<

            // if
            pending_argument_is_comparator,

            // then
            args_list<args<Tag, PendingArgument>>,

            // else
            typename concat_args_list<
              args<Tag, DefaultComparator>,
              typename make_args_list<DefaultComparator, PendingArgument>::type
            >::type // make_args_list

          >::type; // std::conditional
      };

      template<typename DefaultComparator, typename Tag,
        typename PendingArgument, typename ...ExtraRemains>
      struct make_args_list<DefaultComparator, Tag, PendingArgument, ExtraRemains...>
      {
        static constexpr bool pending_argument_is_comparator
          = std::is_default_constructible<PendingArgument>::value;
        using type = typename std::conditional<

            // if
            pending_argument_is_comparator,

            // then
            typename concat_args_list<args<Tag, PendingArgument>,
              typename make_args_list<DefaultComparator, ExtraRemains...>::type
            >::type,

            // else
            typename concat_args_list<args<Tag, DefaultComparator>,
              typename make_args_list<DefaultComparator, PendingArgument, ExtraRemains...>::type
            >::type // make_args_list

          >::type; // std::conditoinal
      };

      template<typename Index, bool is_class=std::is_class<Index>::value>
      class index_holder
      {
        template<typename, typename, typename...> friend class rbtree_node;
      public:

        index_holder(Index index) noexcept
          : m_index(std::move(index))
        { }

        static const Index &get_index(const index_holder &x) noexcept
        { return x.m_index; }

      private:

        static Index &internal_get_index(index_holder &x) noexcept
        { return x.m_index; }

        Index m_index;
      };

      /*
       * @brief Specializaton for class type Index
       * @note We inheriate from Index when it's a class, so it's possible
       * to override its method
       */
      template<typename Index>
      class index_holder<Index, true> : private Index
      {
        template<typename, typename, typename...> friend class rbtree_node;
      private:

        static Index &internal_get_index(index_holder &x) noexcept
        { return x; }
      public:
        index_holder(Index index)
          noexcept(std::is_nothrow_move_constructible<Index>::value)
          : Index(std::move(index))
        { }

        static const Index &get_index(const index_holder &x) noexcept
        { return x; }
      };


      template<typename Index, typename Type, typename Argument>
      struct base_node;

      template<typename Index, typename Type, typename Argument>
      struct base_node<Index, Type, args_list<Argument>>
        : public rbtree_node<Index, Type, Argument>
      { };

      template<typename Index, typename Type, typename ...Arguments>
      struct base_node<Index, Type, args_list<Arguments...>>
        : public rbtree_node<Index, Type, Arguments>...
      { };

    };

    template<typename Index, typename Type, typename Tag, typename Comparator>
    class rbtree_node<Index, Type,
          rbtree_node<void, void>::args<Tag, Comparator>>
      : public rbtree_node<void, void>
    {
      friend class rbtree<Index, Type, Tag, Comparator>;
      friend class rbtree_iterator<Index, Type, Tag, Comparator>;
      friend class rbtree_const_iterator<Index, Type, Tag, Comparator>;

      using indexed_node_type = rbtree_node<Index, Type, Tag, Comparator>;

      static const Index &internal_get_index(const rbtree_node &node) noexcept
      {
        const rbtree_node<void, void>::index_holder<Index> &ref
          = static_cast<const Type&>(node);

        return rbtree_node<void, void>::index_holder<Index>::get_index(ref);
      }

      static Index &internal_get_index(rbtree_node &node) noexcept
      {
        rbtree_node<void, void>::index_holder<Index> &ref
          = static_cast<Type&>(node);

        return rbtree_node<void, void>::index_holder<Index>::internal_get_index(ref);
      }

      rbtree_node(rbtree_node<void, void>::container_tag tag)
        : rbtree_node<void, void>(tag)
      {}

    public:

      rbtree_node() = default;

      /** @brief whether instance Comparator would throw exception when
       * comparing index*/
      constexpr static bool is_comparator_noexcept
        = noexcept(std::declval<Comparator>()(std::declval<Index>(),
              std::declval<Index>()));
      /**
       * @brief Test whether a node is attached to a tree
       * @param The node to be test
       */
      static bool is_linked(const rbtree_node &node) noexcept
      { return node.rbtree_node<void, void>::is_linked(); }

      /**
       * @brief Unlink current node from a tree
       * @note User is responsible to ensure this node is already attached
       * into a tree
       */
      static void unlink(rbtree_node &node) noexcept
      { node.rbtree_node<void, void>::unlink(); }

      /** @brief Update index of this node */
      static Index update_index(rbtree_node &node, Index index)
          noexcept(noexcept(std::swap(index, index)))
      { return update_index(node, std::move(index), policy_unique); }

      /** @brief Update index of this node */
      static Index update_index(rbtree_node &node, Index index, policy_unique_t p)
          noexcept(noexcept(std::swap(index, index)))
      {
        auto *entry = node.unlink_checked();
        std::swap(index, internal_get_index(node));
        if (entry)
          insert(*entry, node, p);
        return index;
      }

      static Index update_index(rbtree_node &node, Index index, policy_override_t p)
          noexcept(noexcept(std::swap(index, index)))
      {
        auto *entry = node.unlink_checked();
        std::swap(index, internal_get_index(node));
        if (entry)
          insert(*entry, node, p);
        return index;
      }

      static Index update_index(rbtree_node &node, Index index, policy_frontmost_t p)
          noexcept(noexcept(std::swap(index, index)))
      {
        auto *entry = node.unlink_checked();
        std::swap(index, internal_get_index(node));
        if (entry)
          insert(*entry, node, p);
        return index;
      }

      static Index update_index(rbtree_node &node, Index index, policy_backmost_t p)
          noexcept(noexcept(std::swap(index, index)))
      {
        auto *entry = node.unlink_checked();
        std::swap(index, internal_get_index(node));
        if (entry)
          insert(*entry, node, p);
        return index;
      }

      static Index update_index(rbtree_node &node, Index index, policy_nearest_t p)
          noexcept(noexcept(std::swap(index, index)))
      {
        auto *entry = node.unlink_checked();
        std::swap(index, internal_get_index(node));
        if (entry)
          insert(*entry, node, p);
        return index;
      }
    private:

      static rbtree_node<void, void> *find(rbtree_node<void, void> &entry,
          const Index &index, policy_backmost_t) noexcept(is_comparator_noexcept)
      {
        auto *p = boundry(entry, index, index_fetcher,
            [](const Index &lhs, const Index &rhs)
            noexcept(is_comparator_noexcept) -> bool
            { return !cmper(rhs, lhs); }).first;

        if (cmper(index_fetcher(*p), index) != !cmper(index, index_fetcher(*p)))
          return p;
        else
          return nullptr;
      }

      static rbtree_node<void, void> *find(rbtree_node<void, void> &entry,
          const Index &index, policy_frontmost_t) noexcept(is_comparator_noexcept)
      {
        auto *p = boundry(entry, index, index_fetcher, cmper).second;

        if (cmper(index_fetcher(*p), index) != !cmper(index, index_fetcher(*p)))
          return p;
        else
          return nullptr;
      }

      static rbtree_node<void, void> *find(rbtree_node<void, void> &entry,
          const Index &index, policy_nearest_t) noexcept(is_comparator_noexcept)
      {
        auto p = search(entry, index, index_fetcher, cmper);

        if (p.first == p.second)
          return p.first;
        else
          return nullptr;
      }

      /**
       * @brief Get the first node in the tree whose index is not less than
       * specified value
       * @param entry Entry node for search
       * @param index The specified value for searching the lower boundry
       */
      static rbtree_node<void, void> *lower_bound(rbtree_node<void, void> &entry,
          const Index &index) noexcept(is_comparator_noexcept)
      { return boundry(entry, index, index_fetcher, cmper).second; }

      /**
       * @brief Get the first node in the tree whose index is not less than
       * specified value
       * @param entry Entry node for search
       * @param index The specified value for searching the lower boundry
       */
      static const rbtree_node<void, void> *lower_bound(const rbtree_node<void, void> &entry,
          const Index &index) noexcept(is_comparator_noexcept)
      { return boundry(entry, index, index_fetcher, cmper).second; }

      /**
       * @brief Get the first node in the tree whose index is greater than
       * specified value
       * @param entry Entry node for search
       * @param index The specified value for searching the upper boundry
       */
      static rbtree_node<void, void> *upper_bound(rbtree_node<void, void> &entry,
          const Index &index) noexcept(is_comparator_noexcept)
      {
        return boundry(entry, index, index_fetcher,
            [] (const Index &lhs, const Index &rhs) noexcept(is_comparator_noexcept)
            { return !cmper(rhs, lhs); }).second;
      }

      /**
       * @brief Get the first node in the tree whose index is greater than
       * specified value
       * @param entry Entry node for search
       * @param index The specified value for searching the upper boundry
       */
      static const rbtree_node<void, void> *upper_bound(const rbtree_node<void, void> &entry,
          const Index &index) noexcept(is_comparator_noexcept)
      {
        return boundry(entry, index, index_fetcher,
            [] (const Index &lhs, const Index &rhs) noexcept(is_comparator_noexcept)
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
      static rbtree_node<void, void> *insert(rbtree_node<void, void> &entry,
          rbtree_node &node, policy_backmost_t) noexcept(is_comparator_noexcept)
      {
        auto p = boundry(entry, internal_get_index(node), index_fetcher,
            [] (const Index &lhs, const Index &rhs) noexcept(is_comparator_noexcept)
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
      static rbtree_node<void, void> *insert(rbtree_node<void, void> &entry,
          rbtree_node &node, policy_frontmost_t) noexcept(is_comparator_noexcept)
      {
        auto p = boundry(entry, internal_get_index(node), index_fetcher, cmper);
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
      static rbtree_node<void, void> *insert(rbtree_node<void, void> &entry,
          rbtree_node &node, policy_nearest_t) noexcept(is_comparator_noexcept)
      {
        auto p = search(entry, internal_get_index(node), index_fetcher, cmper);
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
      static rbtree_node<void, void> *insert(rbtree_node<void, void> &entry,
          rbtree_node &node, policy_unique_t) noexcept(is_comparator_noexcept)
      {
        auto p = search(entry, internal_get_index(node), index_fetcher, cmper);
        insert_unique(p.first, p.second, &node);
        return is_linked(node) ? &node : p.first;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param hint_node The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note User is responsible to ensure hint_node is already attached to
       * a tree; and duplicated node will be replaced by this node
       */
      static rbtree_node<void, void> *insert(rbtree_node<void, void> &entry,
          rbtree_node &node, policy_override_t) noexcept(is_comparator_noexcept)
      {
        auto p = search(entry, internal_get_index(node), index_fetcher, cmper);
        insert_override(p.first, p.second, &node);
        return &node;
      }

      static rbtree_node *internal_cast(rbtree_node<void, void> *x) noexcept
      { return static_cast<rbtree_node *>(x); }

      static const rbtree_node *internal_cast(const rbtree_node<void, void> *x) noexcept
      { return static_cast<const rbtree_node *>(x); }

      static class index_fetcher_t
      {
      public:
        const Index &operator () (const rbtree_node<void, void> &x) const noexcept
        { return internal_get_index(*internal_cast(&x)); }

      } index_fetcher;

      static Comparator cmper;

    };

    template<typename Index, typename Type, typename Tag, typename Comparator>
    Comparator rbtree_node<Index, Type,
      rbtree_node<void, void>::args<Tag, Comparator>>::cmper;

    template<typename Index, typename Type, typename Tag, typename Comparator>
    typename rbtree_node<Index, Type,
      rbtree_node<void, void>::args<Tag, Comparator>>::index_fetcher_t
    rbtree_node<Index, Type,
      rbtree_node<void, void>::args<Tag, Comparator>>::index_fetcher;

    /**
     * @brief Definition of rbtree_node
     * @tparam Index  Representing the type of value which the order of the
     *                is based on
     * @tparam Type   Representing the type which inheriats this rbtree_node
     * @tparam Arguments
     *                Representing the options for this node
     *
     * In most situation, in order to use rbtree to index @p Type by @p Index,
     * you need to let class @Type inheriats from rbtree_node<Index, Type>,
     * this will injects an Red-Black Tree data structure into @p Type, so
     * rbtree<Index, Type> can operates it. That's the simple way to use an
     * this class.
     *
     * When you are in a situation that some advanced features is need, such as
     * you want a custom comparator, want to make it derived from multiple
     * node so it can be stored by multiple tree simultaneously. You'll want
     * to know the rbtree_node options is parsed in the following form.
     *
     * @code
     *  rbtree_node<Index, Type, [Tag, [Comparator]], ...>
     *
     *  class X : public spin::intruse::rbtree_node<int, X,
     *                     tag1, spin::greater<int>,
     *                     tag2, // default to spin::less<int>
     *                     tag3, custom_comparator>
     *          , public spin::intruse::rbtree_node<float, X, tag1>
     *          , public spin::intruse::rbtree_node<Y, X>
     *  {
     *  // ...
     *  }
     *
     *  // Corresponding rbtree are:
     *  spin::intruse::rbtree<int, X, tag1, spin::greater<int>>
     *  spin::intruse::rbtree<int, X, tag2(, spin::less<int>)>
     *  spin::intruse::rbtree<int, X, tag3, custom_comparator>
     *  spin::intruse::rbtree<float, X, tag1, spin::tag1(, spin::less<float>)>
     *  spin::intruse::rbtree<Y, X(, void, (spin::less<Y>))>
     *
     * @endcode
     *
     * There should not be same tag in options, or a static_assert will
     * failed.
     * And a Tag should be a incomplete class or non-default-nconstructible
     * class while a Comparator is should not be. Be care about this,
     * otherwise the compiler may complains you with a horrible error message.
     *
     * By design, rbtree and its nodes is fully RAII complaince. The node will
     * remove itself from the tree if it's linked when it got destroyed
     *
     * @see rbtree
     */
    template<typename Index, typename Type, typename ...Arguments>
    class rbtree_node
      : public rbtree_node<void, void>::index_holder<Index>
      , public rbtree_node<void, void>::base_node<Index, Type,
          typename rbtree_node<void, void>::make_args_list<
          spin::less<Index>, Arguments...>::type
        >
    {
      using index_holder = rbtree_node<void, void>::index_holder<Index>;
      using args_list = typename rbtree_node<void, void>::make_args_list<
          spin::less<Index>, Arguments...>::type;

      using base_node = rbtree_node<void, void>::base_node<Index, Type,
            args_list>;

    public:

      rbtree_node(Index index)
        noexcept(std::is_nothrow_move_constructible<Index>::value)
        : index_holder(std::move(index))
        , base_node()
      { }

      rbtree_node(rbtree_node &&node)
        noexcept(std::is_nothrow_move_constructible<index_holder>::value)
        : index_holder(std::move(node))
        , base_node(std::move(node))
      { }


      ~rbtree_node() = default;

      template<typename Tag=void>
      static void update_index(rbtree_node &node, Index index)
      {
        static_assert(rbtree_node<void, void>
            ::select_argument<Tag, args_list>::found,
            "Tag was not found in this node");

        using node_type = rbtree_node<Index, Type,
              typename rbtree_node<void, void>
                ::select_argument<Tag, args_list>::argument>;

        node_type &ref = node;
        node_type::update_index(ref, std::move(index));
      }

      template<typename Tag=void>
      static void update_index(rbtree_node &node, Index index, policy_override_t p)
      {
        static_assert(rbtree_node<void, void>
            ::select_argument<Tag, args_list>::found,
            "Tag was not found in this node");

        using node_type = rbtree_node<Index, Type,
              typename rbtree_node<void, void>
                ::select_argument<Tag, args_list>::argument>;

        node_type &ref = node;
        node_type::update_index(ref, std::move(index), p);
      }

      template<typename Tag=void>
      static void update_index(rbtree_node &node, Index index, policy_unique_t p)
      {
        static_assert(rbtree_node<void, void>
            ::select_argument<Tag, args_list>::found,
            "Tag was not found in this node");

        using node_type = rbtree_node<Index, Type,
              typename rbtree_node<void, void>
                ::select_argument<Tag, args_list>::argument>;

        node_type &ref = node;
        node_type::update_index(ref, std::move(index), p);
      }

      template<typename Tag=void>
      static void update_index(rbtree_node &node, Index index, policy_nearest_t p)
      {
        static_assert(rbtree_node<void, void>
            ::select_argument<Tag, args_list>::found,
            "Tag was not found in this node");

        using node_type = rbtree_node<Index, Type,
              typename rbtree_node<void, void>
                ::select_argument<Tag, args_list>::argument>;

        node_type &ref = node;
        node_type::update_index(ref, std::move(index), p);
      }

      template<typename Tag=void>
      static void update_index(rbtree_node &node, Index index, policy_backmost_t p)
      {
        static_assert(rbtree_node<void, void>
            ::select_argument<Tag, args_list>::found,
            "Tag was not found in this node");

        using node_type = rbtree_node<Index, Type,
              typename rbtree_node<void, void>
                ::select_argument<Tag, args_list>::argument>;

        node_type &ref = node;
        node_type::update_index(ref, std::move(index), p);
      }

      template<typename Tag=void>
      static void update_index(rbtree_node &node, Index index, policy_frontmost_t p)
      {
        static_assert(rbtree_node<void, void>
            ::select_argument<Tag, args_list>::found,
            "Tag was not found in this node");

        using node_type = rbtree_node<Index, Type,
              typename rbtree_node<void, void>
                ::select_argument<Tag, args_list>::argument>;

        node_type &ref = node;
        node_type::update_index(ref, std::move(index), p);
      }

      template<typename Tag=void>
      static void unlink(rbtree_node &node)
      {
        static_assert(rbtree_node<void, void>
            ::select_argument<Tag, args_list>::found,
            "Tag was not found in this node");

        using node_type = rbtree_node<Index, Type,
              typename rbtree_node<void, void>
                ::select_argument<Tag, args_list>::argument>;

        node_type &ref = node;
        node_type::unlink(ref);
      }


      template<typename Tag>
      static bool is_linked(rbtree_node &node)
      {
        using node_type = rbtree_node<Index, Type,
              typename rbtree_node<void, void>
                ::select_argument<Tag, args_list>::argument>;
        node_type &ref = node;
        return node_type::is_linked(ref);
      }

    };


    /** @brief iterator type for rbtree */
    template<typename Index, typename Type, typename Tag, typename Comparator>
    class rbtree_iterator
    {
    public:
      // Nested type similar with STL
      using iterator_category = std::bidirectional_iterator_tag;
      using node_type         =
        rbtree_node<Index, Type,
          rbtree_node<void, void>::args<Tag, Comparator>>;

      using value_type        = Type;
      using reference         = value_type &;
      using pointer           = value_type *;
      using difference_type   = std::ptrdiff_t;

      explicit rbtree_iterator(rbtree_node<void, void> *node) noexcept
        : m_node(static_cast<node_type*>(node))
      { }

      rbtree_iterator(const rbtree_iterator &i) noexcept
        : m_node(i.m_node)
      { }

      rbtree_iterator &operator = (const rbtree_iterator &i) noexcept
      {
        m_node = i.m_node;
        return *this;
      }

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
      {
        node_type *p = static_cast<node_type*>(m_node);
        return static_cast<pointer>(p);
      }

      rbtree_node<void, void> *m_node;
    };

    /** @brief const iterator type for rbtree */
    template<typename Index, typename Type, typename Tag, typename Comparator>
    class rbtree_const_iterator
    {
    public:
      // Nested type similar with STL
      using iterator_category = std::bidirectional_iterator_tag;
      using node_type         =
        const rbtree_node<Index, Type,
          rbtree_node<void, void>::args<Tag, Comparator>>;
      using value_type        = const Type;
      using reference         = value_type &;
      using pointer           = value_type *;
      using difference_type   = std::ptrdiff_t;

      explicit rbtree_const_iterator(const rbtree_node<void, void> *node) noexcept
        : m_node(node)
      { }

      rbtree_const_iterator
        (const rbtree_iterator<Index, Type, Tag, Comparator> &i) noexcept
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

      const rbtree_node<void, void> *m_node;
    };


    /**
     * @brief Intrusive Red-Black-Tree
     * @tparam Index  Representing the type of value which the order of the
     *                is based on
     * @tparam Type   Representing the type which inheriats this rbtree_node
     * @tparam Tag    Tag of the node that this rbtree manipulates, by default
     *                defined as void
     * @tparam Comparator
     *                Comparator for @p Index , by default defined as
     *                spin::less<Index>
     *
     * This class implements intrusive associative container, it maintains
     * references to instances of @p Type in a Red-Black-Tree, and order them
     * by their index and the compare result from @p Comparator.
     *
     * By design, rbtree and its nodes is fully RAII complaince. The tree will
     * remove all elements when it got destroyed.
     *
     * This implementation of Red-Black-Tree is threaded optimized, that is,
     * the nodes may track their predessor and/or successor when they have no
     * left child and/or right child, to improve the iteration performance
     *
     * @see rbtree_node
     *
     */
    template<typename Index, typename Type, typename Tag, typename Comparator>
    class rbtree
    {
    public:
      // Nested type, similar with STL
      using iterator                = rbtree_iterator<Index, Type, Tag, Comparator>;
      using const_iterator          = rbtree_const_iterator<Index, Type, Tag, Comparator>;
      using reverse_iterator        = std::reverse_iterator<iterator>;
      using const_reverse_iterator  = std::reverse_iterator<const_iterator>;
      using node_type               = rbtree_node<Index, Type, rbtree_node<void,void>
                                            ::args<Tag, Comparator>>;
      using value_type              = Type;
      using reference               = value_type &;
      using pointer                 = value_type *;
      using const_pointer           = const pointer;
      using const_reference         = const reference;
      using size_type               = size_t;
      using difference_type         = std::ptrdiff_t;

      /** @brief Default constructor */
      rbtree() noexcept
        : m_container_node(rbtree_node<void, void>::container)
      { }

      /**
       * @brief Construct an rbtree with initial elements
       * @tparam Type the type of the iterator
       */
      template<typename InputIterator>
      rbtree(InputIterator b, InputIterator e) noexcept
        : rbtree()
      { insert(b, e); }

      /** @brief Move constructor */
      rbtree(rbtree &&t) noexcept
        : m_container_node(std::move(t.m_container_node))
      { }

      /** @brief Copy constructor is forbidden */
      rbtree(const rbtree &) = delete;

      /** @brief Move assignment function */
      rbtree &operator = (rbtree &&t) noexcept
      {
        if (&t != this)
        {
          this->~rbtree();
          new (this) rbtree(std::move(t));
        }
        return *this;
      }

      /** @brief Copy assignment is forbidden */
      rbtree &operator = (const rbtree &t) = delete;

      /** @brief Default destructor */
      ~rbtree() noexcept
      { clear(); }


      // Capacity

      /** @brief Test if this rbtree is empty */
      bool empty() const noexcept
      { return m_container_node.is_empty_container_node(); }

      /** @brief Count the elements in this tree */
      size_type size() const noexcept
      {
        size_type s = 0;
        auto b = begin(), e = end();
        while (++b != e) ++s;
        return s;
      }

      // Access

      /**
       * @brief Get an iterator point to the position of the first element in
       *        this tree
       */
      iterator begin() noexcept
      { return iterator(m_container_node.front_of_container()); }

      /**
       * @brief Get an reverse iterator to the position of the last element in
       *         this tree
       */
      reverse_iterator rbegin() noexcept
      { return reverse_iterator(end()); }

      /**
       * @brief Get an const iterator point to the position of the first element
       *        in this tree
       */
      const_iterator begin() const noexcept
      { return const_iterator(end()); }

      /**
       * @brief Get an const reverse iterator to the position of the last
       *        element in this tree
       */
      const_reverse_iterator rbegin() const noexcept
      { return const_reverse_iterator(end()); }

      /**
       * @brief Get an const iterator point to the position of the first
       *        element in this tree
       */
      const_iterator cbegin() const noexcept
      { return const_iterator(m_container_node.front_of_container()); }

      /**
       * @brief Get an const reverse iterator to the position of the last
       *        element in this tree
       */
      const_reverse_iterator crbegin() const noexcept
      { return const_reverse_iterator(end()); }

      /**
       * @brief Get an iterator to the position after the last element
       *        in this tree
       */
      iterator end() noexcept
      { return iterator(&m_container_node); }

      /**
       * @brief Get an reverse iterator point to the position before the
       *        first element in this tree
       */
      reverse_iterator rend() noexcept
      { return reverse_iterator(begin()); }

      /**
       * @brief Get an const iterator point to the position after the last
       *        element in this tree
       */
      const_iterator end() const noexcept
      { return const_iterator(&m_container_node); }

      /**
       * @brief Get an const reverse iterator point to the position before the
       *        first element in this tree
       */
      const_reverse_iterator rend() const noexcept
      { return const_reverse_iterator(begin()); }

      /**
       * @brief Get an const iterator point to the position after the last
       *        element in this tree
       */
      const_iterator cend() const noexcept
      { return const_iterator(&m_container_node); }

      /**
       * @brief Get an const reverse iterator point to the position before the
       *        first element in this tree
       */
      const_reverse_iterator crend() const noexcept
      { return const_reverse_iterator(begin()); }

      /**
       * @brief Get a reference to the first element in this tree
       * @note User should ensure this tree is not empty
       */
      reference front() noexcept
      { return *begin(); }

      /**
       * @brief Get a const reference to the first element in this tree
       * @note User should ensure this tree is not empty
       */
      const_reference front() const noexcept
      { return *begin(); }

      /**
       * @brief Get a reference to the last element in this tree
       * @note User should ensure this tree is not empty
       */
      reference back() noexcept
      { return *rbegin(); }

      /**
       * @brief Get a const reference to the last element in this tree
       * @note User should ensure this tree is not empty
       */
      const_reference back() const noexcept
      { return *rbegin(); }

      /**
       * @brief Find an element its index is equals to @p val, with default
       *        policy of policy_nearest,
       * @param val The value of index of the element want to find
       * @returns
       *        An iterator point to the element with same index value with
       *        @p val, if there are more than one elements have same index,
       *        the one element (first touched) will be return. if not found,
       *        @a end() will be returned.
       */
      iterator find(const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      { return find(end(), val, policy_nearest); }

      /**
       * @brief Find an element its index is equals to @p val
       * @param val The value of index of the element want to find
       * @returns
       *        An iterator point to the element with same index value with
       *        @p val, if there are more than one elements have same index,
       *        the nearest one (first touched) will be return. if not found,
       *        @a end() will be returned.
       */
      iterator find(const Index &val, policy_nearest_t p)
          noexcept(node_type::is_comparator_noexcept)
      { return find(end(), val, p); }

      /**
       * @brief Find an element its index is equals to @p val
       * @param val The value of index of the element want to find
       * @returns
       *        An iterator point to the element with same index value with
       *        @p val, if there are more than one elements have same index,
       *        the frontmost one will be return. if not found, @a end() will
       *        be returned.
       */
      iterator find(const Index &val, policy_frontmost_t p)
          noexcept(node_type::is_comparator_noexcept)
      { return find(end(), val, p); }

      /**
       * @brief Find an element its index is equals to @p val
       * @param val The value of index of the element want to find
       * @returns
       *        An iterator point to the element with same index value with
       *        @p val, if there are more than one elements have same index,
       *        the backmost one will be return. if not found, @a end() will
       *        be returned.
       */
      iterator find(const Index &val, policy_backmost_t p)
          noexcept(node_type::is_comparator_noexcept)
      { return find(end(), val, p); }

      /**
       * @brief Find an element its index is equals to @p val, with default
       *        policy of policy_nearest,
       * @param val  The value of index of the element want to find
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       *
       * @returns
       *        An iterator point to the element with same index value with
       *        @p val, if there are more than one elements have same index,
       *        the nearest one (first touched) will be return. if not found,
       *        @a end() will be returned.
       */
      iterator find(iterator hint, const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      { return find(hint, val, policy_nearest); }

      /**
       * @brief Find an element its index is equals to @p val
       * @param val  The value of index of the element want to find
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       *
       * @returns
       *        An iterator point to the element with same index value with
       *        @p val, if there are more than one elements have same index,
       *        the nearest one (first touched) will be return. if not found,
       *        @a end() will be returned.
       */
      iterator find(iterator hint, const Index &val, policy_nearest_t p)
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        auto *result = node_type::find(ref, val, p);
        if (result == nullptr) return end();
        else return iterator(result);
      }


      /**
       * @brief Find an element its index is equals to @p val
       * @param val The value of index of the element want to find
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       *
       * @returns
       *        An iterator point to the element with same index value with
       *        @p val, if there are more than one elements have same index,
       *        the frontmost one will be return. if not found, @a end() will
       *        be returned.
       */
      iterator find(iterator hint, const Index &val, policy_frontmost_t p)
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        auto *result = node_type::find(ref, val, p);
        if (result == nullptr) return end();
        else return iterator(result);
      }


      /**
       * @brief Find an element its index is equals to @p val
       * @param val The value of index of the element want to find
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       *
       * @returns
       *        An iterator point to the element with same index value with
       *        @p val, if there are more than one elements have same index,
       *        the backmost one will be return. if not found, @a end() will
       *        be returned.
       */
      iterator find(iterator hint, const Index &val, policy_backmost_t p)
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        auto *result = node_type::find(ref, val, p);
        if (result == nullptr) return end();
        else return iterator(result);
      }

      /**
       * @brief Find an element its index is equals to @p val, with default
       *        policy of policy_nearest,
       * @param val The value of index of the element want to find
       * @returns
       *        A const iterator point to the element with same index value
       *        with @p val, if there are more than one elements have same
       *        index, the one element (first touched) will be return. if not
       *        found, @a end() will be returned.
       */
      const_iterator find(const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      { return find(val, policy_nearest); }

      /**
       * @brief Find an element its index is equals to @p val
       * @param val The value of index of the element want to find
       * @returns
       *        A const iterator point to the element with same index value
       *        with @p val, if there are more than one elements have same
       *        index, the one element (first touched) will be return. if not
       *        found, @a end() will be returned.
       */
      const_iterator find(const Index &val, policy_nearest_t p) const
          noexcept(node_type::is_comparator_noexcept)
      { return find(end(), val, p); }

      /**
       * @brief Find an element its index is equals to @p val
       * @param val The value of index of the element want to find
       * @returns
       *        A const iterator point to the element with same index value
       *        with @p val, if there are more than one elements have same
       *        index, the frontmost one will be return. if not found, @a
       *        end() will be returned.
       */
      const_iterator find(const Index &val, policy_frontmost_t p) const
          noexcept(node_type::is_comparator_noexcept)
      { return find(end(), val, p); }

      /**
       * @brief Find an element its index is equals to @p val
       * @param val The value of index of the element want to find
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns
       *        A const iterator point to the element with same index value
       *        with @p val, if there are more than one elements have same
       *        index, the backmost one will be return. if not found, @a
       *        end() will be returned.
       */
      const_iterator find(const Index &val, policy_backmost_t p) const
          noexcept(node_type::is_comparator_noexcept)
      { return find(end(), val, p); }
      /**
       * @brief Find an element its index is equals to @p val
       * @param val The value of index of the element want to find
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns
       *        A const iterator point to the element with same index value
       *        with @p val, if there are more than one elements have same
       *        index, the nearest one (first touched) will be return. if not
       *        found, @a end() will be returned.
       */
      const_iterator find(const_iterator hint, const Index &val, policy_nearest_t p) const
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        auto *result = node_type::find(ref, val, p);
        if (result == nullptr) return end();
        else return const_iterator(result);
      }

      /**
       * @brief Find an element its index is equals to @p val
       * @param val The value of index of the element want to find
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns
       *        A const iterator point to the element with same index value
       *        with @p val, if there are more than one elements have same
       *        index, the frontmost one will be return. if not found, @a
       *        end() will be returned.
       */
      const_iterator find(const_iterator hint, const Index &val, policy_frontmost_t p) const
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        auto *result = node_type::find(ref, val, p);
        if (result == nullptr) return end();
        else return const_iterator(result);
      }

      /**
       * @brief Find an element its index is equals to @p val
       * @param val The value of index of the element want to find
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns
       *        A const iterator point to the element with same index value
       *        with @p val, if there are more than one elements have same
       *        index, the backmost one will be return. if not found, @a
       *        end() will be returned.
       */
      const_iterator find(const_iterator hint, const Index &val, policy_backmost_t p) const
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        auto *result = node_type::find(ref, val, p);
        if (result == nullptr) return end();
        else return const_iterator(result);
      }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value for searching the boundary
       * @returns An iterator point to the first element that is not less than
       *          @p val
       */
      iterator lower_bound(const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      {
        return lower_bound(end(), val);
      }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value for searching the boundary
       * @returns A const iterator point to the first element that is not less
       *          than @p val
       */
      const_iterator lower_bound(const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      { return lower_bound(end(), val); }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value for searching the boundary
       * @returns An iterator point to the first element that is not less than
       *          @p val
       */
      iterator lower_bound(iterator hint, const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        return iterator(node_type::lower_bound(ref, val));
      }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value for searching the boundary
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns A iterator point to the first element that is not less than
       *          @p val
       */
      iterator lower_bound(iterator hint, const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        return const_iterator(node_type::lower_bound(ref,
              node_type::get_val(val)));
      }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value for searching the boundary
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns A const iterator point to the first element that is not less
       *          than @p val
       */
      const_iterator lower_bound(const_iterator hint, const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        return const_iterator(node_type::lower_bound(ref, val));
      }

      /**
       * @brief Find the upper bound for @p val in this tree
       * @param val The value for searching the boundary
       * @returns An iterator point to the first element that is greater than
       *          @p val
       */
      iterator upper_bound(const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      { return upper_bound(end(), val); }

      /**
       * @brief Find the upper bound for @p val in this tree
       * @param val The value for searching the boundary
       * @returns A const iterator point to the first element that is greater than
       *          @p val
       */
      const_iterator upper_bound(const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      { return upper_bound(end(), val); }

      /**
       * @brief Find the upper bound for @p val in this tree
       * @param val The value for searching the boundary
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns An iterator point to the first element that is greater than
       *          @p val
       */
      iterator upper_bound(iterator hint, const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        return iterator(node_type::upper_bound(ref, val));
      }

      /**
       * @brief Find the upper bound for @p val in this tree
       * @param val The value for searching the boundary
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns A const iterator point to the first element that is greater than
       *          @p val
       */
      const_iterator upper_bound(const_iterator hint, const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        return const_iterator(node_type::upper_bound(ref, val));
      }

      /**
       * @brief Returns a range that indexes of elements inside are equals to
       *        @p val
       * @param val The value for searching the equal range
       * @returns A pair of iterator that represents the equal range. The
       *          first iterator is the lower bound, the second iterator
       *          is the upper bound
       */
      std::pair<iterator, iterator> equals_range(const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      { return equals_range(end(), val); }

      /**
       * @brief Returns a range that indexes of elements inside are equals to
       *        @p val
       * @param val The value for searching the equal range
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns A pair of iterator that represents the equal range. The
       *          first iterator is the lower bound, the second iterator
       *          is the upper bound
       */
      std::pair<iterator, iterator> equals_range(iterator hint, const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      {
        auto l = lower_bound(hint, val);
        auto u = lower_bound(l, val);
        return std::make_pair(std::move(l), std::move(u));
      }

      /**
       * @brief Returns a range that indexes of elements inside are equals to
       *        @p val
       * @param val The value for searching the equal range
       * @returns A pair of const iterator that represents the equal range.
       *          The first iterator is the lower bound, the second iterator
       *          is the upper bound
       */
      std::pair<const_iterator, const_iterator>
      equals_range(const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      { return equals_range(end(), val); }

      /**
       * @brief Returns a range that indexes of elements inside are equals to
       *        @p val
       * @param val The value for searching the equal range
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns A pair of const iterator that represents the equal range.
       *          The first iterator is the lower bound, the second iterator
       *          is the upper bound
       */
      std::pair<const_iterator, const_iterator>
      equals_range(const_iterator hint, const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      {
        auto l = lower_bound(hint, val);
        auto u = lower_bound(l, val);
        return std::make_pair(std::move(l), std::move(u));
      }

      // Modifier

      /**
       * @brief Insert an element into this tree, with unique policy
       * @param e The element will be inserted
       * @returns If the element is successfuly inserted into this tree,
       *          the iterator for @p e is returned, otherwise, the
       *          iterator for the element which conflict with this element
       *          is returned
       */
      iterator insert(value_type &e) noexcept(node_type::is_comparator_noexcept)
      { return insert(end(), e, policy_unique); }


      /**
       * @brief Insert an element into this tree
       * @param e The element will be inserted
       * @returns If the element is successfuly inserted into this tree,
       *          the iterator for @p e is returned, otherwise, the
       *          iterator for the element which conflict with this element
       *          is returned
       */
      iterator insert(value_type &e, policy_unique_t p)
          noexcept(node_type::is_comparator_noexcept)
      { return insert(end(), e, p); }

      /**
       * @brief Insert an element into this tree, unlink other elements that is
       *        conflict with this element
       * @param e The element will be inserted
       * @returns Returns an iterator for @p e
       */
      iterator insert(value_type &val, policy_override_t p)
          noexcept(node_type::is_comparator_noexcept)
      { return insert(end(), val, p); }


      /**
       * @brief Insert an element into this tree before any elements that
       *        their index are equals to index of @p e
       * @param e The element will be inserted
       * @returns Returns an iterator for @p e
       */
      iterator insert(value_type &val, policy_frontmost_t p)
          noexcept(node_type::is_comparator_noexcept)
      { return insert(end(), val, p); }

      /**
       * @brief Insert an element into this tree after any elements that
       *        their index are equals to index of @p e
       * @param e The element will be inserted
       * @returns Returns an iterator for @p e
       */
      iterator insert(value_type &val, policy_backmost_t p)
          noexcept(node_type::is_comparator_noexcept)
      { return insert(end(), val, p); }

      /**
       * @brief Insert an element into this tree at the nearest position
       * @param e The element will be inserted
       * @returns Returns an iterator for @p e
       */
      iterator insert(value_type &val, policy_nearest_t p)
          noexcept(node_type::is_comparator_noexcept)
      { return insert(end(), val, p); }

      /**
       * @brief Insert an element into this tree, with unique policy
       * @param e The element will be inserted
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns If the element is successfuly inserted into this tree,
       *          the iterator for @p e is returned, otherwise, the
       *          iterator for the element which conflict with this element
       *          is returned
       */
      iterator insert(iterator hint, value_type &val)
          noexcept(node_type::is_comparator_noexcept)
      { return insert(hint, val, policy_unique); }


      /**
       * @brief Insert an element into this tree
       * @param e The element will be inserted
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns If the element is successfuly inserted into this tree,
       *          the iterator for @p e is returned, otherwise, the
       *          iterator for the element which conflict with this element
       *          is returned
       */
      iterator insert(iterator hint, value_type &val, policy_unique_t p)
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        return iterator(node_type::insert(ref, val, p));
      }

      /**
       * @brief Insert an element into this tree, unlink other elements that is
       *        conflict with this element
       * @param e The element will be inserted
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns Returns an iterator for @p e
       */
      iterator insert(iterator hint, value_type &val, policy_override_t p)
          noexcept(node_type::is_comparator_noexcept)
      { return iterator(node_type::insert(*hint, val, p)); }

      /**
       * @brief Insert an element into this tree before any elements that
       *        their index are equals to index of @p e
       * @param e The element will be inserted
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns Returns an iterator for @p e
       */
      iterator insert(iterator hint, value_type &val, policy_frontmost_t p)
          noexcept(node_type::is_comparator_noexcept)
      { return iterator(node_type::insert(*hint, val, p)); }

      /**
       * @brief Insert an element into this tree after any elements that
       *        their index are equals to index of @p e
       * @param e The element will be inserted
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns Returns an iterator for @p e
       */
      iterator insert(iterator hint, value_type &val, policy_backmost_t p)
          noexcept(node_type::is_comparator_noexcept)
      { return iterator(node_type::insert(*hint, val, p)); }

      /**
       * @brief Insert an element into this tree at the nearest position
       * @param e The element will be inserted
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns Returns an iterator for @p e
       */
      iterator insert(iterator hint, value_type &val, policy_nearest_t p)
          noexcept(node_type::is_comparator_noexcept)
      { return iterator(node_type::insert(*hint, val, p)); }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with unique policy
       * @tparam Type the type of the iterator
       * @param b The begin of the range
       * @param e The end of the range
       */
      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e)
          noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(*b++);
      }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with unique policy
       * @tparam Type the type of the iterator
       * @param b The begin of the range
       * @param e The end of the range
       */
      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, policy_unique_t p)
          noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(*b++, p);
      }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with override policy
       * @tparam Type the type of the iterator
       * @param b The begin of the range
       * @param e The end of the range
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       */
      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, policy_override_t p)
          noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(*b++, p);
      }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with frontmost policy
       * @tparam Type the type of the iterator
       * @param b The begin of the range
       * @param e The end of the range
       */
      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e,
          policy_frontmost_t p) noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(*b++, p);
      }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with backmost policy
       * @tparam Type the type of the iterator
       * @param b The begin of the range
       * @param e The end of the range
       */
      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, policy_backmost_t p)
          noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(*b++, p);
      }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with nearest policy
       * @tparam Type the type of the iterator
       * @param b The begin of the range
       * @param e The end of the range
       */
      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, policy_nearest_t p)
          noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(*b++, p);
      }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with unique policy
       * @tparam Type the type of the iterator
       * @param b The begin of the range
       * @param e The end of the range
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       */
      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint)
          noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(hint, *b++);
      }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with unique policy
       * @tparam Type the type of the iterator
       * @param b The begin of the range
       * @param e The end of the range
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       */
      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          policy_unique_t p) noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with override policy
       * @tparam Type the type of the iterator
       * @param b The begin of the range
       * @param e The end of the range
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       */
      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          policy_override_t p) noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with frontmost policy
       * @tparam Type the type of the iterator
       * @param b The begin of the range
       * @param e The end of the range
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       */
      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          policy_frontmost_t p) noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with backmost policy
       * @tparam Type the type of the iterator
       * @param b The begin of the range
       * @param e The end of the range
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       */
      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          policy_backmost_t p) noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with nearest policy
       * @tparam Type the type of the iterator
       * @param b The begin of the range
       * @param e The end of the range
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       */
      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e, iterator hint,
          policy_nearest_t p) noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      /** @brief Remote an element that the iterator point to from this tree */
      void erase(iterator iter) noexcept
      {
        node_type &ref = *iter;
        node_type::unlink(ref);
      }

      /**
       * @brief Remote all element inside the range of [@p b, @p e) from
       *        this tree
       */
      void erase(iterator b, iterator e) noexcept
      {
        for (auto i = b; i != e; )
          erase(i++);
      }


      /**
       * @brief Remove all elements that accept by @p predicate from
       *        this tree
       * @param predicate Functor that test whether an element should be
       *        removed
       */
      template<typename Predicate>
      typename std::enable_if<!std::is_same<Index, Predicate>::value, void>::type
      remove(Predicate &&predicate)
          noexcept(noexcept(predicate(std::declval<Index>())))
      {
        auto b = begin(), e = end();
        while (b != e)
        {
          auto x = b++;
          if (predicate(get_index(*x)))
            erase(x);
        }
      }

      /**
       * @brief Remove all elements that their index are equals to @p val from
       *        this tree
       * @param val The value of index for searching elements
       */
      void remove(const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      {
        auto b = lower_bound(end(), val);
        auto e = upper_bound(b, val);
        erase(b, e);
      }

      /** @brief Swap all elements with another tree @p t */
      void swap(rbtree &&t) noexcept
      { swap(t); }

      /** @brief Swap all elements with another tree @p t */
      void swap(rbtree &t) noexcept
      { node_type::swap_nodes(m_container_node, t.m_container_node); }

      /** @brief Remove all the elements in this tree */
      void clear() noexcept
        // Can be optimize
      { erase(begin(), end()); }

      /** @brief Return a reference to the index comparator */
      static const Comparator &index_comparator() noexcept
      { return node_type::cmper; }


    private:
      node_type m_container_node;
    };

    template<typename Index, typename Type, typename Tag, typename Comparator>
    bool operator == (const rbtree<Index, Type, Tag, Comparator> &x,
        const rbtree<Index, Type, Tag, Comparator> &y) noexcept
    {
      auto i = x.begin(), j = y.begin();
      auto m = x.end(), n = y.end();

      while (i != m && j != n && *i == *j)
      { ++i; ++j; }

      return i == m && j == n;
    }

    template<typename Index, typename Type, typename Tag, typename Comparator>
    bool operator != (const rbtree<Index, Type, Tag, Comparator> &x,
        const rbtree<Index, Type, Tag, Comparator> &y) noexcept
    { return !(x == y); }

    template<typename Index, typename Type, typename Tag, typename Comparator>
    bool operator < (const rbtree<Index, Type, Tag, Comparator> &x,
        const rbtree<Index, Type, Tag, Comparator> &y) noexcept
    {
      if (&x == &y) return false;
      return std::lexicographical_compare(x.begin(), x.end(),
          y.begin(), y.end());
    }

    template<typename Index, typename Type, typename Tag, typename Comparator>
    bool operator <= (const rbtree<Index, Type, Tag, Comparator> &x,
        const rbtree<Index, Type, Tag, Comparator> &y) noexcept
    { return !(y < x); }

    template<typename Index, typename Type, typename Tag, typename Comparator>
    bool operator > (const rbtree<Index, Type, Tag, Comparator> &x,
        const rbtree<Index, Type, Tag, Comparator> &y) noexcept
    { return y < x; }

    template<typename Index, typename Type, typename Tag, typename Comparator>
    bool operator >= (const rbtree<Index, Type, Tag, Comparator> &x,
        const rbtree<Index, Type, Tag, Comparator> &y) noexcept
    { return !(y > x); }

  }
}

#endif
