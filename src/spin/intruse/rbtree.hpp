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

#include <iterator>
#include <type_traits>
#include <cassert>

namespace spin
{
  namespace intruse
  {

    template<typename Key, typename Comparer = std::less<Key>, typename Tag = void>
    class rbtree;

    template<typename Key, typename Comparer = std::less<Key>, typename Tag = void>
    class rbtree_node;

    template<typename Key, typename Comparer, typename Tag>
    class rbtree_iterator;

    template<typename Key, typename Comparer, typename Tag>
    class rbtree_const_iterator;

    template<>
    class __SPIN_EXPORT__ rbtree_node<void, void>
    {
      template<typename Key, typename Comparer, typename Tag>
      friend class rbtree;

      template<typename Key, typename Comparer, typename Tag>
      friend class rbtree_node;

      template<typename Key, typename Comparer, typename Tag>
      friend class rbtree_iterator;
    public:

    protected:

      /** @brief Default constructor */
      rbtree_node() noexcept
        : m_p(nullptr)
        , m_l(nullptr)
        , m_r(nullptr)
        , m_has_l(false)
        , m_has_r(false)
        , m_is_red(false)
        , m_is_container(false)
      { }

      /** @brief Destructor */
      ~rbtree_node() noexcept
      { unlink_checked(); }

      /** @brief Move constructor */
      rbtree_node(rbtree_node &&n) noexcept
        : rbtree_node()
      { swap_nodes(*this, n); }

      /** @brief Assign operator overload for rvalue */
      rbtree_node &operator = (rbtree_node &&n) noexcept
      {
        this->~rbtree_node();
        new (this) rbtree_node(std::move(n));
        return *this;
      }

      rbtree_node(const rbtree_node &) = delete;

      rbtree_node &operator = (const rbtree_node &n) = delete;


      // Status

      /** @brief Test if a node is container node or root node, should be
       * faster than is_container_node and is_root_node */
      bool is_container_or_root() const noexcept
      { return m_p != nullptr && m_p->m_p == this; }

      /** @brief Test if a node is container node */
      bool is_container_node() const noexcept
      { return is_container_or_root() && this->m_is_container; }

      /** @brief Test if a node is the root node of rbtree */
      bool is_root_node() const noexcept
      { return is_container_or_root() && !this->m_is_container; }

      /** @brief Test if container is empty */
      bool is_container_empty() const noexcept
      { return m_p == this; }

      /** @brief Test if a node is linked into rbtree */
      bool is_linked() const noexcept
      {
        bool result = m_p != nullptr;

        // Check if corrupt
        assert (result == (m_l != nullptr));
        assert (result == (m_r != nullptr));
        return result;
      }

      // Navigation

      /**
       * @brief Get root node from container node
       * @note User is responsible to ensure this node is container node
       * @returns the root node of this rbtree, or nullptr if this tree is an
       * empty tree
       */
      const rbtree_node *get_root_node_from_container_node() const noexcept
      {
        assert(is_container_node());
        if (m_p == this)
          return nullptr;
        else
          return m_p;
      }

      /**
       * @brief Get root node from container node
       * @note User is responsible to ensure this node is container node
       * @returns the root node of this rbtree, or nullptr if this tree is an
       * empty tree
       */
      rbtree_node *get_root_node_from_container_node() noexcept
      {
        assert(is_container_node());
        if (m_p == this)
          return nullptr;
        else
          return m_p;
      }

      /** @brief Get root node */
      rbtree_node *get_root_node() noexcept
      {
        assert (!is_container_node());
        auto *p = this;
        while (p->m_p->m_p != this)
        {
          if (p->m_p->m_p == p)
            return nullptr;
          else
            p = p->m_p;
        }
        return p;
      }

      /** @brief Get root node */
      const rbtree_node *get_root_node() const noexcept
      {
        assert (!is_container_node());
        auto *p = this;
        while (p->m_p->m_p != this)
          p = p->m_p;
        return p;
      }

      /**
       * @brief Return the back node of container via container node
       * @note If this node is not container node, the result is undefined
       */
      rbtree_node *back_of_container() noexcept
      {
        assert (is_container_node());
        return m_r;
      }

      /**
       * @brief Return the back node of container via container node
       * @note If this node is not container node, the result is undefined
       */
      const rbtree_node *back_of_container() const noexcept
      {
        assert (is_container_node());
        return m_r;
      }

