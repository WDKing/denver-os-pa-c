/*
 * School: Metropolitan State University of Denver
 * Semester: Spring 2016
 * Course: Operating Systems
 * Programming Assignment 1 
 * Created by Ivo Georgiev on 2/9/16.
 * Modified by William King
 * Version: 20Feb2016-2231
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h> // for perror()

#include "mem_pool.h"

/*************/
/*           */
/* Constants */
/*           */
/*************/
static const float      MEM_FILL_FACTOR                 = 0.75;
static const unsigned   MEM_EXPAND_FACTOR               = 2;

static const unsigned   MEM_POOL_STORE_INIT_CAPACITY    = 20;
static const float      MEM_POOL_STORE_FILL_FACTOR      = 0.75; // MEM_FILL_FACTOR;
static const unsigned   MEM_POOL_STORE_EXPAND_FACTOR    = 2;    // MEM_EXPAND_FACTOR;

static const unsigned   MEM_NODE_HEAP_INIT_CAPACITY     = 40;
static const float      MEM_NODE_HEAP_FILL_FACTOR       = 0.75; // MEM_FILL_FACTOR;
static const unsigned   MEM_NODE_HEAP_EXPAND_FACTOR     = 2;    // MEM_EXPAND_FACTOR;

static const unsigned   MEM_GAP_IX_INIT_CAPACITY        = 40;
static const float      MEM_GAP_IX_FILL_FACTOR          = 0.75; // MEM_FILL_FACTOR;
static const unsigned   MEM_GAP_IX_EXPAND_FACTOR        = 2;    // MEM_EXPAND_FACTOR;



