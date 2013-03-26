/*
 * prioque.h: header filr for priority queue
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

#ifndef __PQUEUE_H__
#define __PQUEUE_H__

#include <stddef.h>
#include <stdint.h> /* for intptr_t */
#include <limits.h> /* for SIZE_MAX */

#define PRIOQUE_NODE_INITIALIZER { -1 }

/**
 * Private data structure
 */
struct __prioque_t;
struct __prioque_elem_t;

/**
 * A typedef for **so called** portibility
 */
typedef unsigned prioque_weight_t;

typedef struct __prioque_t *prioque_t;

typedef struct prioque_node_t {
    /* Private data, don't touch them */
    intptr_t __offset;
} prioque_node_t;


/**
 * @brief Create a priority queue
 */
prioque_t prioque_create ();

/**
 * @brief Destroy a priority queue
 * @param q priority queue
 */
void prioque_destroy (prioque_t q);

/**
 * @brief Insert a node into priority queue
 * @param q priority queue
 * @param n the node to be inserted
 * @param weight the priority of the node
 */
int prioque_insert (prioque_t q, prioque_node_t *n, prioque_weight_t weight);


/**
 * @brief Update priority for a node
 * @param q priority queue
 * @param n the node to be inserted
 * @param weight the priority of the node
 */
int prioque_update (prioque_t q, prioque_node_t *n, prioque_weight_t weight);

/**
 * @brief First node of the priority queue
 * @param q priority queue
 */
int prioque_front (prioque_t q, prioque_node_t **n);


/**
 * @brief Remove a node from priority queue
 * @param q priority queue
 * @param n node to be removed
 */
int prioque_remove (prioque_t q, prioque_node_t *n);

/**
 * @brief Get length of priority queue
 * @param q priority queue
 */
int prioque_length (prioque_t q, size_t *length);

/**
 * @brief Get weight of a given node in a priority queue
 * @param q priority queue
 * @param n node of which to get weight
 */
int prioque_get_node_weight (prioque_t q, prioque_node_t *n,
                             prioque_weight_t *weight);

#endif