      /**
       * @brief Return the front node of container via container node
       * @note If this node is not container node, the result is undefined
       */
      rbtree_node *front_of_container() noexcept
      {
        assert (is_container_node());
        return m_l;
      }

      /**
       * @brief Return the front node of container via container node
       * @note If this node is not container node, the result is undefined
       */
      const rbtree_node *front_of_container() const noexcept
      {
        assert (is_container_node());
        return m_l;
      }

      /**
       * @brief Get successor of this node
       * @note if this node is container node, the result is undefined
       */
      rbtree_node *next() noexcept
      {
        if (!is_container_node())
        {
          if (this->m_has_r)
            return m_r->front();
          else
            return this->m_r;
        }
        else
          return this->m_l;
      }

      /**
       * @brief Get successor of this node
       * @note if this node is container node, the result is undefined
       */
      const rbtree_node *next() const noexcept
      {
        if (!is_container_node())
        {
          if (this->m_has_r)
            return m_r->front();
          else
            return this->m_r;
        }
        else
          return this->m_l;
      }

      /**
       * @brief Get predecessor of this node
       * @note if this node is container node, the result is undefined
       */
      rbtree_node *prev() noexcept
      {
        if (!is_container_node())
        {
          if (this->m_has_l)
            return m_l->back();
          else
            return this->m_l;
        }
        else
          return this->m_r;
      }

      /**
       * @brief Get predecesssor of this node
       * @note if this node is container node, the result is undefined
       */
      const rbtree_node *prev() const noexcept
      {
        if (!is_container_node())
        {
          if (this->m_has_l)
            return m_l->back();
          else
            return this->m_l;
        }
        else
          return this->m_r;
      }

      /** @brief Get front node of this subtree */
      rbtree_node *front() noexcept
      {
        auto *p = this;
        while (p->m_has_l) p = p->m_l;
        return p;
      }

      /** @brief Get back node of this subtree */
      rbtree_node *back() noexcept
      {
        auto *p = this;
        while (p->m_has_r) p = p->m_r;
        return p;
      }

      /** @brief Get front node of this subtree */
      const rbtree_node *front() const noexcept
      {
        auto *p = this;
        while (p->m_has_l) p = p->m_l;
        return p;
      }

      /** @brief Get back node of this subtree */
      const rbtree_node *back() const noexcept
      {
        auto *p = this;
        while (p->m_has_r) p = p->m_r;
        return p;
      }


      /** @brief Get sibling node */
      rbtree_node *get_sibling_node() noexcept
      {
        if (m_p->is_container_node())
          return nullptr;

        if (this == m_p->m_l)
          if (m_p->m_has_r)
            return m_p->m_r;
          else
            return nullptr;
        else
          if (m_p->m_has_l)
            return m_p->m_l;
          else
            return nullptr;
      }

      // Rotation

      /** @brief Do left rotate */
      void lrotate() noexcept
      {
        assert(m_has_r);
        assert(!is_container_node());
        auto *y = m_r;
        if (y->m_has_l)
        {
          m_r = y->m_l;
          m_r->m_p = this;
        }
        else
        {
          m_has_r = false;
          m_r = y;
          y->m_has_l = true;
        }

        y->m_p = m_p;
        if (m_p->m_p == this)
          m_p->m_p = y;
        else if (m_p->m_l == this)
          m_p->m_l = y;
        else
          m_p->m_r = y;

        y->m_l = this;
        m_p = y;
      }

      /** @brief Do right rotate */
      void rrotate() noexcept
      {
        assert(m_has_l);
        assert(!is_container_node());
        auto *y = m_l;
        if (y->m_has_r)
        {
          m_l = y->m_r;
          m_l->m_p = this;
        }
        else
        {
          m_has_l = false;
          m_l = y;
          y->m_has_r = true;
        }

        y->m_p = m_p;
        if (m_p->m_p == this)
          m_p->m_p = y;
        else if (m_p->m_r == this)
          m_p->m_r = y;
        else
          m_p->m_l = y;

        y->m_r = this;
        m_p = y;
      }

      // Insertion

      /**
       * @brief Insert a node as left child of this node
       * @note User is responsible for ensure this node does not have left
       * child
       */
      void insert_to_left(rbtree_node *node) noexcept
      {
        assert(m_has_l == false);
        node->m_l = m_l;
        if (m_l->is_container_node())
          m_l->m_l = node;
        node->m_r = this;
        node->m_p = this;
        m_l = node;
        m_has_l = true;
        node->m_is_red = true;
        rebalance_for_insertion(node);
      }

