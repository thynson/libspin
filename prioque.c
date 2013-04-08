/*
 * prioque.c: source file for prioque queue
 *
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

#include "prioque.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>

#define PRIOQUE_INIT_SIZE 32


typedef struct __prioque_elem_t {
    prioque_weight_t weight;
    prioque_node_t *node;
} *prioque_elem_t;

struct __prioque_t {
    prioque_elem_t queue;
    size_t size;
    size_t count;
};


static prioque_elem_t adjust_heap (prioque_elem_t first,
                                   prioque_elem_t current,
                                   prioque_elem_t boundry)
{
    struct __prioque_elem_t tmp = *current;

    for ( ; ; ) {
        prioque_elem_t left_child = first + (current - first) * 2 + 1;
        prioque_elem_t right_child = first + (current - first + 1) * 2;
        prioque_elem_t elem = &tmp;

        if (left_child < boundry && left_child->weight < elem->weight)
            elem = left_child;

        if (right_child < boundry && right_child->weight < elem->weight)
            elem = right_child;

        if (elem != &tmp) {
            *current = *elem;
            current->node->__offset = current - first;
            current = elem;
        } else
            break;
    }

    *current = tmp;
    current->node->__offset = current - first;
    return current;
}

static prioque_elem_t push_heap (prioque_elem_t first, prioque_elem_t elem)
{
    struct __prioque_elem_t tmp;
    if (first == elem)
        return elem;
    tmp = *elem;
    while (elem != first) {
        prioque_elem_t parent = first + (elem - first - 1) / 2;
        if (tmp.weight >= parent->weight)
            break;
        *elem = *parent;
        elem->node->__offset = elem - first;
        elem = parent;
    }
    *elem = tmp;
    elem->node->__offset = elem - first;
    return elem;
}

static prioque_elem_t pop_heap (prioque_elem_t first, prioque_elem_t last,
                                prioque_elem_t target)
{
    struct __prioque_elem_t tmp;
    if (last == target)
        return last;
    tmp = *target;
    *target = *last;
    adjust_heap(first, target, last);
    *last = tmp;
    last->node->__offset = last - first;
    return last;
}

prioque_t prioque_create ()
{
    prioque_t ret;
    ret = (prioque_t) malloc (sizeof (*ret));
    if (ret == NULL)
        return NULL;

    ret->count = 0;
    ret->size = PRIOQUE_INIT_SIZE;
    ret->queue = (prioque_elem_t) malloc (ret->size * sizeof (*ret->queue));

    if (ret->queue == NULL)
        goto clean_and_exit;

    return ret;

clean_and_exit:
    if (ret->queue != NULL)
        free (ret->queue);
    free (ret);
    return NULL;
}

void prioque_destroy (prioque_t q)
{
    if (q == NULL) {
        errno = EINVAL;
        return;
    }
    free (q->queue);
    free (q);
}

int prioque_insert (prioque_t q, prioque_node_t *n, prioque_weight_t weight)
{
    if (q == NULL || n == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (q->size == q->count) {
        prioque_elem_t new_queue;
        q->size <<= 1;
        new_queue = (prioque_elem_t) realloc (q->queue,
                    q->size * sizeof (*new_queue));
        if (new_queue == NULL) {
            q->size >>= 1;
            return -1;
        } else {
            q->queue = new_queue;
        }
    }

    q->queue[q->count].node = n;
    q->queue[q->count].weight = weight;
    n->__offset = q->count;
    push_heap(q->queue, q->queue + q->count);
    q->count++;
    return 0;
}

int prioque_update (prioque_t q, prioque_node_t *n, prioque_weight_t weight)
{
    if (q == NULL || n == NULL || n->__offset < 0 || n->__offset >= q->count
            || q->queue[n->__offset].node != n) {
        errno = EINVAL;
        return -1;
    }

    prioque_elem_t elem = q->queue + n->__offset;
    if (elem->weight < weight) {
        elem->weight = weight;
        adjust_heap (q->queue, elem, q->queue + q->count);
    } else if (weight < elem->weight) {
        elem->weight = weight;
        push_heap (q->queue, elem);
    }
    return 0;
}

int prioque_remove (prioque_t q, prioque_node_t *n)
{
    if (q == NULL || n == NULL || n->__offset < 0 || n->__offset >= q->count
            || q->queue[n->__offset].node != n) {
        errno = EINVAL;
        return -1;
    }

    prioque_elem_t elem = q->queue + n->__offset;
    q->count--;
    elem = pop_heap (q->queue, q->queue + q->count, elem);
    elem->node = NULL;
    n->__offset = -1;
    return 0;
}

int prioque_front (prioque_t q, prioque_node_t **n)
{
    if (q == NULL || n == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (q->count == 0)
        *n = NULL;
    else
        *n = q->queue[0].node;

    return 0;
}

int prioque_length (prioque_t q, size_t *length)
{
    if (q == NULL || length == NULL) {
        errno = EINVAL;
        return -1;
    }
    *length = q->count;
    return 0;
}

int prioque_is_empty (prioque_t q)
{
    if (q == NULL) {
        errno = EINVAL;
        return -1;
    }
    return q->count == 0 ? 1 : 0;
}

int prioque_get_node_weight (prioque_t q, prioque_node_t *n,
                             prioque_weight_t *weight)
{
    if (q == NULL || n == NULL || n->__offset < 0 || n->__offset >= q->count
            || q->queue[n->__offset].node != n) {
        errno = EINVAL;
        return -1;
    }
    *weight = q->queue[n->__offset].weight;
    return 0;
}
