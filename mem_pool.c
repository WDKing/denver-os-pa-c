/*
 * Created by Ivo Georgiev on 2/9/16.
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h> // for perror()

#include "mem_pool.h"

/* Constants */
static const float      MEM_FILL_FACTOR                 = 0.75;
static const unsigned   MEM_EXPAND_FACTOR               = 2;

static const unsigned   MEM_POOL_STORE_INIT_CAPACITY    = 20;
static const float      MEM_POOL_STORE_FILL_FACTOR      = 0.75; /* MEM_FILL_FACTOR; */
static const unsigned   MEM_POOL_STORE_EXPAND_FACTOR    = 2; /* MEM_EXPAND_FACTOR; */

static const unsigned   MEM_NODE_HEAP_INIT_CAPACITY     = 40;
static const float      MEM_NODE_HEAP_FILL_FACTOR       = 0.75; /* MEM_FILL_FACTOR; */
static const unsigned   MEM_NODE_HEAP_EXPAND_FACTOR     = 2; /* MEM_EXPAND_FACTOR; */

static const unsigned   MEM_GAP_IX_INIT_CAPACITY        = 40;
static const float      MEM_GAP_IX_FILL_FACTOR          = 0.75; /* MEM_FILL_FACTOR; */
static const unsigned   MEM_GAP_IX_EXPAND_FACTOR        = 2; /* MEM_EXPAND_FACTOR; */

/* Type declarations */
typedef struct _node {
    alloc_pt alloc_record;
    unsigned used;
    unsigned allocated;
    struct _node *next, *prev; // doubly-linked list for gap deletion
} node_t, *node_pt;

typedef struct _gap {
    size_t size;
    node_pt node;
} gap_t, *gap_pt;

typedef struct _pool_mgr {
    pool_t pool;
    node_pt node_heap;
    unsigned total_nodes;
    unsigned used_nodes;
    gap_pt gap_ix;
    unsigned gap_ix_capacity;
} pool_mgr_t, *pool_mgr_pt;


/* Static global variables */
static pool_mgr_pt *pool_store = NULL; // an array of pointers, only expand
static unsigned pool_store_size = 0;
static unsigned pool_store_capacity = 0;


/* Forward declarations of static functions */
static alloc_status _mem_resize_pool_store();
static alloc_status _mem_resize_node_heap(pool_mgr_pt pool_mgr);
static alloc_status _mem_resize_gap_ix(pool_mgr_pt pool_mgr);
static alloc_status
        _mem_add_to_gap_ix(pool_mgr_pt pool_mgr,
                           size_t size,
                           node_pt node);
static alloc_status
        _mem_remove_from_gap_ix(pool_mgr_pt pool_mgr,
                                size_t size,
                                node_pt node);
static alloc_status _mem_sort_gap_ix(pool_mgr_pt pool_mgr);


/* Definitions of user-facing functions */
alloc_status mem_init() {
    pool_store_capacity = MEM_POOL_STORE_INIT_CAPACITY * sizeof(pool_mgr_t);
    *pool_store = malloc(sizeof(pool_store_capacity));

    if(pool_store) {
        return ALLOC_OK;
    }
    else {
        return ALLOC_FAIL;
    }
}

alloc_status mem_free() {
    alloc_status free_status = ALLOC_OK, pool_close_status;

    for(int index=0; index<pool_store_size; index++) {
        pool_close_status = mem_pool_close((pool_pt)pool_store[index]);

        if(pool_close_status != ALLOC_OK) {
            free_status = ALLOC_FAIL;
        }
    }

    free(pool_store);

    if(!pool_store) {
        free_status = ALLOC_FAIL;
    }

    return free_status;
}

pool_pt mem_pool_open(size_t size, alloc_policy policy) {

    if((pool_store_size/pool_store_capacity) > MEM_POOL_STORE_FILL_FACTOR) {
        _mem_resize_pool_store();
    }

    pool_pt new_pool = (pool_pt)malloc(
        size                                              /* size */
        + sizeof(alloc_policy)                            /* policy */
        + sizeof(size_t)                                  /* total_size */
        + sizeof(size_t)                                  /* alloc_size */
        + sizeof(unsigned)                                /* num_allocs */
        + sizeof(unsigned)                                /* num_gaps */
        + (MEM_NODE_HEAP_INIT_CAPACITY * sizeof(node_pt)) /* node_heap */
        + sizeof(unsigned)                                /* total_nodes */
        + sizeof(unsigned)                                /* used_nodes */
        + (MEM_GAP_IX_INIT_CAPACITY * sizeof(gap_pt))     /* gap_ix */
        + sizeof(unsigned) );                             /* gap_ix_capacity */

    (new_pool)->policy = policy;

    pool_store[size] = (pool_mgr_pt)new_pool;

    return (pool_pt)pool_store;
}