      /**
       * @brief Insert a node as right child of this node
       * @note User is responsible for ensure this node does not have right
       * child
       */
      void insert_to_right(rbtree_node *node) noexcept
      {
        assert(m_has_r == false);
        node->m_r = m_r;
        if (m_r->is_container_node())
          m_r->m_r = node;
        node->m_l = this;
        node->m_p = this;
        m_r = node;
        m_has_r = true;
        node->m_is_red = true;
        rebalance_for_insertion(node);
      }

      /**
       * @brief Insert a node as root node
       * @note User is responible to ensure this node is container node
       */
      void insert_root_node(rbtree_node *node) noexcept
      {
        assert(is_container_node());
        assert(!node->m_is_red);
        m_p = m_l = m_r = node;
        node->m_p = node->m_l = node->m_r = this;
      }

      /** @brief Insert a node as predecessor of this node */
      void insert_before (rbtree_node *node) noexcept
      {
        node->unlink_checked();

        if (is_container_node())
          insert_root_node(node);
        else if (m_has_l)
          prev()->insert_to_left(node);
        else
          insert_to_left(node);
      }

      /** @brief Insert a node as successor of this node */
      void insert_after(rbtree_node *node) noexcept
      {
        node->unlink_checked();

        if (is_container_node())
          insert_root_node(node);
        else if (this->m_has_r)
          next()->insert_to_right(node);
        else
          insert_to_right(node);
      }


      /** @breif Do clean up work after unlink */
      void unlink_cleanup() noexcept
      {
        // We can do our best to check corrupt here
        assert (m_l && m_l->m_p != this);
        assert (m_r && m_r->m_p != this);
        assert (m_p && m_p->m_p != this);
        assert (m_p && m_p->m_l != this);
        assert (m_p && m_p->m_r != this);

        m_l = m_r = m_p = nullptr;
        m_is_red = m_has_l = m_has_r = false;
      }

      /**
       * @brief Unlink this node from the rbtree
       * @returns the successor of this node, or container node if this node
       * is the last node in the tree
       * @note User is responsible to ensure this node is linked and is not
       * container node,
       * @see use unlink_checked
       */
      rbtree_node *unlink() noexcept
      {
        assert (is_linked());
        assert (!is_container_node());

        rbtree_node *y = next(), *x;

        if (m_has_l && m_has_r)
          swap_nodes(*this, *y);

        if (m_has_l)
          x = m_l;
        else if (m_has_r)
          x = m_r;
        else
          x = this;

        x->m_p = m_p;
        if (m_p->is_container_node())
        {
          if (this == x)
          {
            m_p->m_p = m_p;
            m_p->m_l = m_p;
            m_p->m_r = m_p;
          }
          else
          {
            m_p->m_p = x;
            m_p->m_l = x->front();
            m_p->m_r = x->back();
            if (x == m_l)
              x->back()->m_r = m_p;
            else
              x->front()->m_l = m_p;
          }
        }
        else if (this == m_p->m_l)
        {
          if (this == x)
          {
            m_p->m_l = m_l;
            m_p->m_has_l = false;
            if (m_l->is_container_node())
              m_l->m_l = m_p;
          }
          else
          {
            m_p->m_l = x;
            if (x == this->m_r)
            {
              if (m_l->is_container_node())
              {
                m_l->m_l = x->front();
                m_l->m_l->m_l = m_l;
              }
              else
                x->front()->m_l = m_l;
            }
            else
              x->back()->m_r = m_p;
          }
        }
        else
        {
          if (this == x)
          {
            m_p->m_r = m_r;
            m_p->m_has_r = false;
            if (m_r->is_container_node())
              m_r->m_r = m_p;
          }
          else
          {
            m_p->m_r = x;
            if (x == this->m_l)
            {
              if (m_r->is_container_node())
              {
                m_r->m_r = x->back();
                m_r->m_r->m_r = m_r;
              }
              else
                x->back()->m_r = m_r;
            }
            else
              x->front()->m_l = m_p;
          }
        }

        bool need_rebalance = !m_is_red;

        if (need_rebalance)
          rebalance_for_unlink(x);
        unlink_cleanup();
        return y;
      }

