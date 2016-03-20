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
static const float      MEM_POOL_STORE_FILL_FACTOR      = 0.75;
static const unsigned   MEM_POOL_STORE_EXPAND_FACTOR    = 2;

static const unsigned   MEM_NODE_HEAP_INIT_CAPACITY     = 40;
static const float      MEM_NODE_HEAP_FILL_FACTOR       = 0.75;
static const unsigned   MEM_NODE_HEAP_EXPAND_FACTOR     = 2;

static const unsigned   MEM_GAP_IX_INIT_CAPACITY        = 40;
static const float      MEM_GAP_IX_FILL_FACTOR          = 0.75;
static const unsigned   MEM_GAP_IX_EXPAND_FACTOR        = 2;



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
    alloc_status mem_init_status = ALLOC_FAIL;


    if(!pool_store) {

        pool_store = calloc( MEM_POOL_STORE_INIT_CAPACITY, sizeof(pool_mgr_pt));

        if (pool_store) {
            pool_store_capacity = MEM_POOL_STORE_INIT_CAPACITY;
            mem_init_status = ALLOC_OK;
        }
    }
    else {
        mem_init_status = ALLOC_CALLED_AGAIN;
    }

    return mem_init_status;
}

alloc_status mem_free() {
    // ensure that it's called only once for each mem_init
    // make sure all pool managers have been deallocated
    // can free the pool store array
    // update static variables

    alloc_status mem_free_status;

    if (!pool_store) {
        mem_free_status = ALLOC_CALLED_AGAIN;
    }
    else {
            free(pool_store);
            pool_store = NULL;
            pool_store_capacity = 0;
            pool_store_size = 0;
            mem_free_status = ALLOC_OK;
    }

    return mem_free_status;
}

pool_pt mem_pool_open(size_t size, alloc_policy policy) {
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

    pool_mgr_pt new_pool_mgr = NULL;
    char *new_pool = NULL;
    node_pt new_node_heap = NULL;
    gap_pt new_gap_ix = NULL;

    if(pool_store != NULL) {

        _mem_resize_pool_store();

        new_pool_mgr = (pool_mgr_pt)calloc(1,sizeof(pool_mgr_t));

        if(new_pool_mgr) {

            new_pool = (char *)calloc(size,sizeof(char));

            if (new_pool) {

                new_node_heap = (node_pt)calloc(MEM_NODE_HEAP_INIT_CAPACITY,sizeof(node_t));

                if(new_node_heap) {

                    new_gap_ix = (gap_pt)calloc(MEM_GAP_IX_INIT_CAPACITY,sizeof(gap_t));

                    if(new_gap_ix) {

                        // Initialize Node Pointer
                        ((node_pt)new_node_heap)->allocated = 0;
                        ((node_pt)new_node_heap)->used = 1;
                        ((node_pt)new_node_heap)->next = NULL;
                        ((node_pt)new_node_heap)->prev = NULL;

                        // Initialize Allocation Pointer
                        (new_node_heap[0].alloc_record).mem = new_pool;
                        (new_node_heap[0].alloc_record).size = size;

                        // Initialize Memory Pool
                        (new_pool_mgr->pool).mem = new_pool;
                        (new_pool_mgr->pool).policy = policy;
                        (new_pool_mgr->pool).total_size = size;
                        (new_pool_mgr->pool).alloc_size = 0;
                        (new_pool_mgr->pool).num_allocs = 0;
                        (new_pool_mgr->pool).num_gaps = 1;

                        // Initialize Memory Pool Manager
                        new_pool_mgr->total_nodes = MEM_NODE_HEAP_INIT_CAPACITY;
                        new_pool_mgr->used_nodes = 1;
                        new_pool_mgr->gap_ix_capacity = MEM_GAP_IX_INIT_CAPACITY;

                        // Link New Pool Manager to All Else
                        new_pool_mgr->node_heap = new_node_heap;
                        new_pool_mgr->gap_ix = new_gap_ix;

                        // Initialize Gap Index Information
                        _mem_add_to_gap_ix(new_pool_mgr,
                                           size,
                                           (node_pt)new_node_heap);

                        // Link Pool Store to New Pool Manager
                        pool_store[pool_store_size] = new_pool_mgr;
                        pool_store_size++;
                    }
                    else {
                        free(new_node_heap);
                        free(new_pool);
                        free(new_pool_mgr);

                        new_node_heap = NULL;
                        new_pool = NULL;
                        new_pool_mgr = NULL;
                    }
                }
                else {
                    free(new_pool);
                    free(new_pool_mgr);

                    new_pool = NULL;
                    new_pool_mgr = NULL;
                }
            }
            else {
                free(new_pool_mgr);

                new_pool_mgr = NULL;
            }
        }
    }
    return (pool_pt)new_pool_mgr;
}

