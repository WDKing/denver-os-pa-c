/* Authors: William King & Ivo Georgiev */

#include <stdio.h>
#include <assert.h>

#include "mem_pool.h"

int main(int argc, char *argv[]) {

    printf("Testing C Language Programming Assignment\n");
printf("Testing\n");
    /*printf("Test suite will be published soon. You will have to update your main.c file.\n");

    /* Personal & Pre-change Testing Code */

    /* printf("%ld\n", __STDC_VERSION__); */
    const unsigned POOL_SIZE = 1000000;
    pool_pt pool = NULL;

    /* Test mem_init */
printf("Before mem_init\n")    ;

    alloc_status status = mem_init();
printf("after mem_init\n");
    assert(status == ALLOC_OK);
printf("After status allocation\n");
if(status == ALLOC_OK){printf("Alloc_ok.\n");}

    /* Test mem_pool_open */
    pool = mem_pool_open(POOL_SIZE, FIRST_FIT); // TODO add policy to pool
    assert(pool);

    /* Test mem_new_alloc */
    alloc_pt alloc1 = mem_new_alloc(pool, POOL_SIZE);
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