      /**
       * @breif Unlink this node from the tree if this node is already linked
       * @returns the successor of this node, or nullptr if this node is not
       * linked into a rbtree
       * @note However it doesn't not check if this node is container node of
       * a rbtree, and unlink a container node is always cause undefined
       * behaviours
       */
      rbtree_node *unlink_checked() noexcept
      {
        if (is_container_node())
          return nullptr;
        if (is_linked())
          return unlink();
        else
          return nullptr;
      }

    private:

      /** @brief Auxilary class used for rbtree(container_tag) */
      static struct container_tag {} const container;

      /** Initialize this node as container node */
      rbtree_node(container_tag) noexcept
        : m_p(this)
        , m_l(this)
        , m_r(this)
        , m_has_l(false)
        , m_has_r(false)
        , m_is_red(true)
        , m_is_container(true)
      { }

      /** @brief Rebalance a node after insertion */
      static void rebalance_for_insertion(rbtree_node *node) noexcept
      {
        while(node->m_p->m_is_red && !node->is_container_or_root())
          // Check node is not root of node and its parent are red
        {
          auto *parent = node->m_p;

          assert (!parent->is_root_node());

          if (parent == parent->m_p->m_l)
          {
            if (parent->m_p->m_has_r && parent->m_p->m_r->m_is_red)
            {
              auto *y = parent->m_p->m_r;
              parent->m_is_red = false;

              y->m_is_red = false;
              parent->m_p->m_is_red = true;
              node = parent->m_p;
              parent = node->m_p;
            }
            else
            {
              if (parent->m_r == node)
              {
                node = parent;
                node->lrotate();
                parent = node->m_p;
              }

              parent->m_p->rrotate();
              parent->m_is_red = false;
              parent->m_r->m_is_red = true;
            }
          }
          else if (parent == parent->m_p->m_r)
          {
            if (parent->m_p->m_has_l && parent->m_p->m_l->m_is_red)
            {
              auto *y = parent->m_p->m_l;
              parent->m_is_red = false;
              y->m_is_red = false;
              parent->m_p->m_is_red = true;
              node = parent->m_p;
              parent = node->m_p;
            }
            else
            {
              if (parent->m_l == node)
              {
                node = parent;
                node->rrotate();
                parent = node->m_p;
              }

              parent->m_p->lrotate();
              parent->m_is_red = false;
              parent->m_l->m_is_red = true;

            }
          }
          else
            assert (parent->is_container_or_root());
        }

        if (node->is_container_or_root())
          node->m_is_red = false;
      }

      /** @brief Rebalance the tree after unlink */
      static void rebalance_for_unlink(rbtree_node *node) noexcept
      {
        // Be careful that node may have detached from the tree
        while (node->m_is_red == false && !node->is_container_or_root())
        {
          auto *parent = node;
          assert(!parent->is_root_node());

          if (parent->m_l == node)
          {
            assert (parent->m_has_r);

            auto *w = parent->m_r;
            if (w->m_is_red)
              // case 1:
            {
              // as node is black but w is red, the following assertion must
              // satisfied
              assert (w->m_has_l);
              assert (w->m_has_r);
              parent->lrotate();
              parent->m_is_red = true;
              parent->m_p->m_is_red = false;
              w = parent->m_r;
            }


            if ((!w->m_has_l || w->m_l->m_is_red == false)
                && (!w->m_has_r || w->m_r->m_is_red == false))
            // case 2:
            {
              w->m_is_red = true;
              node = parent;
              parent = node->m_p;
            }
            else
            {
              if (!w->m_has_r || w->m_r->m_is_red == false)
                // case 3:
              {
                assert(w->m_has_l);
                //w->m_is_red = true;
                //w->m_l->m_is_red = false;

                w->rrotate();
                w->m_p->m_is_red = false;
                w->m_is_red = true;
                w = parent->m_r;
              }

              // case 4:

              assert(w->m_has_r);
              w->m_is_red = parent->m_is_red;
              //parent->m_is_red = false;
              //w->m_r->m_is_red = false;
              parent->lrotate();
              parent->m_is_red = false;
              w->m_r->m_is_red = false;
              break;
            }
          }
          else if (parent->m_r == node)
          {
            assert (parent->m_has_l);

            auto *w = parent->m_l;
            if (w->m_is_red)
              // case 1:
            {
              // as node is black but w is red, the following assertion must
              // satisfied
              assert (w->m_has_l);
              assert (w->m_has_r);
              //parent->m_l->m_is_red = false;
              //parent->m_is_red = true;
              parent->rrotate();
              parent->m_is_red = true;
              parent->m_p->m_is_red = false;
              w = parent->m_l;
            }


            if ((!w->m_has_l || w->m_l->m_is_red == false)
                && (!w->m_has_r || w->m_r->m_is_red == false))
            // case 2:
            {
              w->m_is_red = true;
              node = parent;
              parent = node->m_p;
            }
            else
            {
              if (!w->m_has_l || w->m_l->m_is_red == false)
                // case 3:
              {
                assert(w->m_has_r);
                //w->m_is_red = true;
                //w->m_r->m_is_red = false;

                w->lrotate();
                w->m_is_red = true;
                w->m_p->m_is_red = false;
                w = parent->m_l;
              }

              // case 4:

              assert(w->m_has_l);
              w->m_is_red = parent->m_is_red;
              parent->rrotate();
              parent->m_is_red = false;
              w->m_l->m_is_red = false;
              break;
            }
          }
          else
            break;
        }

        if (node->is_container_node())
          return;
        else if (!node->is_root_node())
          node = node->get_root_node();
        if (node)
          node->m_is_red = false;
        //if (node->is_root_node())
        //  node->m_is_red = false;
      }