/*********************/
/*                   */
/* Type declarations */
/*                   */
/*********************/
typedef struct _node {
    alloc_t alloc_record;
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



/***************************/
/*                         */
/* Static global variables */
/*                         */
/***************************/
static pool_mgr_pt *pool_store = NULL; // an array of pointers, only expand
static unsigned pool_store_size = 0;
static unsigned pool_store_capacity = 0;



/********************************************/
/*                                          */
/* Forward declarations of static functions */
/*                                          */
/********************************************/
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



/****************************************/
/*                                      */
/* Definitions of user-facing functions */
/*                                      */
/****************************************/
alloc_status mem_init() {
    // ensure that it's called only once until mem_free
    // allocate the pool store with initial capacity
    // note: holds pointers only, other functions to allocate/deallocate

    alloc_status new_alloc =  ALLOC_FAIL;

    if(!pool_store) {
        unsigned pool_store_element_size =
                sizeof(char) /* pool: mem */
                + sizeof(alloc_policy) /* pool: policy */
                + sizeof(size_t) /* pool: total_size */
                + sizeof(size_t) /* pool: alloc_size */
                + sizeof(unsigned) /* pool: num_allocs */
                + sizeof(unsigned) /* pool: num_gaps */
                + sizeof(alloc_t)  /* node: alloc_record */
                + sizeof(unsigned) /* node: used */
                + sizeof(unsigned) /* node: allocated */
                + sizeof(node_pt) /* node: next */
                + sizeof(node_pt) /* node: prev */
                + sizeof(size_t) /* gap: size */
                + sizeof(node_pt) /* gap: node */
                + sizeof(unsigned) /* pool_mgr: total_nodes */
                + sizeof(unsigned) /* pool_mgr: used_nodes */
                + sizeof(unsigned);  /* pool_mgr: gap_ix_capacity */

        pool_store_capacity = MEM_POOL_STORE_INIT_CAPACITY * pool_store_element_size;
        pool_store = (pool_mgr_pt *)calloc(pool_store_capacity,pool_store_element_size);

        if(pool_store) {
            new_alloc = ALLOC_OK;
        }
    }
    else {
        new_alloc = ALLOC_CALLED_AGAIN;
    }

    return new_alloc;
}

alloc_status mem_free() {
    // ensure that it's called only once for each mem_init
    // make sure all pool managers have been deallocated
    // can free the pool store array
    // update static variables

    alloc_status free_status = ALLOC_OK, pool_close_status;

    if(pool_store) {
        for(int index=0; index<pool_store_size; index++) {
            pool_close_status = mem_pool_close((pool_pt)pool_store[index]);

            if(pool_close_status != ALLOC_OK ){
                free_status = ALLOC_NOT_FREED;
            }
        }

        if(free_status == ALLOC_OK) {
            free(pool_store);
            pool_store_size = 0;
            pool_store_capacity = 0;
        }
    }

    return free_status;
}

pool_pt mem_pool_open(size_t size, alloc_policy policy) {
    // TODO
    // make sure there the pool store is allocated
    // expand the pool store, if necessary
    // allocate a new mem pool mgr
    // check success, on error return null
    // allocate a new memory pool
    // check success, on error deallocate mgr and return null
    // allocate a new node heap
    // check success, on error deallocate mgr/pool and return null
    // allocate a new gap index
    // check success, on error deallocate mgr/pool/heap and return null
    // assign all the pointers and update meta data:
    //   initialize top node of node heap
    //   initialize top node of gap index
    //   initialize pool mgr
    //   link pool mgr to pool store
    // return the address of the mgr, cast to (pool_pt)
    pool_pt new_pool = NULL;

    if(pool_store) {

        if((pool_store_size/pool_store_capacity) > MEM_POOL_STORE_FILL_FACTOR) {
            _mem_resize_pool_store();
        }

        new_pool = (pool_pt)malloc(
                sizeof(char)                     /* size */
                + sizeof(alloc_policy)  /* policy */
                + sizeof(size_t)         /* total_size */
                + sizeof(size_t)        /* alloc_size */
                + sizeof(unsigned)       /* num_allocs */
                + sizeof(unsigned)       /* num_gaps */
                + sizeof(node_pt) /* node_heap */
                + sizeof(unsigned)        /* total_nodes */
                + sizeof(unsigned)      /* used_nodes */
                + sizeof(gap_pt)     /* gap_ix */
                + sizeof(unsigned) ) ;   /* gap_ix_capacity */

        if(new_pool) {
            // TODO - finish step by step as above
            (new_pool)->policy = policy;
            (new_pool)->mem = (char *)malloc(size);
            (new_pool)->total_size = size;
            (new_pool)->alloc_size = MEM_POOL_STORE_INIT_CAPACITY;
            (new_pool)->num_allocs = 0;
            (new_pool)->num_gaps = 1;
            ((pool_mgr_pt)new_pool)->node_heap = (node_pt)malloc(MEM_NODE_HEAP_INIT_CAPACITY * sizeof(node_t));
            ((pool_mgr_pt)new_pool)->gap_ix = (gap_pt)malloc(MEM_GAP_IX_INIT_CAPACITY * sizeof(gap_t));
            ((pool_mgr_pt)new_pool)->gap_ix_capacity = MEM_GAP_IX_INIT_CAPACITY;

            pool_store[size] = (pool_mgr_pt)new_pool;
            pool_store_size++;
        }

    }

    return new_pool;
}

alloc_status mem_pool_close(pool_pt pool) {
    // TODO
    // get mgr from pool by casting the pointer to (pool_mgr_pt)
    // check if this pool is allocated
    // check if pool has only one gap
    // check if it has zero allocations
    // free memory pool
    // free node heap
    // free gap index
    // find mgr in pool store and set to null
    // note: don't decrement pool_store_size, because it only grows
    // free mgr

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
    // get mgr from pool by casting the pointer to (pool_mgr_pt)
    // check if any gaps, return null if none
    // expand heap node, if necessary, quit on error
    // check used nodes fewer than total nodes, quit on error
    // get a node for allocation:
    // if FIRST_FIT, then find the first sufficient node in the node heap
    // if BEST_FIT, then find the first sufficient node in the gap index
    // check if node found
    // update metadata (num_allocs, alloc_size)
    // calculate the size of the remaining gap, if any
    // remove node from gap index
    // convert gap_node to an allocation node of given size
    // adjust node heap:
    //   if remaining gap, need a new node
    //   find an unused one in the node heap
    //   make sure one was found
    //   initialize it to a gap node
    //   update metadata (used_nodes)
    //   update linked list (new node right after the node for allocation)
    //   add to gap index
    //   check if successful
    // return allocation record by casting the node to (alloc_pt)

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
    // get mgr from pool by casting the pointer to (pool_mgr_pt)
    // get node from alloc by casting the pointer to (node_pt)
    // find the node in the node heap
    // this is node-to-delete
    // make sure it's found
    // convert to gap node
    // update metadata (num_allocs, alloc_size)
    // if the next node in the list is also a gap, merge into node-to-delete
    //   remove the next node from gap index
    //   check success
    //   add the size to the node-to-delete
    //   update node as unused
    //   update metadata (used nodes)
    //   update linked list:
    /*
                    if (next->next) {
                        next->next->prev = node_to_del;
                        node_to_del->next = next->next;
                    } else {
                        node_to_del->next = NULL;
                    }
                    next->next = NULL;
                    next->prev = NULL;
     */

    // this merged node-to-delete might need to be added to the gap index
    // but one more thing to check...
    // if the previous node in the list is also a gap, merge into previous!
    //   remove the previous node from gap index
    //   check success
    //   add the size of node-to-delete to the previous
    //   update node-to-delete as unused
    //   update metadata (used_nodes)
    //   update linked list
    /*
                    if (node_to_del->next) {
                        prev->next = node_to_del->next;
                        node_to_del->next->prev = prev;
                    } else {
                        prev->next = NULL;
                    }
                    node_to_del->next = NULL;
                    node_to_del->prev = NULL;
     */
    //   change the node to add to the previous node!
    // add the resulting node to the gap index
    // check success

    // TODO implement
    // find the node location for which the allocation pointer points
    // change current node to use 0
    // if next node is also use 0, change allocation to 0 and add size
    // to current node, change next to next's next
    // if previous node is also use 0, change current allocation to 0, and add size
    // to previous node, change previous' next to current's next
    // TODO
    pool_mgr_pt pool_mgr = (pool_mgr_pt)pool;
    node_pt curr_node_pt = pool_mgr->node_heap;




    return ALLOC_FAIL;
}

void mem_inspect_pool(pool_pt pool,
                      pool_segment_pt *segments,
                      unsigned *num_segments) {
    
    // TODO
    // get the mgr from the pool
    // allocate the segments array with size == used_nodes
    // check successful
    // loop through the node heap and the segments array
    //    for each node, write the size and allocated in the segment
    // "return" the values:
    /*
                    *segments = segs;
                    *num_segments = pool_mgr->used_nodes;
     */
     
     
}



/***********************************/
/*                                 */
/* Definitions of static functions */
/*                                 */
/***********************************/
static alloc_status _mem_resize_pool_store() {
    
    // TODO
    // check if necessary
    /*
                if (((float) pool_store_size / pool_store_capacity)
                    > MEM_POOL_STORE_FILL_FACTOR) {...}
     */
    // don't forget to update capacity variables




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
    // TODO
    // see above

    alloc_status new_status = ALLOC_OK;





    return ALLOC_FAIL;
}

static alloc_status _mem_resize_gap_ix(pool_mgr_pt pool_mgr) {
    // TODO
    // see above

    return ALLOC_FAIL;
}

static alloc_status _mem_add_to_gap_ix(pool_mgr_pt pool_mgr,
                                       size_t size,
                                       node_pt node) {

    // TODO
    // expand the gap index, if necessary (call the function)
    // add the entry at the end
    // update metadata (num_gaps)
    // sort the gap index (call the function)
    // check success

    return ALLOC_FAIL;
}

static alloc_status _mem_remove_from_gap_ix(pool_mgr_pt pool_mgr,
                                            size_t size,
                                            node_pt node) {
    // TODO
    // find the position of the node in the gap index
    // loop from there to the end of the array:
    //    pull the entries (i.e. copy over) one position up
    //    this effectively deletes the chosen node
    // update metadata (num_gaps)
    // zero out the element at position num_gaps!

    gap_pt new_gap = pool_mgr->gap_ix;
    int location = -1;

    for(unsigned index = 0; index < pool_mgr->gap_ix_capacity; index++) {
        if(new_gap[index].node == node) {
            location = index;
        }
    }

    if(location == -1) {
        return ALLOC_FAIL;
    }
    else {
        for(int trace = location; trace < pool_mgr->gap_ix_capacity-1; trace++) {
            new_gap[trace] = new_gap[trace+1];
        }
        new_gap[pool_mgr->gap_ix_capacity-1].size = 0;
        new_gap[pool_mgr->gap_ix_capacity-1].node = NULL;

        pool_mgr->gap_ix_capacity -= 1;

        return ALLOC_OK;
    }

}

// note: only called by _mem_add_to_gap_ix, which appends a single entry
static alloc_status _mem_sort_gap_ix(pool_mgr_pt pool_mgr) {
    // the new entry is at the end, so "bubble it up"
    // loop from num_gaps - 1 until but not including 0:
    //    if the size of the current entry is less than the previous (u - 1)
    //       swap them (by copying) (remember to use a temporary variable)

    gap_pt new_gap = pool_mgr->gap_ix;
    gap_t temp_gap;

    if(new_gap) {
        for (unsigned index = pool_mgr->gap_ix_capacity; index > 0; index--) {
            if (new_gap[index].size < new_gap[index - 1].size) {
                temp_gap = new_gap[index];
                new_gap[index - 1] = temp_gap;
                new_gap[index] = temp_gap;
            }
        }

        return ALLOC_OK;
    }
    else {
        return ALLOC_FAIL;
    }
}


