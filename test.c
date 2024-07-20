#include "ds.h"

#include <stdio.h>

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define RESET "\033[0m"

#define PASS NULL
#define TEST(fn) const char *test_##fn(void)
#define DOTEST(fn) (msg = test_##fn()) ? printf(RED "FAIL" RESET " (" #fn "): %s", msg) \
			 : puts(GREEN "PASS" RESET " (" #fn ")")

TEST(ds_malloc) {
    ds_alloc = ds_malloc;

    int *tmp = ds_alloc(NULL, NULL, 0, 4);
    if(!tmp) return "Failed to allocate memory.";
    *tmp = 4;
    tmp = ds_alloc(NULL, tmp, 4, 8);
    if(!tmp) return "Failed to reallocate memory.";
    if(*tmp != 4) return "Failed to copy existing data when reallocating.";
    tmp = ds_alloc(NULL, tmp, 8, 0);
    if(tmp) return "Failed to free memory or failed to return NULL after freeing memory.";

    return PASS;
}

TEST(ds_new_darray) {
    ds_alloc = ds_malloc;

    ds_darray array = ds_new_darray(4, 2, NULL);
    if(array.length != 0) return "Set the wrong length.";
    if(array.capacity != 2) return "Set the wrong capacity.";
    if(array.size != 4) return "Set the wrong size.";

    ds_delete_darray(&array, NULL);
    return PASS;
}

TEST(ds_delete_darray) {
    ds_alloc = ds_malloc;

    ds_darray array = ds_new_darray(4, 2, NULL);
    void *ret = ds_delete_darray(&array, NULL);
    if(ret) return "Failed to deallocate array data.";
    if(array.length) return "Failed to set length to 0.";
    if(array.capacity) return "Failed to set capacity to 0.";
    if(array.size) return "Failed to set size to 0.";
    if(array.data) return "Failed to set data to NULL.";
    return PASS;
}

TEST(ds_darray_resize) {
    ds_alloc = ds_malloc;

    ds_darray array = ds_new_darray(4, 2, NULL);
    DS_DARRAY_AS(array, int)[0] = 1;
    DS_DARRAY_AS(array, int)[1] = 2;
    array.length = 2;
    size_t newcap = ds_darray_resize(&array, 4, NULL);
    if(newcap == 0) return "Resize failed.";
    if(newcap != 4) return "Resize did not return new capcity.";
    if(array.capacity != 4) return "Did not set capacity to new capacity.";
    if(array.length != 2) return "Did not keep array length the same.";
    if(array.size != 4) return "Did not keep array element size the same.";
    if(DS_DARRAY_AS(array, int)[0] != 1 || DS_DARRAY_AS(array, int)[1] != 2)
        return "Resize did not copy over old values.";
    ds_delete_darray(&array, NULL);
    return PASS;
}

TEST(ds_darray_push) {
    return PASS;
}

TEST(ds_darray_at) {
    return PASS;
}

int main(int argc, char **argv) {
    const char *msg;
    DOTEST(ds_malloc);
    DOTEST(ds_new_darray);
    DOTEST(ds_delete_darray);
    DOTEST(ds_darray_resize);
    DOTEST(ds_darray_push);
    DOTEST(ds_darray_at);

    return 0;
}
