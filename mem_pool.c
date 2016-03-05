/*
 * School: Metropolitan State University of Denver
 * Semester: Spring 2016
 * Course: Operating Systems
 * Programming Assignment 1 
 * Created by Ivo Georgiev on 2/9/16.
 * Modified by William King
 * Version: 05March16-1618
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
//        unsigned pool_store_element_size =

        //unsigned pool_store_element_size =
        //        sizeof(char) /* pool: mem */
        //        + sizeof(alloc_policy) /* pool: policy */
        //        + sizeof(size_t) /* pool: total_size */
        //        + sizeof(size_t) /* pool: alloc_size */
        //        + sizeof(unsigned) /* pool: num_allocs */
        //        + sizeof(unsigned) /* pool: num_gaps */
        //        + sizeof(alloc_t)  /* node: alloc_record */
        //        + sizeof(unsigned) /* node: used */
        //        + sizeof(unsigned) /* node: allocated */
        //        + sizeof(node_pt) /* node: next */
        //        + sizeof(node_pt) /* node: prev */
        //        + sizeof(size_t) /* gap: size */
        //        + sizeof(node_pt) /* gap: node */
        //        + sizeof(unsigned) /* pool_mgr: total_nodes */
        //        + sizeof(unsigned) /* pool_mgr: used_nodes */
        //        + sizeof(unsigned);  /* pool_mgr: gap_ix_capacity */

        //pool_store_capacity = MEM_POOL_STORE_INIT_CAPACITY * pool_store_element_size;
        //pool_store = (pool_mgr_pt *)calloc(pool_store_capacity,pool_store_element_size);

        pool_store_capacity = MEM_POOL_STORE_INIT_CAPACITY;
        pool_store = (pool_mgr_pt *)calloc(pool_store_capacity,sizeof(pool_mgr_t));

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
    pool_pt new_pool = NULL;
    node_pt new_node_heap = NULL;
    gap_pt new_gap_pt = NULL;
//printf("Enter mem_pool_open:: size: %u\n",(unsigned)size); //TODO
    if(pool_store != NULL) {

        _mem_resize_pool_store();

        new_pool_mgr = (pool_mgr_pt)calloc(1,sizeof(pool_mgr_t));

        if(new_pool_mgr) {

            new_pool = (pool_pt)calloc(size,sizeof(pool_t));

            if (new_pool) {
                new_node_heap = (node_pt)calloc(MEM_NODE_HEAP_INIT_CAPACITY,sizeof(node_t));

                if(new_node_heap) {
                    new_gap_pt = (gap_pt)calloc(MEM_GAP_IX_INIT_CAPACITY,sizeof(gap_t));

                    if(new_gap_pt) {
                        new_pool_mgr->total_nodes = MEM_NODE_HEAP_INIT_CAPACITY;
                        new_pool_mgr->used_nodes = 1;

                        new_pool_mgr->gap_ix = new_gap_pt;
                        ((alloc_pt)new_node_heap)->size = size;
                        ((node_pt)new_node_heap)->used = 1;
                        ((node_pt)new_node_heap)->allocated = 0;
                        ((alloc_pt)new_node_heap)->size = size;
                        new_pool_mgr->node_heap = new_node_heap;

                        new_pool = (pool_pt)new_pool_mgr;
                        new_pool->policy = policy;
                        new_pool->num_allocs = 0;
                        new_pool->num_gaps = 1;
                        new_pool->total_size = size;
                        new_pool->alloc_size = 0;
                        ((alloc_pt)new_pool)->size = size;
//printf("mem_pool_open, Mark B::  ((gap_pt)new_pool)->size: %u\n",
//                     (unsigned)((gap_pt)new_pool)->size); //TODO
//TODO                        ((pool_pt)new_pool_mgr)->mem = (char *)new_pool;
                        new_pool_mgr->gap_ix_capacity = MEM_GAP_IX_INIT_CAPACITY;
                        _mem_add_to_gap_ix(new_pool_mgr,
                                           size,
                                           (node_pt)new_pool);

//printf("Gap after add: %u\n",(unsigned)((gap_pt)new_pool_mgr->gap_ix)[0].size);
                        //(new_pool_mgr->gap_ix)->node = ((node_pt)(new_pool));
                        //((gap_pt)new_pool)->size = size;

                        pool_store[pool_store_size] = new_pool_mgr;
// TODO
//printf("mem_pool_open, Mark C.  Size: %u, size of pool: %u, gap_pt: %u\n",
//       (unsigned)size,
//       (unsigned)new_pool->total_size,
//       (unsigned)((gap_pt)new_pool)->size);
//TODO
//printf("mem_pool_open, Mark D.  Used Nodes, from pool: %u, from pool_mgr: %u\n",
//       ((pool_mgr_pt)new_pool)->used_nodes,
//       new_pool_mgr->used_nodes);

                    }
                    else {
                        free(new_node_heap);
                        free(new_pool);
                        free(new_pool_mgr);
                    }
                }
                else {
                    free(new_pool);
                    free(new_pool_mgr);
                }
            }
            else {
                free(new_pool_mgr);
            }
        }
    }