      void transfer_link(rbtree_node &node) noexcept
      {
        assert(!node.m_is_container);
        assert(!m_is_container);
        assert(!node.is_linked());

        if (is_linked())
        {
          node.m_p = m_p;
          node.m_l = m_l;
          node.m_r = m_r;
          node.m_has_l = m_has_l;
          node.m_has_r = m_has_r;
          node.m_is_red = m_is_red;

          if (m_p->m_is_container)
            m_p->m_p = &node;
          else if (this == m_p->m_l)
            m_p->m_l = &node;
          else
            m_p->m_r = &node;

          if (m_has_l)
          {
            m_l->m_p = &node;
            m_l->back()->m_r = &node;
          }
          else if (m_l->m_is_container)
            m_l->m_l = &node;

          if (m_has_r)
          {
            m_r->m_p = &node;
            m_r->front()->m_l = &node;
          }
          else if (m_r->m_is_container)
            m_r->m_r = &node;

          unlink_cleanup();
        }
      }

      /**
       * @brief Swap two nodes in the tree
       * @note This will generally break the order or nodes, so it's declared
       * privately
       */
      static void swap_nodes(rbtree_node<void, void> &lhs, rbtree_node<void, void> &rhs) noexcept
      {
        rbtree_node tmp;
        lhs.transfer_link(tmp);
        rhs.transfer_link(lhs);
        tmp.transfer_link(rhs);
      }

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
          insert_hinted(p, *this);
        return key;
      }