alloc_status mem_pool_close(pool_pt pool) {
    alloc_status free_pool_status = ALLOC_OK;

    for(int index=0; index < pool_store_size; index++) {
        if(pool == (pool_pt)pool_store[index]) {
            free(pool_store[index]);

            if(pool_store[index] != NULL) {
                free_pool_status = ALLOC_FAIL;
            }
        }
    }

    return free_pool_status;
}

alloc_pt mem_new_alloc(pool_pt pool, size_t size) {
    // TODO
    alloc_pt new_alloc = NULL, current_alloc;
    gap_pt current_gap;
    node_pt current_node;
    size_t location, difference;     /* track which gap index to follow */
    pool_mgr_pt pool_mgr = (pool_mgr_pt)pool;

    if((pool_mgr->used_nodes
            / pool_mgr->total_nodes)
            > MEM_NODE_HEAP_FILL_FACTOR) {
        _mem_resize_node_heap((pool_mgr_pt)pool);
    }

    if(pool_mgr->gap_ix == NULL){
        new_alloc = NULL;
    }
    else {
        current_gap = pool_mgr->gap_ix;
        location = current_gap->size;

        if (pool->policy == FIRST_FIT) {
            for (unsigned index = 0; index < pool_mgr->gap_ix_capacity; index++) {
                if (current_gap[index].size > size && location > index) {
                    location = index;
                }
            }
        }
        else { /* BEST_FIT */
            for (unsigned index = 0; index < pool_mgr->gap_ix_capacity; index++) {
                if (current_gap[index].size > size) {
                    if ((current_gap[index].size - size)
                        < (current_gap[location].size - size)) {
                        location = index;
                    }
                }
            }
        }

        // TODO check if the size matches exactly, if yes done, otherwise continue
        // find the next spare slot in the node heap
        // assign the next value to the next node, and the next node's next to this
        // nodes former next
        // change allocation of next node to 1, use to 0
        // change next node's size to difference of current and size
        // change current node's use to 1, change current size

    }

    return new_alloc;
}

alloc_status mem_del_alloc(pool_pt pool, alloc_pt alloc) {
    // TODO implement
    // find the node location for which the allocation pointer points
    // change current node to use 0
    // if next node is also use 0, change allocation to 0 and add size
    // to current node, change next to next's next
    // if previous node is also use 0, change current allocation to 0, and add size
    // to previous node, change previous' next to current's next

    pool_mgr_pt pool_mgr = (pool_mgr_pt)pool;
    node_pt curr_node_pt = pool_mgr->node_heap;




    return ALLOC_FAIL;
}

// NOTE: Allocates a dynamic array. Caller responsible for releasing.
void mem_inspect_pool(pool_pt pool, pool_segment_pt segments, unsigned *num_segments) {
    // TODO implement
}


/* Definitions of static functions */
static alloc_status _mem_resize_pool_store() {
    pool_mgr_pt *new_pool_store = NULL;
    unsigned new_pool_store_capacity = MEM_POOL_STORE_EXPAND_FACTOR * pool_store_capacity;
    alloc_status new_alloc_status = ALLOC_OK;

    *new_pool_store = (pool_mgr_pt)realloc(pool_store, new_pool_store_capacity);

    if(new_pool_store == NULL) {
        new_alloc_status = ALLOC_FAIL;
    }
    else {
        pool_store = new_pool_store;
        pool_store_capacity = new_pool_store_capacity;
    }

    return new_alloc_status;
}

static alloc_status _mem_resize_node_heap(pool_mgr_pt pool_mgr) {
    // TODO implement

    return ALLOC_FAIL;
}

static alloc_status _mem_resize_gap_ix(pool_mgr_pt pool_mgr) {
    // TODO implement

    return ALLOC_FAIL;
}

static alloc_status _mem_add_to_gap_ix(pool_mgr_pt pool_mgr,
                                       size_t size,
                                       node_pt node) {
    // TODO implement

    return ALLOC_FAIL;
}

static alloc_status _mem_remove_from_gap_ix(pool_mgr_pt pool_mgr,
                                            size_t size,
                                            node_pt node) {
    // TODO implement

    return ALLOC_FAIL;
}


static alloc_status _mem_sort_gap_ix(pool_mgr_pt pool_mgr) {

    // TODO implement

    return ALLOC_FAIL;
}


