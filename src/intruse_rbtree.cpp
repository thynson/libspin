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

#include <spin/intruse/rbtree.hpp>

#include <cassert>

namespace spin
{
  namespace intruse
  {
    rbtree_node<void, void>:: rbtree_node() noexcept
      : m_p(nullptr)
      , m_l(nullptr)
      , m_r(nullptr)
      , m_has_l(false)
      , m_has_r(false)
      , m_is_red(false)
      , m_is_container(false)
    { }

    rbtree_node<void, void>::
      rbtree_node(rbtree_node<void, void>::container_tag) noexcept
      : m_p(this)
      , m_l(this)
      , m_r(this)
      , m_has_l(false)
      , m_has_r(false)
      , m_is_red(true)
      , m_is_container(true)
    { }

    rbtree_node<void, void>::~rbtree_node() noexcept
    { unlink_checked(); }


    rbtree_node<void, void>::rbtree_node(rbtree_node &&n) noexcept
      : rbtree_node()
    { n.transfer_link(*this); }

    rbtree_node<void, void> &
      rbtree_node<void, void>::operator = (rbtree_node &&n) noexcept
    {
      this->~rbtree_node();
      new (this) rbtree_node(std::move(n));
      return *this;
    }

    bool rbtree_node<void, void>::is_linked() const noexcept
    {
      bool result = m_p != nullptr;

      // Check if corrupt
      assert (result == (m_l != nullptr));
      assert (result == (m_r != nullptr));
      return result;
    }

    const rbtree_node<void, void> *
      rbtree_node<void, void>::get_root_node_from_container_node()
      const noexcept
    {
      assert(is_container_node());
      if (m_p == this)
        return nullptr;
      else
        return m_p;
    }

    rbtree_node<void, void> *
      rbtree_node<void, void>::get_root_node_from_container_node() noexcept
    {
      assert(is_container_node());
      if (m_p == this)
        return nullptr;
      else
        return m_p;
    }

    rbtree_node<void, void> *
      rbtree_node<void, void>::get_root_node() noexcept
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

    const rbtree_node<void, void> *
      rbtree_node<void, void>::get_root_node() const noexcept
    {
      assert (!is_container_node());
      auto *p = this;
      while (p->m_p->m_p != this)
        p = p->m_p;
      return p;
    }

    rbtree_node<void, void> *
      rbtree_node<void, void>::back_of_container() noexcept
    {
      assert (is_container_node());
      return m_r;
    }

    const rbtree_node<void, void> *
      rbtree_node<void, void>::back_of_container() const noexcept
    {
      assert (is_container_node());
      return m_r;
    }

    const rbtree_node<void, void> *
      rbtree_node<void, void>::front_of_container() const noexcept
    {
      assert (is_container_node());
      return m_l;
    }

    rbtree_node<void, void> *
      rbtree_node<void, void>::front_of_container() noexcept
    {
      assert (is_container_node());
      return m_l;
    }

    rbtree_node<void, void> *
      rbtree_node<void, void>::next() noexcept
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

    const rbtree_node<void, void> *
      rbtree_node<void, void>::next() const noexcept
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


    const rbtree_node<void, void> *
      rbtree_node<void, void>::prev() const noexcept
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

    rbtree_node<void, void> *
      rbtree_node<void, void>::prev() noexcept
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

    rbtree_node<void, void> *
      rbtree_node<void, void>::front() noexcept
    {
      auto *p = this;
      while (p->m_has_l) p = p->m_l;
      return p;
    }

    const rbtree_node<void, void> *
      rbtree_node<void, void>::front() const noexcept
    {
      auto *p = this;
      while (p->m_has_l) p = p->m_l;
      return p;
    }

    rbtree_node<void, void> *
      rbtree_node<void, void>::back() noexcept
    {
      auto *p = this;
      while (p->m_has_r) p = p->m_r;
      return p;
    }

    const rbtree_node<void, void> *
      rbtree_node<void, void>::back() const noexcept
    {
      auto *p = this;
      while (p->m_has_r) p = p->m_r;
      return p;
    }

    void rbtree_node<void, void>::lrotate() noexcept
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

    void rbtree_node<void, void>::rrotate() noexcept
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

    void rbtree_node<void, void>::
      insert_to_left(rbtree_node<void, void> *node) noexcept
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

    void rbtree_node<void, void>::
      insert_to_right(rbtree_node<void, void> *node) noexcept
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

    void rbtree_node<void, void>::
      insert_root_node(rbtree_node<void, void> *node) noexcept
    {
      assert(is_container_node());
      assert(!node->m_is_red);
      m_p = m_l = m_r = node;
      node->m_p = node->m_l = node->m_r = this;
    }

    void rbtree_node<void, void>::
      insert_before(rbtree_node<void, void> *node) noexcept
    {
      node->unlink_checked();

      if (is_container_node())
        insert_root_node(node);
      else if (m_has_l)
        prev()->insert_to_left(node);
      else
        insert_to_left(node);
    }

    void rbtree_node<void, void>::
      insert_after(rbtree_node *node) noexcept
    {
      node->unlink_checked();

      if (is_container_node())
        insert_root_node(node);
      else if (this->m_has_r)
        next()->insert_to_right(node);
      else
        insert_to_right(node);
    }

    void rbtree_node<void, void>::unlink_cleanup() noexcept
    {
      // We can do our best to check corrupt here
      assert (m_l && m_l->m_p != this);
      assert (m_r && m_r->m_p != this);
      assert (m_p && m_p->m_p != this);
      assert (m_p && m_p->m_l != this);
      assert (m_p && m_p->m_r != this);

      m_l = m_r = m_p = nullptr;
      m_is_container = m_is_red = m_has_l = m_has_r = false;
    }

    rbtree_node<void, void> *rbtree_node<void, void>::unlink() noexcept
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


    rbtree_node<void, void> *
      rbtree_node<void, void>::unlink_checked() noexcept
    {
      if (is_container_node())
        return nullptr;
      if (is_linked())
        return unlink();
      else
        return nullptr;
    }

    void rbtree_node<void, void>::
      transfer_link(rbtree_node<void, void> &node) noexcept
    {
      assert(!node.m_is_container);
      assert(!m_is_container);
      assert(!node.is_linked());

      if (is_linked())
      {
        if (is_empty_container_node())
        {
          node.~rbtree_node();
          new (&node) rbtree_node(container);
          unlink_cleanup();
          return;
        }

        node.m_p = m_p;
        node.m_l = m_l;
        node.m_r = m_r;
        node.m_has_l = m_has_l;
        node.m_has_r = m_has_r;
        node.m_is_red = m_is_red;

        if (m_is_container)
        {
          node.m_is_container = true;
          node.m_p->m_p = &node;
          node.m_l->m_l = &node;
          node.m_r->m_r = &node;
          return;
        }

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

    void rbtree_node<void, void>::
    rebalance_for_insertion(rbtree_node *node) noexcept
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

    void rbtree_node<void, void>::
      rebalance_for_unlink(rbtree_node *node) noexcept
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

    void rbtree_node<void, void>::
      swap_nodes(rbtree_node<void, void> &lhs, rbtree_node<void, void> &rhs)
      noexcept
    {
      rbtree_node tmp;
      lhs.transfer_link(tmp);
      rhs.transfer_link(lhs);
      tmp.transfer_link(rhs);
    }
  }
}