      static void unlink(rbtree_node &node) noexcept
      { node.rbtree_node<void, void>::unlink(); }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param hint_node The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note Use is responsible to ensure hint_node is already attached to a
       * tree; and if duplicate is permitted, the node is insert before any
       * node duplicate with this node
       */
      static rbtree_node *insert_after(rbtree_node<void, void> &hint_node,
          rbtree_node &node, bool duplicate = false)
      {
        auto *pos = &hint_node;
        if (pos->is_container_node())
        {
          if (pos->is_container_empty())
          {
            pos->insert_after(&node);
            return &node;
          }
          else
            pos = pos->get_root_node_from_container_node();
        }

        // FIXME: Visiting
        while (!pos->is_root_node())
        {
          auto *p = internal_cast(pos);
          if (cmper(p->get_key(), node.get_key()))
          {
            if (p->m_p->m_l == p && p->m_p->m_has_l)
              pos = p->m_p;
            else
            {
              if (p->m_has_r)
              {
                pos = p->m_r;
                break;
              }
              else
              {
                pos->insert_after(&node);
                return &node;
              }
            }
          }
          else if (cmper(node.get_key(), p->get_key()))
          {
            if (p->m_p->m_r == p && p->m_p->m_has_r)
              pos = p->m_p;
            else
            {
              if (p->m_has_l)
              {
                pos = p->m_l;
                break;
              }
              else
              {
                pos->insert_before(&node);
                return &node;
              }
            }
          }
          else if (duplicate)
          {
            pos->insert_after(&node);
            return &node;
          }
          else
            return p;
        }

        for ( ; ; )
        {
          auto *p = internal_cast(pos);
          if (cmper(p->get_key(), node.get_key()))
          {
            if (p->m_has_r)
            {
              pos = p->m_r;
              continue;
            }
            else
            {
              p->insert_after(&node);
              return &node;
            }
          }
          else if (cmper(node.get_key(), p->get_key()))
          {
            if (p->m_has_l)
            {
              pos = p->m_l;
              continue;
            }
            else
            {
              p->insert_before(&node);
              return &node;
            }
          }
          else if (duplicate)
          {
            pos->insert_after(&node);
            return &node;
          }
        }
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
      static rbtree_node *insert_before(rbtree_node<void, void> &hint_node,
          rbtree_node &node, bool duplicate = false)
      {
        auto *pos = &hint_node;
        if (pos->is_container_node())
        {
          if (pos->is_container_empty())
          {
            pos->insert_after(&node);
            return &node;
          }
          else
            pos = pos->get_root_node_from_container_node();
        }

        // FIXME: Visiting
        while (!pos->is_root_node())
        {
          auto *p = internal_cast(pos);
          if (cmper(p->get_key(), node.get_key()))
          {
            if (p->m_p->m_l == p && p->m_p->m_has_l)
              pos = p->m_p;
            else
            {
              if (p->m_has_r)
              {
                pos = p->m_r;
                break;
              }
              else
              {
                pos->insert_after(&node);
                return &node;
              }
            }
          }
          else if (cmper(node.get_key(), p->get_key()))
          {
            if (p->m_p->m_r == p && p->m_p->m_has_r)
              pos = p->m_p;
            else
            {
              if (p->m_has_l)
              {
                pos = p->m_l;
                break;
              }
              else
              {
                pos->insert_before(&node);
                return &node;
              }
            }
          }
          else if (duplicate)
          {
            pos->insert_before(&node);
            return &node;
          }
          else
            return p;
        }

        for ( ; ; )
        {
          auto *p = internal_cast(pos);
          if (cmper(p->get_key(), node.get_key()))
          {
            if (p->m_has_r)
            {
              pos = p->m_r;
              continue;
            }
            else
            {
              pos->insert_after(&node);
              return &node;
            }
          }
          else if (cmper(node.get_key(), p->get_key()))
          {
            if (p->m_has_l)
            {
              pos = p->m_l;
              continue;
            }
            else
            {
              pos->insert_before(&node);
              return &node;
            }
          }
          else if (duplicate)
          {
            pos->insert_before(&node);
            return &node;
          }
        }
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

      friend bool operator != (const rbtree_iterator &l, const rbtree_iterator &r) noexcept
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
        : m_container_node(rbtree_node<void, void>::container)
      { }

      template<typename InputIterator>
      rbtree(InputIterator b, InputIterator e) noexcept;

      ~rbtree() noexcept
      { clear(); }


      // Capacity
      bool empty() const noexcept;

      size_type size() const noexcept;

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

      iterator lowwer_bound(const Key &key) noexcept;

      const_iterator lowwer_bound(const Key &key) const noexcept;

      iterator lowwer_bound(const_iterator hint, const Key &key) noexcept;

      const_iterator lowwer_bound(const_iterator hint, const Key &key) const noexcept;

      iterator upper_bound(const Key &key) noexcept;

      const_iterator upper_bound(const Key &key) const noexcept;

      iterator upper_bound(const_iterator hint, const Key &key) noexcept;

      const_iterator upper_bound(const_iterator hint, const Key &key) const noexcept;

      // Modifier
      std::pair<iterator, bool> insert(value_type &val) noexcept
      {
        return insert(end(), val);
      }

      std::pair<iterator, bool> insert(iterator hint, value_type &val) noexcept
      {
        auto *p = node_type::insert_before(*hint, val);
        return std::make_pair(iterator(p), p == &val);
      }

      template<typename InputIterator>
      void insert(InputIterator b, InputIterator e) noexcept;

      void erase(iterator iter) noexcept
      { iter->unlink(); }

      void erase(iterator b, iterator e) noexcept
      {
        for (auto i = b; i != e; ++i)
          node_type::unlink(*i++);
      }

      void remove(Key &key) noexcept(noexcept(key == key));

      template<typename Predicate>
      void remove(Predicate &&predicate) noexcept(noexcept(pred(std::declval<Key>())));

      void swap(rbtree &&t) noexcept
      { swap(t); }

      void swap(rbtree &t) noexcept;

      void clear() noexcept
      {
        erase(begin(), end());
      }

      const Comparer &key_comp() const noexcept;
    private:
      rbtree_node<void, void> m_container_node;
    };
  }
}

#endif
