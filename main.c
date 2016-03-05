/* Authors: William King & Ivo Georgiev */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "mem_pool.h"

/* forward declarations */
void print_pool(pool_pt pool);

/* main */
int main(int argc, char *argv[]) {

    const unsigned POOL_SIZE = 1000000;
    pool_pt pool = NULL;

/*
pool_pt pool2 = NULL;
mem_init();
    pool2 = mem_pool_open(POOL_SIZE,FIRST_FIT);
    print_pool(pool2);
    alloc_pt alloca1 = mem_new_alloc(pool2, 200);
    print_pool(pool2);
    alloc_pt alloca2 = mem_new_alloc(pool2, 2000);
    print_pool(pool2);
    alloc_pt alloca3 = mem_new_alloc(pool2, 3000);
    print_pool(pool2);
    alloc_pt alloca4 = mem_new_alloc(pool2, 74500);
    print_pool(pool2);
printf("main, in between adds and deletes.\n");
printf("main, de-allocation 1: %u\n",(unsigned)alloca1->size);
    mem_del_alloc(pool2,alloca1);
    print_pool(pool2);
printf("main, de-allocation 3: %u\n",(unsigned)alloca3->size);
    mem_del_alloc(pool2,alloca3);
    print_pool(pool2);
printf("main, de-allocation 4: %u\n",(unsigned)alloca4->size);
    mem_del_alloc(pool2,alloca4);
    print_pool(pool2);
printf("main, de-allocation 2: %u\n",(unsigned)alloca2->size);
    mem_del_alloc(pool2,alloca2);
    print_pool(pool2);

    mem_pool_close(pool2);
mem_free();
*/







    alloc_status status = mem_init();
    assert(status == ALLOC_OK);
//printf("main.\nPool_size: %u\n\n",POOL_SIZE);
    pool = mem_pool_open(POOL_SIZE, FIRST_FIT);
    assert(pool);

    /*
     * Basic allocation scenario:
     *
     * 1. Pool starts out as a single gap.
     * 2. Allocate 100. That will be at the top. The rest is a gap.
     * 3. Allocate 1000. That will be underneath it. The rest is a gap.
     * 4. Deallocate the 100 allocation. Now gaps on both sides of 1000.
     * 4. Deallocate the 1000 allocation. Pool is again one single gap.
     */


    print_pool(pool);
printf("main:: pre-alloc series.\n");//todo
    // + alloc-0
    alloc_pt alloc0 = mem_new_alloc(pool, 100);
printf("main:: alloc0->size: %u.\n",
       (unsigned)alloc0->size);//todo
    assert(alloc0);

    print_pool(pool);

    // + alloc-1
    alloc_pt alloc1 = mem_new_alloc(pool, 1000);
    assert(alloc1);
printf("main:: alloc1->size: %u\n",(unsigned)alloc1->size);//todo
    print_pool(pool);

    // - alloc-0
    status = mem_del_alloc(pool, alloc0);
    assert(status == ALLOC_OK);

    print_pool(pool);

    // - alloc-1
    status = mem_del_alloc(pool, alloc1);
    assert(status == ALLOC_OK);

    print_pool(pool);


    /*
     * End of allocation scenario. Clean up.
     */


    status = mem_pool_close(pool);
    assert(status == ALLOC_OK);
    status = mem_free();

    printf("status = %d\n", status);
    assert(status == ALLOC_OK);

    /* Personal & Pre-change Testing Code */

    printf("%ld\n", __STDC_VERSION__);
    pool;

    /* Test mem_init */
    status = mem_init();
    assert(status == ALLOC_OK);

    /* Test mem_pool_open */
    pool = mem_pool_open(POOL_SIZE, FIRST_FIT);
    assert(pool);

    /* Test mem_new_alloc */
    alloc1 = mem_new_alloc(pool, POOL_SIZE);
    assert(alloc1);
    alloc_pt alloc2 = mem_new_alloc(pool, POOL_SIZE);
    assert(!alloc2);

    /* Test mem_delete_alloc */
    status = mem_del_alloc(pool, alloc1);
    assert(status == ALLOC_OK);

    /* Test mem_pool_close */
    status = mem_pool_close(pool);
    assert(status == ALLOC_OK);

    /* Test mem_free */
    status = mem_free();
    assert(status == ALLOC_OK);

    return 0;
}

/* function definitions */
void print_pool(pool_pt pool) {
    pool_segment_pt segs = NULL;
    unsigned size = 0;
    assert(pool);
printf("Enter print_pool.\n");
    mem_inspect_pool(pool, &segs, &size);

    assert(segs);
    assert(size);
    for (unsigned u = 0; u < size; u++){
    printf("%10lu - %s\n", (unsigned long) segs[u].size, (segs[u].allocated) ? "alloc" : "gap"); }
printf("print_pool, post printing.");

    free(segs);
printf("post free segs\n");
    printf("\n");
}
