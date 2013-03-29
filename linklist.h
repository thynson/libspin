/*
 * Copyright (C) 2013 LAN Xingcan
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

#ifndef __LINKLIST_H__
#define __LINKLIST_H__

#include <stddef.h>
#include <assert.h>

struct link_node_t;

typedef struct link_list_t {
    struct link_node_t *head;
    struct link_node_t *tail;
} link_list_t;

typedef struct link_node_t {
    struct link_node_t *next;
    struct link_node_t *prev;
} link_node_t;


static inline void link_list_init (link_list_t *l)
{
    assert (l != NULL);
    l->head = NULL;
    l->tail = NULL;
}

static inline int link_list_is_empty (link_list_t *l)
{
    assert (l != NULL);
    return l->head == NULL && l->tail == NULL;
}

static inline void link_list_swap (link_list_t *lhs, link_list_t *rhs)
{
    struct link_node_t *tmp_head, *tmp_tail;

    assert (lhs != NULL);
    assert (rhs != NULL);

    tmp_head = lhs->head;
    tmp_tail = lhs->tail;
    lhs->head = rhs->head;
    lhs->tail = rhs->tail;
    rhs->head = tmp_head;
    rhs->tail = tmp_tail;
}

static inline void link_list_cat (link_list_t *dst, link_list_t *src)
{
    assert (dst != NULL);
    assert (src != NULL);

    if (dst->head == NULL) {
        assert (dst->tail == NULL);
        dst->head = src->head;
        dst->tail = src->tail;
        src->head = src->tail = NULL;
    } else if (src->head != NULL) {
        assert (dst->tail != NULL);
        dst->tail->next = src->head;
        src->head->prev = dst->tail;
        dst->tail = src->tail;
        src->head = src->tail = NULL;
    }
}

static inline void link_list_attach_to_tail (link_list_t *l, link_node_t *n)
{
    assert (l != NULL && n != NULL);

    if (l->head != NULL) {
        assert (l->tail != NULL);
        n->next = n;
        n->prev = l->tail;
        l->tail->next = n;
        l->tail = n;
    } else {
        assert (l->tail == NULL);
        n->next = n->prev = n;
        l->head = l->tail = n;
    }
}

static inline void link_list_attach_to_head (link_list_t *l, link_node_t *n)
{
    assert (l != NULL && n != NULL);

    if (l->head != NULL) {
        assert (l->tail != NULL);
        n->prev = n;
        n->next = l->head;
        l->head->prev = n;
        l->head = n;
    } else {
        assert (l->tail == NULL);
        n->next = n->prev = n;
        l->head = l->tail = n;
    }
}

static inline void link_list_dettach (link_list_t *l, link_node_t *n)
{
    assert (l != NULL && n != NULL);

    if (n->next == n) {
        if (n->prev == n) {
            n->next = n->prev = l->head = l->tail = NULL;
        } else {
            l->tail = n->prev;
            n->prev->next = n->prev;
            n->next = n->prev = NULL;
        }
    } else {
        if (n->prev == n) {
            l->head = n->next;
            n->next->prev = n->next;
            n->prev = n->next = NULL;
        } else {
            n->next->prev = n->prev;
            n->prev->next = n->next;
            n->next = n->prev = NULL;
        }
    }
}

#endif