alloc_status mem_pool_close(pool_pt pool) {
    // get mgr from pool by casting the pointer to (pool_mgr_pt)
    // check if this pool is allocated
    // check if pool has only one gap
    // check if it has zero allocations
    // free memory pool
    // free node heap
    // free gap index
    // find mgr in pool store and set to null
    // note: don't decrement pool_store_size, because it only grows

    alloc_status free_pool_status = ALLOC_FAIL;
    pool_mgr_pt new_pool_mgr = (pool_mgr_pt)pool;
    unsigned count = 0;


    if(new_pool_mgr != NULL) {

        for(unsigned index = 0; index < new_pool_mgr->gap_ix_capacity; index++) {
            if( (new_pool_mgr->gap_ix)[index].size > 0) {
                count++;
            }
        }

        if( count == 1 && pool->num_allocs == 0 ) {

            for (unsigned tally = 0; tally < pool_store_capacity; tally++) {
                if (pool_store[tally] == new_pool_mgr) {

                    free((new_pool_mgr->pool).mem);
                    free(new_pool_mgr->node_heap);
                    free(new_pool_mgr->gap_ix);

                    pool_store[tally] == NULL;

                    break;
                }
            }

            free_pool_status = ALLOC_OK;
        }
        else {
            free_pool_status = ALLOC_NOT_FREED;
        }
    }

    return free_pool_status;
}

alloc_pt mem_new_alloc(pool_pt pool, size_t size) {
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


    alloc_pt new_alloc_pt = NULL;
    alloc_status new_status = ALLOC_FAIL;
    pool_mgr_pt new_pool_mgr = (pool_mgr_pt)pool;
    node_pt working_node = NULL, next_node = NULL;
    node_pt unused_node_locator = NULL;
    gap_pt next_gap = NULL;
    size_t leftover_alloc_size = 0;
    int tally;

    if( (new_pool_mgr->gap_ix)->size > 0 ) {  // Check first gap, if zero there is no available gap

        new_status = _mem_resize_node_heap(new_pool_mgr);
        if( new_status == ALLOC_FAIL) {
            return new_alloc_pt;
        }
        if( new_pool_mgr->used_nodes >= new_pool_mgr->total_nodes ) {
            return new_alloc_pt;
        }

        /*  FIRST_FIT replacement policy */
        if(pool->policy == FIRST_FIT) {

            next_node = (node_pt)new_pool_mgr->node_heap;

            /* Search for first node that is used, but not allocated */
            while(working_node == NULL && next_node != NULL) {

                if(next_node->allocated == 0 && ((alloc_pt)next_node)->size >= size) {
                    working_node = next_node;
                }

                next_node = next_node->next;
            }
        }
        else { /* BEST_FIT */
            tally = 0;
            next_gap = (gap_pt)new_pool_mgr->gap_ix;

            while(tally < new_pool_mgr->gap_ix_capacity) {
                if(next_gap->size >= size){
                    working_node = next_gap->node;
                    break;
                }
                tally++;
                *next_gap = ((new_pool_mgr->gap_ix)[tally]);
            }
        }

        /* No valid node found */
        if(working_node == NULL) {
            return new_alloc_pt;
        }

        /* Calculate differrence between found gap and desired size */
        leftover_alloc_size = ((alloc_pt)working_node)->size - size;
        /* Change current node to allocated */
        working_node->allocated = 1;
        /* Increase total allocation count */
        ((pool_pt)new_pool_mgr)->num_allocs += 1;
        /* Increase allocated size */
        ((pool_pt)new_pool_mgr)->alloc_size += size;

        /* Remove previous gap from gap pool */
        new_status = _mem_remove_from_gap_ix(new_pool_mgr,((alloc_pt)working_node)->size,working_node);

        /* If not removed from Gap Index, Quit */
        if(new_status == ALLOC_FAIL) {
            return NULL;
        }

        /* Desired size matches size of located gap */
        if(leftover_alloc_size == 0) {
            /* Set Node to Allocated */
            working_node->used = 1;
            /* Reduce number of gaps, since no gap leftover */
            pool->num_gaps--;
            /* Assign Node Found to Allocation Pointer */
            new_alloc_pt = (alloc_pt)working_node;
        }
            /* Desired size is smaller than located gap */
        else {
            /* Set desired size into located node */
            ((alloc_pt)working_node)->size = size;
            /* increase total of used nodes */
            new_pool_mgr->used_nodes += 1;

            tally = 0;
            unused_node_locator = ((node_pt)(new_pool_mgr->node_heap));

            while(unused_node_locator->used == 1
                  && tally < new_pool_mgr->total_nodes) {
                tally++;
                unused_node_locator = &((new_pool_mgr->node_heap)[tally]);
            }

            /* Did not find unused node */
            if(unused_node_locator->used == 1) {
                return new_alloc_pt;
            }

            /* Set used status to used */
            unused_node_locator->used = 1;
            /* Set size of located node to remaining size */
            ((alloc_pt)unused_node_locator)->size = leftover_alloc_size;

            /* Insert node between old allocated node, and next node in list */
            unused_node_locator->next = working_node->next;

            if(working_node->next != NULL) {
                (working_node->next)->prev = unused_node_locator;
            }
            else {
                unused_node_locator->next = NULL;
            }
            working_node->next = unused_node_locator;
            unused_node_locator->prev = working_node;

            (unused_node_locator->alloc_record).mem = (working_node->alloc_record).mem + (size * sizeof(char *));

            new_status = _mem_add_to_gap_ix(new_pool_mgr,
                                            ((gap_pt)unused_node_locator)->size,
                                            unused_node_locator);
            if(new_status != ALLOC_FAIL) {
                new_alloc_pt = (alloc_pt)working_node;
            }
        }

        return new_alloc_pt;
    }
    else {
        return new_alloc_pt;
    }
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

    /* Track the status of the attempted deletion */
    alloc_status new_status = ALLOC_FAIL;
    /* Pool_mg_pt for managing the pool */
    pool_mgr_pt new_pool_mgr = (pool_mgr_pt)pool;
    /* Node_pt to travel through node heap */
    node_pt working_node = new_pool_mgr->node_heap;
    /* Node to Delete */
    node_pt node_to_delete;
    /* Boolean to determine if node was found */
    int found = 0;
    size_t working_size;


    if(alloc != NULL && pool != NULL) {
        while (working_node != NULL) {
            if (((alloc_pt) working_node)->size == alloc->size
                && ((alloc_pt)working_node)->mem == alloc->mem) {
                found = 1;
                break;
            }

            working_node = working_node->next;
        }

        if (found) {
            ((pool_pt) new_pool_mgr)->num_allocs -= 1;
            ((pool_pt) new_pool_mgr)->num_gaps += 1;
            ((pool_pt) new_pool_mgr)->alloc_size -= ((gap_pt) working_node)->size;
            working_node->allocated = 0;

            if (working_node->next != NULL) {

                if ((working_node->next)->allocated == 0) {

                    working_size = ((gap_pt) (working_node->next))->size;

                    new_status = _mem_remove_from_gap_ix((pool_mgr_pt) pool,
                                                         ((gap_pt) (working_node->next))->size,
                                                         working_node->next);

                    if ( new_status == ALLOC_OK) {
                        ((gap_pt) working_node)->size = ((gap_pt) working_node)->size
                                                        + working_size;
                        node_to_delete = working_node->next;
                        node_to_delete->used = 0;
                        node_to_delete->allocated = 0;
                        new_pool_mgr->used_nodes -= 1;
                        ((pool_pt) new_pool_mgr)->num_gaps -= 1;

                        if (node_to_delete->next != NULL) {
                            (node_to_delete->next)->prev = working_node;
                            working_node->next = node_to_delete->next;
                            node_to_delete->next = NULL;
                            node_to_delete->prev = NULL;
                        }
                        else {
                            working_node->next = NULL;
                        }
                    }
                }
            }

            if (working_node->prev != NULL) {
                if ((working_node->prev)->allocated == 0) {
                    /* Size of current allocation, to add into previous */
                    working_size = ((gap_pt)working_node)->size;

                    if(_mem_remove_from_gap_ix((pool_mgr_pt)pool,
                                               ((gap_pt)(working_node)->prev)->size,
                                               working_node->prev) == ALLOC_OK) {
                        ((alloc_pt)(working_node->prev))->size += working_size;
                        working_node->used = 0;
                        working_node->allocated = 0;
                        new_pool_mgr->used_nodes = new_pool_mgr->used_nodes - 1;
                        ((pool_pt) new_pool_mgr)->num_gaps -= 1;

                        if(working_node->next != NULL) {
                            node_to_delete = working_node;
                            working_node = working_node->prev;

                            (node_to_delete->next)->prev = working_node;
                            working_node->next = node_to_delete->next;
                            node_to_delete->prev = NULL;
                            node_to_delete->next = NULL;

                        }
                        else {
                            working_node = working_node->prev;
                            (working_node->next)->prev = NULL;
                            working_node->next = NULL;
                        }
                    }
                }
            }

            if (_mem_add_to_gap_ix(new_pool_mgr,
                                   ((gap_pt) working_node)->size,
                                   working_node) == ALLOC_OK) {
                new_status = ALLOC_OK;
            }

        }
    }
    return new_status;
}

void mem_inspect_pool(pool_pt pool,
                      pool_segment_pt *segments,
                      unsigned *num_segments) {
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

    pool_mgr_pt new_pool_mgr = (pool_mgr_pt)pool;
    *num_segments = new_pool_mgr->used_nodes;
    node_pt working_node = new_pool_mgr->node_heap;

    *segments = (pool_segment_pt) calloc(((pool_mgr_pt)pool)->used_nodes, sizeof(pool_segment_t));

    if (*segments != NULL) {

        for (unsigned index = 0; index < new_pool_mgr->used_nodes; index++) {
            (*segments)[index].allocated = working_node->allocated;
            (*segments)[index].size = ((alloc_pt)working_node)->size;

            working_node = working_node->next;
        }
    }
}



/***********************************/
/*                                 */
/* Definitions of static functions */
/*                                 */
/***********************************/
static alloc_status _mem_resize_pool_store() {
    // check if necessary
    /*
                if (((float) pool_store_size / pool_store_capacity)
                    > MEM_POOL_STORE_FILL_FACTOR) {...}
     */
    // don't forget to update capacity variables

    pool_mgr_pt new_pool_store;
    unsigned new_pool_store_capacity;
    alloc_status new_alloc_status = ALLOC_FAIL;

    if((float)pool_store_size/pool_store_capacity > MEM_POOL_STORE_FILL_FACTOR) {
        new_pool_store_capacity = MEM_POOL_STORE_EXPAND_FACTOR * pool_store_capacity;
        new_pool_store = (pool_mgr_pt) realloc(pool_store, new_pool_store_capacity);

        if (new_pool_store != NULL) {
            *pool_store = new_pool_store;
            pool_store_capacity = new_pool_store_capacity;
            new_alloc_status = ALLOC_OK;
        }
    }
    else {
        new_alloc_status = ALLOC_OK;
    }

    return new_alloc_status;

}

static alloc_status _mem_resize_node_heap(pool_mgr_pt pool_mgr) {
    // see above

    unsigned new_node_total;
    node_pt new_node_heap;
    alloc_status new_resize_status = ALLOC_FAIL;

    if((float)pool_mgr->used_nodes/pool_mgr->total_nodes
       > MEM_NODE_HEAP_FILL_FACTOR) {
        new_node_total = pool_mgr->total_nodes
                         * MEM_NODE_HEAP_EXPAND_FACTOR;
        new_node_heap = (node_pt)realloc(pool_mgr->node_heap,
                                         new_node_total);

        if(new_node_heap != NULL) {
            pool_mgr->total_nodes = new_node_total;
            pool_mgr->node_heap = new_node_heap;
            new_resize_status = ALLOC_OK;
        }
    }
    else {
        new_resize_status = ALLOC_OK;
    }

    return new_resize_status;
}

static alloc_status _mem_resize_gap_ix(pool_mgr_pt pool_mgr) {
    // see above

    alloc_status new_alloc = ALLOC_FAIL;
    gap_pt new_gap_ix;
    unsigned gap_count = 0;
    unsigned new_gap_capacity;
    gap_pt new_gap = pool_mgr->gap_ix;

    for(unsigned index = 0; index < pool_mgr->gap_ix_capacity; index++) {

        if(new_gap[index].node != NULL && new_gap[index].size != 0) {
            gap_count++;
        }
    }

    if((float)gap_count/pool_mgr->gap_ix_capacity > MEM_GAP_IX_FILL_FACTOR) {
        new_gap_capacity = pool_mgr->gap_ix_capacity * MEM_GAP_IX_EXPAND_FACTOR;
        new_gap_ix = (gap_pt) realloc(pool_mgr->gap_ix, new_gap_capacity);

        if (new_gap_ix != NULL) {
            pool_mgr->gap_ix_capacity = new_gap_capacity;
            pool_mgr->gap_ix = new_gap_ix;
            new_alloc = ALLOC_OK;
        }
    }
    else {
        new_alloc = ALLOC_OK;
    }

    return new_alloc;

}

static alloc_status _mem_add_to_gap_ix(pool_mgr_pt pool_mgr,
                                       size_t size,
                                       node_pt node) {

    // expand the gap index, if necessary (call the function)
    // add the entry at the end
    // update metadata (num_gaps)
    // sort the gap index (call the function)
    // check success

    alloc_status new_status = _mem_resize_gap_ix(pool_mgr);
    int locator = -1;

    if(new_status == ALLOC_OK) {
/* TODO
        int finder = 0;
        while (locator < 0 && finder < pool_mgr->gap_ix_capacity) {

            if (pool_mgr->gap_ix[finder].node == NULL
                && pool_mgr->gap_ix[finder].size == 0) {

                locator = finder;
            }
            finder++;
        }

        if (locator < 0) {
            new_status = ALLOC_FAIL;
        }
        else {
            (pool_mgr->gap_ix)[locator].size = size;
            pool_mgr->gap_ix[locator].node = node;

        }
TODO*/
        (pool_mgr->gap_ix)[pool_mgr->gap_ix_capacity - 1].size = size;
        (pool_mgr->gap_ix)[pool_mgr->gap_ix_capacity - 1].node = node;

        _mem_sort_gap_ix(pool_mgr);
    }

    return new_status;
}

static alloc_status _mem_remove_from_gap_ix(pool_mgr_pt pool_mgr,
                                            size_t size,
                                            node_pt node) {
    // find the position of the node in the gap index
    // loop from there to the end of the array:
    //    pull the entries (i.e. copy over) one position up
    //    this effectively deletes the chosen node
    // update metadata (num_gaps)
    // zero out the element at position num_gaps!

    gap_pt new_gap = pool_mgr->gap_ix;
    int location = -1;

    for(unsigned index = 0; index < pool_mgr->gap_ix_capacity; index++) {

        if(new_gap[index].size == size && ((new_gap[index].node)->alloc_record).mem == (node->alloc_record).mem)  {
            location = index;
            break;
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

        return ALLOC_OK;
    }
}

// note: only called by _mem_add_to_gap_ix, which appends a single entry
static alloc_status _mem_sort_gap_ix(pool_mgr_pt pool_mgr) {
    // the new entry is at the end, so "bubble it up"
    // loop from num_gaps - 1 until but not including 0:
    //    if the size of the current entry is less than the previous (u - 1)
    //    or if the sizes are the same but the current entry points to a
    //    node with a lower address of pool allocation address (mem)
    //       swap them (by copying) (remember to use a temporary variable)

    gap_pt new_gap = pool_mgr->gap_ix;
    gap_t temp_gap;

    if(new_gap) {
        for (unsigned index = pool_mgr->gap_ix_capacity-1; index > 0; index--) {
            // If: lower gap is smaller, and not zero
            // If: higher gap is greater, and lower is zero
            // If  gaps are the same and address of higher gap is less
            // Then: switch gaps
            if( (new_gap[index].size < new_gap[index - 1].size
                 && new_gap[index].size != 0 )
                || (new_gap[index].size > new_gap[index - 1].size
                    && new_gap[index-1].size == 0 )
                || (new_gap[index].size == new_gap[index-1].size
                    && &((new_gap[index].node)->alloc_record).mem < &((new_gap[index-1].node)->alloc_record).mem ) ) {
                temp_gap = new_gap[index];
                new_gap[index] = new_gap[index - 1];
                new_gap[index - 1] = temp_gap;
            }
        }

        return ALLOC_OK;
    }
    else {
        return ALLOC_FAIL;
    }
}