//TODO
//printf("mem_pool_open, exit:: ((gap_pt)new_pool)->size: %u\n",(unsigned)((gap_pt)new_pool)->size);
    return new_pool;
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
    // free mgr


    alloc_status free_pool_status = ALLOC_FAIL;
    pool_mgr_pt new_pool_mgr = (pool_mgr_pt)pool;
    unsigned count = 0;


    assert(pool);
    if(new_pool_mgr != NULL) {
        for(unsigned index = 0;
                index < new_pool_mgr->gap_ix_capacity;
                index++) {
            if( (new_pool_mgr->gap_ix)[index].size > 0) {
                count++;
            }
        }

        if( count == 1) {
            if( pool->num_allocs == 0 ) {
                free( new_pool_mgr->node_heap );
                free( new_pool_mgr->gap_ix );
                free( pool );

                for(unsigned tally = 0; tally < pool_store_capacity; tally++) {
                    if( pool_store[tally] == new_pool_mgr ) {
                        pool_store[tally] == NULL;
                    }
                }

                free(new_pool_mgr);
            }
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
    pool_mgr_pt new_pool_mgr = (pool_mgr_pt)pool;
    node_pt working_node = NULL, next_node = NULL;
    node_pt unused_node_locator = NULL;
    gap_pt next_gap = NULL;
    size_t leftover_alloc_size = 0;
    int tally, locator;

//printf("Enter mem_new_alloc.  Size of gap pool: %u.\n",
//       (unsigned)(((gap_pt)(new_pool_mgr->gap_ix))->size));  // TODO
    assert((((gap_pt)(new_pool_mgr->gap_ix))->size) > 0);
    if( ((gap_pt)(new_pool_mgr->gap_ix))->size > 0 ) {

        if( _mem_resize_node_heap(new_pool_mgr) == ALLOC_FAIL) {
            return new_alloc_pt;
        }
//printf("mem_new_alloc, mark A\n");  //TODO
        if( new_pool_mgr->used_nodes >= new_pool_mgr->total_nodes ) {
            return new_alloc_pt;
        }
//printf("mem_new_alloc, mark B\n"); //TODO

        /*  FIRST_FIT replacement policy */
        if(pool->policy == FIRST_FIT) {

//printf("mem_new_alloc, mark FIRST_FIT\n"); //TODO
            next_node = (node_pt)new_pool_mgr->node_heap;

            /* Search for first node that is used, but not allocated */
            while(working_node == NULL && next_node != NULL) {
//printf("mem_new_alloc, while-search for gap:: next_node->allocated: %u\n",next_node->allocated);//TODO
                if(next_node->allocated == 0 && ((alloc_pt)next_node)->size > size) {
                    working_node = next_node;
                }

                next_node = next_node->next;
            }
        }
        else { /* BEST_FIT */
//printf("mem_new_alloc, mark BEST_FIT\n"); //TODO
            //TODO not working
//            next_gap = new_pool_mgr->gap_ix;
//            tally = 0;

//            while(tally < new_pool_mgr->gap_ix_capacity && working_node == NULL) {
//                if(next_gap->size > size) {
//                    working_node = (node_pt)next_gap;
//                }

//                tally++;
//            }
        }
//printf("mem_new_alloc, mark C\n"); //TODO

        /* No valid node found */
        assert(working_node);
        if(working_node == NULL) {
            return new_alloc_pt;
        }
// TODO
//printf("mem_new_alloc:: ((alloc_pt)working_node)->size: %u, alloc: %u.\n",
//       (unsigned)((alloc_pt)working_node)->size,
//       working_node->allocated);
        /* Calculate differrence between found gap and desired size */
        leftover_alloc_size = ((alloc_pt)working_node)->size - size;
        /* Change current node to allocated */
        working_node->allocated = 1;
//printf("mem_new_alloc, new alloc test:: alloc: %u\n",working_node->allocated);
        /* Increase total allocation count */
        ((pool_pt)new_pool_mgr)->num_allocs += 1;
        /* Increase allocated size */
        ((pool_pt)new_pool_mgr)->alloc_size += size;

//TODO        new_pool_mgr->used_nodes += 1;

        /* Remove previous gap from gap pool */
        _mem_remove_from_gap_ix(new_pool_mgr,((alloc_pt)working_node)->size,working_node);

        /* Desired size matches size of located gap */
        if(leftover_alloc_size == 0) {
//printf("mem_new_alloc, leftover_alloc_size == 0"); //TODO
//            ((pool_pt)new_pool_mgr)->num_gaps -= 1;
//            new_alloc_pt = (alloc_pt)working_node;
        }
        /* Desired size is smaller than located gap */
        else {
//printf("mem_new_alloc, lefotver_alloc: %u, size: %u\n",
//       (unsigned)leftover_alloc_size,
//       (unsigned)size); //TODO
            /* Set desired size into located node */
            ((alloc_pt)working_node)->size = size;
            /* increase total of used nodes */
            new_pool_mgr->used_nodes += 1;
//printf("mem_new mark 1\n"); //TODO



            tally = 0;
            unused_node_locator = ((node_pt)(new_pool_mgr->node_heap));

//printf("mem_new mark 2\n"); //TODO
            while(unused_node_locator->used == 1
                  && tally < new_pool_mgr->total_nodes) {
//printf("mem_new_alloc search while:: unused_node_locator->used: %u, size: %u, alloc: %u\n",
//                       unused_node_locator->used,
//                      ((alloc_pt)unused_node_locator)->size,
//                      unused_node_locator->allocated);//TODO
//TODO                if(((node_pt)(new_pool_mgr->node_heap))[tally].used == 0) {
//TODO                    locator = tally;
//TODO                    break;


                unused_node_locator = unused_node_locator + sizeof(unused_node_locator);
                tally++;
            }
//printf("mem_new mark 3:: unused_node_locator->used: %u, size: %u, alloc: %u, tally: %u\n",
//       unused_node_locator->used,
//       ((alloc_pt)unused_node_locator)->size,
//       unused_node_locator->allocated,
//         tally); //TODO


            assert(unused_node_locator->used == 0);
            assert(tally < new_pool_mgr->total_nodes);
            /* Did not find unused node */
            if(unused_node_locator->used == 1) {
                return new_alloc_pt;
            }
//printf("mem_new mark 4\n"); //TODO


            /* Increase pool gap count */
            ((pool_pt)new_pool_mgr)->num_gaps += 1;
//printf("mem_new mark 4.1\n"); //TODO
            /* Ensure that allocated status is not allocated */
            unused_node_locator->allocated = 0;
            /* Set used status to used */
            unused_node_locator->used = 1;
            /* Set size of located node to remaining size */
            ((alloc_pt)unused_node_locator)->size = leftover_alloc_size;

//printf("mem_new mark 4.2:: ((alloc_pt)(unused_node_lcoator)->size: %u\n",
//       (unsigned)((alloc_pt)unused_node_locator)->size); //TODO
//            /* Insert node between old allocated node, and next node in list */
            unused_node_locator->next = working_node->next;
//printf("mem_new mark 4.3\n"); //TODO
            if(working_node->next != NULL) {
                (working_node->next)->prev = unused_node_locator;
            }
            else {
                unused_node_locator->next = NULL;
            }
//printf("mem_new mark 4.4\n"); //TODO
            working_node->next = unused_node_locator;
//printf("mem_new mark 4.5\n"); //TODO
            unused_node_locator->prev = working_node;
//printf("mem_add mark 5\n"); //TODO
            if(_mem_add_to_gap_ix(new_pool_mgr,
                                  ((gap_pt)unused_node_locator)->size,
                                  unused_node_locator)
               != ALLOC_FAIL) {
//printf("mem_add_alloc, assigning working node. Sizes: %u, %u\n",
//       (unsigned)((alloc_pt)working_node)->size,
//       (unsigned)((alloc_pt)(working_node->next))->size); //TODO

                new_alloc_pt = (alloc_pt)working_node;
            }
        }



        return new_alloc_pt;
    }
    else {
//printf("mem_new_alloc, else gap index not greater than 0.");
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
    /* Node to find */
    node_pt node_to_find;
    /* Boolean to determine if node was found */
    int found = 0;
    size_t working_size;


    assert(working_node);
    if(alloc != NULL && pool != NULL) {
        while (working_node != NULL) {
            assert(working_node != NULL);
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

                    if (_mem_remove_from_gap_ix((pool_mgr_pt) pool,
                                                ((gap_pt) (working_node->next))->size,
                                                working_node->next) == ALLOC_OK) {
                        ((gap_pt) working_node)->size = ((gap_pt) working_node)->size
                                                        + working_size;
                        (working_node->next)->used = 0;
                        (working_node->next)->allocated = 0;
                        new_pool_mgr->used_nodes = new_pool_mgr->used_nodes - 1;
                        ((pool_pt) new_pool_mgr)->num_gaps -= 1;

                        if (((working_node)->next)->next != NULL) {
                            (((working_node)->next)->next)->prev = working_node;
                            ((working_node->next)->next) = NULL;
                            ((working_node->next)->prev) = NULL;
                            (working_node)->next = ((working_node->next)->next);
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
                                               ((gap_pt)working_node)->size,
                                               working_node)) {
                        ((alloc_pt)(working_node->prev))->size += working_size;
                        working_node->used = 0;
                        working_node->allocated = 0;
                        new_pool_mgr->used_nodes = new_pool_mgr->used_nodes - 1;
                        ((pool_pt) new_pool_mgr)->num_gaps -= 1;

                        if(working_node->next != NULL) {
                            working_node = working_node->prev;

                            ((working_node->next)->next)->prev = working_node;
                            (working_node->next)->prev = NULL;
                            (working_node->next)->next = NULL;
                            working_node->next = (working_node->next)->next;

                        }
                        else {
                            working_node = working_node->prev;
                            working_node->next = NULL;
                        }

/*                    working_node = working_node->prev;

                    working_size = ((gap_pt) (working_node->next))->size;

                    if (_mem_remove_from_gap_ix((pool_mgr_pt) pool,
                                                ((gap_pt) (working_node->next))->size,
                                                working_node->next) == ALLOC_OK) {
                        ((gap_pt) working_node)->size = ((gap_pt) working_node)->size
                                                        + working_size;
                        (working_node->next)->used = 0;
                        new_pool_mgr->used_nodes = new_pool_mgr->used_nodes - 1;
                        ((pool_pt) new_pool_mgr)->num_gaps -= 1;

                        if (((working_node)->next)->next != NULL) {
                            (((working_node)->next)->next)->prev = working_node;
                            ((working_node->next)->next) = NULL;
                            ((working_node->next)->prev) = working_node;
                            (working_node)->next = ((working_node->next)->next);
                        }
                        else {
                            working_node->next = NULL;
                        }
                    } */
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

//    pool_mgr_pt new_pool_mgr = (pool_mgr_pt)pool;

    pool_mgr_pt new_pool_mgr = (pool_mgr_pt)pool;
    *num_segments = new_pool_mgr->used_nodes;
// TODO
//printf("mem_inspect_pool: *num_segments: %u, total_size: %u %u, alloc_size: %u, num_allocs: %u \n",
//       *num_segments,
//       (unsigned)pool->total_size,
//       (unsigned)((pool_pt)new_pool_mgr)->total_size,
//       (unsigned)pool->alloc_size,
//       pool->num_allocs); //TODO
    assert(*num_segments);
    node_pt working_node = new_pool_mgr->node_heap;

    *segments = (pool_segment_pt) calloc(((pool_mgr_pt)pool)->used_nodes, sizeof(pool_segment_t));

    assert(*segments);
    if (*segments != NULL) {

        for (unsigned index = 0; index < new_pool_mgr->used_nodes; index++) {
printf("inspect_pool - Pre, segment for loop:: index: %u, w_n: %s, alloc: %s, size: %u.\n",
         index,
         (working_node)?"Not NULL":"NULL",
       (working_node->allocated == 0)?"Not allocated":"Allocated",
       (unsigned)((alloc_pt)working_node)->size);
            assert(segments[index]);
            (*segments)[index].allocated = working_node->allocated;
            (*segments)[index].size = ((alloc_pt)working_node)->size;
//TODO
printf("inspect_pool - post, for loop:: index: %u, allocated: %u %u, size: %u %u\n",
       index,
       segments[index]->allocated,
       working_node->allocated,
       (unsigned)segments[index]->size,
       (unsigned)((alloc_pt)working_node)->size);

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
    alloc_status new_status = ALLOC_FAIL;

    if((float)pool_mgr->used_nodes/pool_mgr->total_nodes
            > MEM_NODE_HEAP_FILL_FACTOR) {
        new_node_total = pool_mgr->total_nodes
                         * MEM_NODE_HEAP_EXPAND_FACTOR;
        new_node_heap = (node_pt)realloc(pool_mgr->node_heap,
                                         new_node_total);

        if(new_node_heap != NULL) {
            pool_mgr->total_nodes = new_node_total;
            pool_mgr->node_heap = new_node_heap;
            new_status = ALLOC_OK;
        }
    }
    else {
        new_status = ALLOC_OK;
    }

    return new_status;
}

static alloc_status _mem_resize_gap_ix(pool_mgr_pt pool_mgr) {
    // see above

//printf("Enter _mem_resize_gap_ix:: first gap: %u\n",(unsigned)((gap_pt)pool_mgr->gap_ix)->size);//TODO
    alloc_status new_alloc = ALLOC_FAIL;
    gap_pt new_gap_ix;
    unsigned gap_count = 0;
    unsigned new_gap_capacity;
    gap_pt new_gap = pool_mgr->gap_ix;
//printf("_mem_resize_gap_ix:: gap_ix_capacity: %u\n",pool_mgr->gap_ix_capacity);// TODO
    for(unsigned index = 0; index < pool_mgr->gap_ix_capacity; index++) {
//printf("_mem_resize_gap_ix, track index: %u, gap_count: %u, size: %u\n",
//       index,
//       gap_count,
//       (unsigned)((gap_pt)(pool_mgr->gap_ix))->size); //TODO

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
//printf("Enter mem_add_to_gap_ix:: size to add: %u\n",(unsigned)size); //TODO
    alloc_status new_status = _mem_resize_gap_ix(pool_mgr);
    int locator = -1;
//printf("mem_add_to_gap_ix, mark A: new_status: %s\n",
//       (new_status == ALLOC_OK)?"Alloc_Ok":"Alloc_Fail"); //TODO
    if(new_status == ALLOC_OK) {
        int finder = 0;
        while(locator < 0 && finder < pool_mgr->gap_ix_capacity){

//printf("mem_add_to_gap_ix, finder loop:: finder: %u, locator: %i\n",(unsigned)finder,locator);
//printf("mem_add_go_gap_ix, node: %s, locator: %i\n", //TODO
//       ((pool_mgr->gap_ix)[finder].node == NULL)?"NULL":"Not NULL",
//       locator); //TODO

            if(pool_mgr->gap_ix[finder].node == NULL
                    && pool_mgr->gap_ix[finder].size == 0) {
//printf("mem_add_to_gap_ix, matching locator-finder: %u\n",finder);  //TODO
                locator = finder;
            }
            finder++;
        }
        assert(locator >= 0);
        if(locator < 0) {
            new_status = ALLOC_FAIL;
        }
        else {
            (pool_mgr->gap_ix)[locator].size = size;
            pool_mgr->gap_ix[locator].node = node;

///printf("mem_add_to_gap_ix, locator >= 0:: pool_mgr->gap_ix[locator].size %u, "
//                           "pool_mgr->gap_ix[locator].node\n",
//                   (unsigned)pool_mgr->gap_ix[locator].size ); //TODO
        }

        _mem_sort_gap_ix(pool_mgr);

//printf("mem_add_to_gap_ix, after sort_gap_ix:: pool_mgr->gap_ix[locator].size %u, "
//                       "pool_mgr->gap_ix[locator].node\n",
//       (unsigned)pool_mgr->gap_ix[locator].size ); //TODO
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
        if(new_gap[index].node == node && new_gap[index].size == size) {
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
        for (unsigned index = pool_mgr->gap_ix_capacity-1; index > 0; index--) {
//printf("mem_sort_gap_ix, for loop:: index: %u, gap at index: %u, gap below index: %u\n", //TODO
//        index,
//       (unsigned)new_gap[index].size,
//       (unsigned)new_gap[index-1].size);
            if( (new_gap[index].size < new_gap[index - 1].size
                 && new_gap[index].size != 0 )
                 || (new_gap[index].size > new_gap[index - 1].size
                     && new_gap[index].size == 0 )) {
//printf("mem_sort, truth compare: 1: %s, 2: %s\n", //TODO
//       ((unsigned)(new_gap[index].size) < (unsigned)(new_gap[index - 1].size))?"True":"False",
//       (new_gap[index-1].size == 0)?"True":"False");
                temp_gap = new_gap[index];
                new_gap[index] = new_gap[index - 1];
                new_gap[index - 1] = temp_gap;
//printf("mem_sort_gap_ix, switch if:: index: %u, gap at index: %u, gap below index: %u\n", ??TODO
//                index,
//                (unsigned)new_gap[index].size,
//                (unsigned)new_gap[index-1].size);

            }
        }

        return ALLOC_OK;
    }
    else {
        return ALLOC_FAIL;
    }
}


