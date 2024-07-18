#include "ds.h"

#include <stdio.h>

#define PASS 0
#define FAIL -1

int test_ds_malloc() {
    ds_alloc = ds_malloc;

    int *tmp = ds_alloc(NULL, NULL, 0, 4);
    if(!tmp) return FAIL;
    *tmp = 4;
    tmp = ds_alloc(NULL, tmp, 4, 8);
    if(!tmp || *tmp != 4) return FAIL;
    tmp = ds_alloc(NULL, tmp, 8, 0);
    if(tmp) return FAIL;

    return PASS;
}

int main(int argc, char **argv) {
    if(test_ds_malloc() == FAIL)
        puts("Failed ds_malloc test");
    else
        puts("Passed ds_malloc test");

    return 0;
}
