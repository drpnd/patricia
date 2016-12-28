/*_
 * Copyright (c) 2016 Hirochika Asai <asai@jar.jp>
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "patricia.h"

/*
 * Initialize the data structure for patricia trie
 */
struct patricia *
patricia_init(struct patricia *rt)
{
    if ( NULL == rt ) {
        /* Allocate new data structure */
        rt = malloc(sizeof(struct patricia));
        if ( NULL == rt ) {
            return NULL;
        }
        rt->_allocated = 1;
    } else {
        rt->_allocated = 0;
    }

    /* Set NULL to the root node */
    rt->root = NULL;

    return rt;
}

/*
 * Release the node and descendant nodes
 */
static void
_free_nodes(struct patricia_node *node)
{
    if ( NULL != node ) {
        _free_nodes(node->left);
        _free_nodes(node->right);
        free(node);
    }
}

/*
 * Release the patricia
 */
void
patricia_release(struct patricia *rt)
{
    _free_nodes(rt->root);
    if ( rt->_allocated ) {
        free(rt);
    }
}


/*
 * Recursive process of the lookup procedure
 */
static void *
_lookup(struct patricia_node *p, struct patricia_node *x, uint8_t *key)
{
    struct patricia_node *r0;

    r0 = NULL;
    while ( NULL != x && ( NULL == p || p->bit > x->bit ) ) {
        p = x;
        if ( (key[x->bit >> 4] & (x->bit & 0x7)) & 1 ) {
            /* Right */
            x = x->right;
        } else {
            /* Left */
            x = x->left;
        }
    }

    if ( NULL != x && 0 == memcmp(key, x->key, x->prefixlen) ) {
        if ( NULL == r0 || x->prefixlen > r0->prefixlen ) {
            r0 = x;
        }
    }

    return r0;
}

/*
 * Lookup the data corresponding to the key specified by the argument
 */
void *
patricia_lookup(struct patricia *rt, uint8_t *key)
{
    struct patricia_node *p;
    struct patricia_node *x;
    struct patricia_node *r;

    p = NULL;
    x = rt->root;

    r = _lookup(p, x, key);

    return NULL != r ? r->data : NULL;
}

/*
 * Add a data value (recursive)
 */
static int
_add(struct patricia_node **cur, uint8_t *key, int prefixlen, void *data,
     int depth)
{
    struct patricia_node *new;

    /* Allocate a new node */
    if ( NULL == *cur ) {
        new = malloc(sizeof(struct patricia_node));
        if ( NULL == new ) {
            return -1;
        }
        memset(new, 0, sizeof(struct patricia_node));
        *cur = new;
    }

    if ( prefixlen == depth ) {
        /* The current node is the point to add the data */
        if ( (*cur)->valid ) {
            /* Already exists */
            return -1;
        }
        (*cur)->valid = 1;
        (*cur)->data = data;
        return 0;
    } else {
        /* Check the corresponding bit */
        if ( key[depth >> 4] & (1 << (depth & 0x7)) ) {
            /* Right node */
            return _add(&(*cur)->right, key, prefixlen, data, depth + 1);
        } else {
            /* Left node */
            return _add(&(*cur)->left, key, prefixlen, data, depth + 1);
        }
    }
}

/*
 * Add a data value to the key
 */
int
patricia_add(struct patricia *rt, uint8_t *key, int prefixlen, void *data)
{
    return _add(&rt->root, key, prefixlen, data, 0);
}

/*
 * Delete the data value corresponding to the key and return it
 */
void *
patricia_delete(struct patricia *rt, uint8_t *key, int prefixlen)
{
    return NULL;
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */

