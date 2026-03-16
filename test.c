#include "ds.h"

#include <stdio.h>

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define RESET "\033[0m"

#define PASS NULL
#define TEST(fn) const char *test_##fn(void)
#define DOTEST(fn) (msg = test_##fn()) ? printf(RED "FAIL" RESET " (" #fn "): %s\n", msg) \
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
    ds_alloc = ds_malloc;

    ds_darray array = ds_new_darray(4, 2, NULL);
    int one = 1;
    int success = ds_darray_push(&array, &one, NULL);
    if(!success) return "Push returned 0, indicating failure.";
    if(array.length != 1) return "Did not increment array length.";
    if(array.size != 4) return "Incorrectly changed array element size.";
    if(array.capacity != 2) return "Incorrectly changed array capacity.";
    if(DS_DARRAY_AS(array, int)[0] != 1) return "Value copied over is incorrect.";
    int two = 2;
    success = ds_darray_push(&array, &two, NULL);
    if(!success) return "Push returned 0, indicating failure.";
    if(array.length != 2) return "Did not increment array length.";
    if(DS_DARRAY_AS(array, int)[0] != 1) return "Incorrectly changed first value.";
    if(DS_DARRAY_AS(array, int)[1] != 2) return "Second value copied over is incorrect.";
    ds_delete_darray(&array, NULL);
    return PASS;
}

TEST(ds_darray_at) {
    ds_alloc = ds_malloc;

    ds_darray array = ds_new_darray(4, 2, NULL);
    int one = 1;
    int two = 2;
    int success = ds_darray_push(&array, &one, NULL);
    success = ds_darray_push(&array, &two, NULL);
    int *val = ds_darray_at(array, 1);
    if(!val) return "Wasn't able to get 2nd element of array with 2 elements.";
    if(*val != 2) return "Returned pointer to incorrect value.";
    int *val_null = ds_darray_at(array, 3);
    if(val_null) return "Incorrectly returned pointer for index > length.";
    ds_delete_darray(&array, NULL);
    return PASS;
}

uint64_t inthash(void *e) {
    return (uint64_t)*(int*)e;
}

int intcmp(void *l, void *r) {
    return *(int*)l - *(int*)r;
}

TEST(ds_new_hashset) {
    ds_alloc = ds_malloc;

    ds_hashset hs = ds_new_hashset(sizeof(int), 8, inthash, intcmp, NULL);
    if(hs.data.data == NULL) return "Did not allocate data array.";
    if(hs.data.size != sizeof(int)) return "Array size not size of int.";
    if(hs.data.length != 0) return "Initial set doesn't have length 0.";
    if(hs.data.capacity != 8) return "Initial set doesn't have length 0.";
    if(hs.hashes == NULL) return "Didn't allocate space for hashes.";
    if(hs.hash != inthash) return "Didn't save correct hash function.";
    if(hs.cmp != intcmp) return "Didn't save correct cmp function.";
    return PASS;
}

TEST(ds_delete_hashset) {
    ds_alloc = ds_malloc;

    ds_hashset hs = ds_new_hashset(sizeof(int), 8, inthash, intcmp, NULL);
    if(ds_delete_hashset(&hs, NULL) != NULL)
        return "Delete hashset did not return NULL.";
    if(hs.data.data != NULL) return "Hashset array not set to NULL.";
    if(hs.data.size != 0) return "Hashset data size not 0 after deletion.";
    if(hs.data.capacity != 0) return "Hashset data capacity not 0 after deletion.";
    if(hs.data.length != 0) return "Hashset data length not 0 after deletion.";
    if(hs.hashes != NULL) return "Hashset hashes not NULL after deletion.";
    return PASS;
}

TEST(ds_hashset_insert) {
    ds_alloc = ds_malloc;

    ds_hashset hs = ds_new_hashset(sizeof(int), 8, inthash, intcmp, NULL);
    int vals[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    size_t n = sizeof(vals)/sizeof(vals[0]);
    for(size_t i = 0; i < n; ++i) {
        void *v = ds_hashset_insert(&hs, &vals[i], NULL);
        if(v == NULL)
            return "Insertion failed.";
        if(*(int*)v != vals[i])
            return "Returned value from insert not equal to value passed in.";
        if(hs.data.length != i+1)
            return "Length of hashset not increasing with insertion.";
    }
    if(hs.data.capacity <= 8)
        return "Capacity of hashset did not increase even though we inserted more elements than initial allocation.";
    ds_delete_hashset(&hs, NULL);
    return PASS;
}

TEST(ds_hashset_contains) {
    ds_alloc = ds_malloc;

    ds_hashset hs = ds_new_hashset(sizeof(int), 8, inthash, intcmp, NULL);
    int vals[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    size_t n = sizeof(vals)/sizeof(vals[0]);
    for(size_t i = 0; i < n; ++i) {
        ds_hashset_insert(&hs, &vals[i], NULL);
        if(!ds_hashset_contains(&hs, &vals[i]))
            return "Hashset does not contain element we just inserted?";
    }
    for(size_t i = 0; i < n; ++i) {
        if(!ds_hashset_contains(&hs, &vals[i]))
            return "Hashset does not contain element we inserted previously.";
    }

    ds_delete_hashset(&hs, NULL);
    return PASS;
}

TEST(ds_hashset_remove) {
    ds_alloc = ds_malloc;

    ds_hashset hs = ds_new_hashset(sizeof(int), 8, inthash, intcmp, NULL);
    int vals[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    size_t n = sizeof(vals)/sizeof(vals[0]);
    for(size_t i = 0; i < n; ++i) {
        void *el = ds_hashset_insert(&hs, &vals[i], NULL);
    }
    for(size_t i = 0; i < n; ++i) {
        ds_hashset_remove(&hs, &vals[i]);
        if(ds_hashset_contains(&hs, &vals[i]))
            return "Hashset contains element we just removed?";
    }

    ds_delete_hashset(&hs, NULL);
    return PASS;
}

int main(int argc, char **argv) {
    const char *msg;

    // Array
    DOTEST(ds_malloc);
    DOTEST(ds_new_darray);
    DOTEST(ds_delete_darray);
    DOTEST(ds_darray_resize);
    DOTEST(ds_darray_push);
    DOTEST(ds_darray_at);

    // Hash Set
    DOTEST(ds_new_hashset);
    DOTEST(ds_delete_hashset);
    DOTEST(ds_hashset_insert);
    DOTEST(ds_hashset_contains);
    DOTEST(ds_hashset_remove);

    return 0;
}

