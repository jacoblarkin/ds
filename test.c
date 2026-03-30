#include "ds.h"

#include <stdio.h>

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define RESET "\033[0m"

#define PASS NULL
#define TESTFN(fn) const char *test_##fn(void)
#define TEST(expr, msg) if(expr) return msg
#define DOTEST(fn) (msg = test_##fn()) ? printf(RED "FAIL" RESET " (" #fn "): %s\n", msg) \
			 : puts(GREEN "PASS" RESET " (" #fn ")")

TESTFN(ds_malloc) {
    ds_alloc = ds_malloc;

    int *tmp = ds_alloc(NULL, NULL, 0, 4);
    TEST(!tmp, "Failed to allocate memory.");
    *tmp = 4;
    tmp = ds_alloc(NULL, tmp, 4, 8);
    TEST(!tmp, "Failed to reallocate memory.");
    TEST(*tmp != 4, "Failed to copy existing data when reallocating.");
    tmp = ds_alloc(NULL, tmp, 8, 0);
    TEST(tmp, "Failed to free memory or failed to return NULL after freeing memory.");

    return PASS;
}

TESTFN(ds_new_darray) {
    ds_alloc = ds_malloc;

    ds_darray array = ds_new_darray(4, 2, NULL);
    TEST(array.length != 0, "Set the wrong length.");
    TEST(array.capacity != 2, "Set the wrong capacity.");
    TEST(array.size != 4, "Set the wrong size.");

    ds_delete_darray(&array, NULL);
    return PASS;
}

TESTFN(ds_delete_darray) {
    ds_alloc = ds_malloc;

    ds_darray array = ds_new_darray(4, 2, NULL);
    void *ret = ds_delete_darray(&array, NULL);
    TEST(ret, "Failed to deallocate array data.");
    TEST(array.length, "Failed to set length to 0.");
    TEST(array.capacity, "Failed to set capacity to 0.");
    TEST(array.size, "Failed to set size to 0.");
    TEST(array.data, "Failed to set data to NULL.");

    return PASS;
}

TESTFN(ds_darray_resize) {
    ds_alloc = ds_malloc;

    ds_darray array = ds_new_darray(4, 2, NULL);
    DS_DARRAY_AS(array, int)[0] = 1;
    DS_DARRAY_AS(array, int)[1] = 2;
    array.length = 2;
    size_t newcap = ds_darray_resize(&array, 4, NULL);
    TEST(newcap == 0, "Resize failed.");
    TEST(newcap != 4, "Resize did not return new capacity.");
    TEST(array.capacity != 4, "Did not set capacity to new capacity.");
    TEST(array.length != 2, "Did not keep array length the same.");
    TEST(array.size != 4, "Did not keep array element size the same.");
    TEST(DS_DARRAY_AS(array, int)[0] != 1 || DS_DARRAY_AS(array, int)[1] != 2,
         "Resize did not copy over old values.");
    ds_delete_darray(&array, NULL);
    return PASS;
}

TESTFN(ds_darray_push) {
    ds_alloc = ds_malloc;

    ds_darray array = ds_new_darray(4, 2, NULL);
    int one = 1;
    int success = ds_darray_push(&array, &one, NULL);
    TEST(!success, "Push returned 0, indicating failure.");
    TEST(array.length != 1, "Did not increment array length.");
    TEST(array.size != 4, "Incorrectly changed array element size.");
    TEST(array.capacity != 2, "Incorrectly changed array capacity.");
    TEST(DS_DARRAY_AS(array, int)[0] != 1, "Value copied over is incorrect.");
    int two = 2;
    success = ds_darray_push(&array, &two, NULL);
    TEST(!success, "Push returned 0, indicating failure.");
    TEST(array.length != 2, "Did not increment array length.");
    TEST(DS_DARRAY_AS(array, int)[0] != 1, "Incorrectly changed first value.");
    TEST(DS_DARRAY_AS(array, int)[1] != 2, "Second value copied over is incorrect.");
    int three = 3;
    success = ds_darray_push(&array, &three, NULL);
    TEST(!success, "Push returned 0, indicating failure.");
    TEST(array.length != 3, "Did not increment array length.");
    TEST(array.capacity == 2, "Did not increase array capacity.");
    TEST(DS_DARRAY_AS(array, int)[0] != 1, "Incorrectly changed first value.");
    TEST(DS_DARRAY_AS(array, int)[1] != 2, "Second value copied over is incorrect.");
    TEST(DS_DARRAY_AS(array, int)[2] != 3, "Third value copied over is incorrect.");
    ds_delete_darray(&array, NULL);
    return PASS;
}

TESTFN(ds_darray_at) {
    ds_alloc = ds_malloc;

    ds_darray array = ds_new_darray(4, 2, NULL);
    int one = 1;
    int two = 2;
    ds_darray_push(&array, &one, NULL);
    ds_darray_push(&array, &two, NULL);
    int *val = ds_darray_at(array, 1);
    TEST(!val, "Wasn't able to get 2nd element of array with 2 elements.");
    TEST(*val != 2, "Returned pointer to incorrect value.");
    int *val_null = ds_darray_at(array, 3);
    TEST(val_null, "Incorrectly returned pointer for index > length.");
    ds_delete_darray(&array, NULL);
    return PASS;
}

TESTFN(ds_darray_concat) {
    ds_alloc = ds_malloc;

    ds_darray array1 = ds_new_darray(sizeof(int), 4, NULL);
    int one[] = { 1, 2, 3, 4 };
    for(size_t i = 0; i < 4; ++i) ds_darray_push(&array1, &one[i], NULL);
    ds_darray array2 = ds_new_darray(sizeof(int), 4, NULL);
    int two[] = { 5, 6, 7, 8 };
    for(size_t i = 0; i < 4; ++i) ds_darray_push(&array2, &two[i], NULL);
    int success = ds_darray_concat(&array1, &array2, NULL);
    TEST(!success, "Concatenation did not succeed.");
    size_t j;
    for(j = 0; j < 4; ++j)
        TEST(*(int*)ds_darray_at(array1, j) != one[j], 
                "Unexpected value in concatenated array.");
    for(j = 0; j < 4; ++j)
        TEST(*(int*)ds_darray_at(array1, j+4) != two[j], 
                "Unexpected value in concatenated array.");

    ds_delete_darray(&array1, NULL);
    ds_delete_darray(&array2, NULL);
    return PASS;
}

TESTFN(ds_darray_pop) {
    ds_alloc = ds_malloc;

    ds_darray array1 = ds_new_darray(sizeof(int), 4, NULL);
    int one[] = { 1, 2, 3, 4 };
    for(size_t i = 0; i < 4; ++i) ds_darray_push(&array1, &one[i], NULL);
    ds_darray_push(&array1, &one[0], NULL);
    int success = ds_darray_pop(&array1, NULL);
    TEST(!success, "Unsuccessful pop.");
    for(size_t i = 0; i < 4; ++i) {
        int v;
        success = ds_darray_pop(&array1, &v);
        TEST(!success, "Unsuccessful pop.");
        TEST(v != one[3-i], "Value popped off is not what we expected.");
        TEST(array1.length != 3-i, "Array length not decremented after pop.");
    }
    TEST(array1.length != 0, "Array is not empty after popping all elements.");
    success = ds_darray_pop(&array1, NULL);
    TEST(success, "Expected pop to fail for empty array.");

    ds_delete_darray(&array1, NULL);
    return PASS;
}

TESTFN(ds_darray_insert) {
    ds_alloc = ds_malloc;

    ds_darray array1 = ds_new_darray(sizeof(int), 4, NULL);
    int one[] = { 1, 2, 3, 4 };
    for(size_t i = 0; i < 4; ++i) ds_darray_push(&array1, &one[i], NULL);
    int five = 5;
    int success = ds_darray_insert(&array1, 2, &five, NULL);
    TEST(!success, "Insertion failed.");
    TEST(array1.length != 5, "Insertion did not increase length.");
    TEST(array1.capacity < 5, "Insertion did not increase capacity even though array was already full.");
    int expect[] = { 1, 2, 5, 3, 4 };
    for(size_t i = 0; i < 5; ++i)
        TEST(*(int*)ds_darray_at(array1, i) != expect[i], "Value in array is not what was expected.");

    ds_delete_darray(&array1, NULL);
    return PASS;
}

TESTFN(ds_darray_insert_array) {
    ds_alloc = ds_malloc;

    ds_darray array1 = ds_new_darray(sizeof(int), 4, NULL);
    int one[] = { 1, 2, 3, 4 };
    for(size_t i = 0; i < 4; ++i) ds_darray_push(&array1, &one[i], NULL);
    ds_darray array2 = ds_new_darray(sizeof(int), 4, NULL);
    int two[] = { 5, 6, 7, 8 };
    for(size_t i = 0; i < 4; ++i) ds_darray_push(&array2, &two[i], NULL);
    int success = ds_darray_insert_array(&array1, 2, &array2, NULL);
    TEST(!success, "Insertion did not succeed.");
    size_t j;
    for(j = 0; j < 2; ++j)
        TEST(*(int*)ds_darray_at(array1, j) != one[j], 
                "Unexpected value in combined array.");
    for(; j < 6; ++j)
        TEST(*(int*)ds_darray_at(array1, j) != two[j-2], 
                "Unexpected value in combined array.");
    for(; j < 8; ++j)
        TEST(*(int*)ds_darray_at(array1, j) != one[j-4], 
                "Unexpected value in combined array.");

    ds_delete_darray(&array1, NULL);
    ds_delete_darray(&array2, NULL);
    return PASS;
}

TESTFN(ds_darray_remove) {
    ds_alloc = ds_malloc;

    ds_darray array1 = ds_new_darray(sizeof(int), 8, NULL);
    int one[] = { 1, 2, 3, 4 };
    for(size_t i = 0; i < 4; ++i) ds_darray_push(&array1, &one[i], NULL);

    int el;
    int ret = ds_darray_remove(&array1, 2, &el);
    TEST(!ret, "Removal unsuccessful.");
    TEST(array1.length != 3, "Expected removal to decrease length.");
    TEST(*(int*)ds_darray_at(array1, 2) != 4, "Expected removal to move elements beyond removal location down.");
    TEST(el != 3, "Expected removal to fill third parameter.");
    ret = ds_darray_remove(&array1, 1, NULL);
    TEST(!ret, "Removal with NULL passed in to third parameter unsuccessful.");
    ret = ds_darray_remove(&array1, 5, NULL);
    TEST(ret, "Should return 0 when index is greater than array length.");

    ds_delete_darray(&array1, NULL);
    return PASS;
}

TESTFN(ds_darray_remove_range) {
    ds_alloc = ds_malloc;

    ds_darray array1 = ds_new_darray(sizeof(int), 8, NULL);
    int one[] = { 1, 2, 3, 4, 5, 6 };
    for(size_t i = 0; i < 6; ++i) ds_darray_push(&array1, &one[i], NULL);
    ds_darray array2 = ds_new_darray(sizeof(int), 1, NULL);
    int ret = ds_darray_remove_range(&array1, 2, 4, &array2, NULL);
    TEST(!ret, "Removal of range unsuccessful.");
    TEST(array1.length != 4, "Did not change length after removing 2 elements.");
    TEST(*(int*)ds_darray_at(array1, 2) != 5, "Did not shift elements after removal.");
    TEST(*(int*)ds_darray_at(array1, 3) != 6, "Did not shift elements after removal.");
    TEST(array2.length != 2, "Length of array filled is not correct.");
    TEST(array2.capacity == 1, "Capacity of array filled did not increase.");
    TEST(*(int*)ds_darray_at(array2, 0) != 3, "Array not filled correctly.");
    TEST(*(int*)ds_darray_at(array2, 1) != 4, "Array not filled correctly.");
    ret = ds_darray_remove_range(&array1, 3, 6, NULL, NULL);
    TEST(ret, "Should return 0 when range is out of bounds for array.");

    ds_delete_darray(&array1, NULL);
    ds_delete_darray(&array2, NULL);
    return PASS;
}

TESTFN(ds_darray_subarray) {
    ds_alloc = ds_malloc;

    ds_darray array1 = ds_new_darray(sizeof(int), 8, NULL);
    int one[] = { 1, 2, 3, 4, 5, 6 };
    for(size_t i = 0; i < 6; ++i) ds_darray_push(&array1, &one[i], NULL);
    ds_darray newarray = ds_darray_subarray(&array1, 2, 4, NULL);
    TEST(newarray.length != 2, "Subarray is not the correct length.");
    TEST(newarray.capacity < 2, "Subarray capacity is too small.");
    TEST(newarray.size != array1.size, "Subarray element size is not correct.");
    TEST(array1.length != 6, "Old array length is changed.");
    TEST(*(int*)ds_darray_at(newarray, 0) != *(int*)ds_darray_at(array1, 2),
            "Elements in subarray do not match old array.");
    TEST(*(int*)ds_darray_at(newarray, 1) != *(int*)ds_darray_at(array1, 3),
            "Elements in subarray do not match old array.");

    ds_delete_darray(&newarray, NULL);
    ds_delete_darray(&array1, NULL);

    return PASS;
}

uint64_t inthash(void *e) {
    return (uint64_t)*(int*)e;
}

int intcmp(void *l, void *r) {
    return *(int*)l - *(int*)r;
}

TESTFN(ds_new_hashset) {
    ds_alloc = ds_malloc;

    ds_hashset hs = ds_new_hashset(sizeof(int), 8, inthash, intcmp, NULL);
    TEST(hs.data.data == NULL, "Did not allocate data array.");
    TEST(hs.data.size != sizeof(int), "Array size not size of int.");
    TEST(hs.data.length != 0, "Initial set doesn't have length 0.");
    TEST(hs.data.capacity != 8, "Initial set doesn't have length 0.");
    TEST(hs.hashes == NULL, "Didn't allocate space for hashes.");
    TEST(hs.hash != inthash, "Didn't save correct hash function.");
    TEST(hs.cmp != intcmp, "Didn't save correct cmp function.");
    return PASS;
}

TESTFN(ds_delete_hashset) {
    ds_alloc = ds_malloc;

    ds_hashset hs = ds_new_hashset(sizeof(int), 8, inthash, intcmp, NULL);
    if(ds_delete_hashset(&hs, NULL) != NULL)
        return "Delete hashset did not return NULL.";
    TEST(hs.data.data != NULL, "Hashset array not set to NULL.");
    TEST(hs.data.size != 0, "Hashset data size not 0 after deletion.");
    TEST(hs.data.capacity != 0, "Hashset data capacity not 0 after deletion.");
    TEST(hs.data.length != 0, "Hashset data length not 0 after deletion.");
    TEST(hs.hashes != NULL, "Hashset hashes not NULL after deletion.");
    TEST(hs.hash != NULL, "Hashset hash function not set to NULL after deletion.");
    TEST(hs.cmp != NULL, "Hashset comparison function not set to NULL after deletion.");
    return PASS;
}

TESTFN(ds_hashset_insert) {
    ds_alloc = ds_malloc;

    ds_hashset hs = ds_new_hashset(sizeof(int), 8, inthash, intcmp, NULL);
    int vals[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    size_t n = sizeof(vals)/sizeof(vals[0]);
    for(size_t i = 0; i < n; ++i) {
        void *v = ds_hashset_insert(&hs, &vals[i], NULL);
        TEST(v == NULL, "Insertion failed.");
        TEST(*(int*)v != vals[i], "Returned value from insert not equal to value passed in.");
        TEST(hs.data.length != i+1, "Length of hashset not increasing with insertion.");
    }
    TEST(hs.data.capacity <= 8,
         "Capacity of hashset did not increase even though we inserted more elements "
         "than initial allocation.");
    ds_delete_hashset(&hs, NULL);
    return PASS;
}

TESTFN(ds_hashset_contains) {
    ds_alloc = ds_malloc;

    ds_hashset hs = ds_new_hashset(sizeof(int), 8, inthash, intcmp, NULL);
    int vals[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    size_t n = sizeof(vals)/sizeof(vals[0]);
    for(size_t i = 0; i < n; ++i) {
        ds_hashset_insert(&hs, &vals[i], NULL);
        TEST(!ds_hashset_contains(&hs, &vals[i]),
                "Hashset does not contain element we just inserted?");
    }
    for(size_t i = 0; i < n; ++i) {
        TEST(!ds_hashset_contains(&hs, &vals[i]),
                "Hashset does not contain element we inserted previously.");
    }

    ds_delete_hashset(&hs, NULL);
    return PASS;
}

TESTFN(ds_hashset_remove) {
    ds_alloc = ds_malloc;

    ds_hashset hs = ds_new_hashset(sizeof(int), 8, inthash, intcmp, NULL);
    int vals[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    size_t n = sizeof(vals)/sizeof(vals[0]);
    for(size_t i = 0; i < n; ++i) {
        ds_hashset_insert(&hs, &vals[i], NULL);
    }
    for(size_t i = 0; i < n; ++i) {
        ds_hashset_remove(&hs, &vals[i]);
        TEST(ds_hashset_contains(&hs, &vals[i]), "Hashset contains element we just removed?");
    }

    ds_delete_hashset(&hs, NULL);
    return PASS;
}

TESTFN(ds_new_hashmap) {
    ds_alloc = ds_malloc;

    ds_hashmap map = ds_new_hashmap(sizeof(int), sizeof(int), 8, inthash, intcmp, NULL);
    TEST(map.keys.data.data == NULL, "Allocation of key array failed.");
    TEST(map.keys.hashes == NULL, "Allocation of hashes array failed.");
    TEST(map.values == NULL, "Allocation of values array failed.");
    TEST(map.keys.data.size != sizeof(int), "Key size does not match sizeof key type.");
    TEST(map.valuesize != sizeof(int), "Value size does not match sizeof value type.");
    TEST(map.keys.data.length != 0, "Initial length of hashmap is not 0.");
    TEST(map.keys.data.capacity != 8, "Initial capacity of hashmap is not the value passed in.");
    TEST(map.keys.hash != inthash, "Hash function is not the function passed in.");
    TEST(map.keys.cmp != intcmp, "Comparison function is not the function passed in.");

    return PASS;
}

TESTFN(ds_delete_hashmap) {
    ds_alloc = ds_malloc;

    ds_hashmap map = ds_new_hashmap(sizeof(int), sizeof(int), 8, inthash, intcmp, NULL);
    void *ret = ds_delete_hashmap(&map, NULL);
    TEST(ret, "Deletion of hashmap did not return NULL.");
    TEST(map.keys.data.data, "Keys array not set to NULL.");
    TEST(map.keys.data.size, "Keys size not set to 0.");
    TEST(map.keys.data.capacity, "Keys capacity not set to 0.");
    TEST(map.keys.data.length, "Keys length not set to 0.");
    TEST(map.keys.hashes, "Keys hashes not set to NULL.");
    TEST(map.keys.hash, "Keys hash function not set to NULL.");
    TEST(map.keys.cmp, "Keys comparison function not set to NULL.");
    TEST(map.values, "Map values not set to NULL.");
    TEST(map.valuesize, "Map valuesize not set to 0.");
    return PASS;
}

TESTFN(ds_hashmap_insert) {
    ds_alloc = ds_malloc;

    ds_hashmap map = ds_new_hashmap(sizeof(int), sizeof(int), 8, inthash, intcmp, NULL);

    int keys[]   = { 1, 2, 4,  8, 16,  32,  64,  128,  256 };
    int values[] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561 };
    size_t n = sizeof(keys)/sizeof(keys[0]);
    for(size_t i = 0; i < n; ++i) {
        void *v = ds_hashmap_insert(&map, &keys[i], &values[i], NULL);
        TEST(!v, "Hashmap returned NULL from insertion.");
        TEST(*(int*)v != values[i], "Hashmap value is not correct");
    }

    ds_delete_hashmap(&map, NULL);

    return PASS;
}

TESTFN(ds_hashmap_contains) {
    ds_alloc = ds_malloc;

    ds_hashmap map = ds_new_hashmap(sizeof(int), sizeof(int), 8, inthash, intcmp, NULL);

    int keys[]   = { 1, 2, 4,  8, 16,  32,  64,  128,  256 };
    int values[] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561 };
    size_t n = sizeof(keys)/sizeof(keys[0]);
    for(size_t i = 0; i < n; ++i) {
        ds_hashmap_insert(&map, &keys[i], &values[i], NULL);
    }
    for(size_t i = 0; i < n; ++i) {
        int c = ds_hashmap_contains(&map, &keys[i]);
        TEST(!c, "Hash map does not contain key we already inserted.");
    }

    ds_delete_hashmap(&map, NULL);

    return PASS;
}

TESTFN(ds_hashmap_at) {
    ds_alloc = ds_malloc;

    ds_hashmap map = ds_new_hashmap(sizeof(int), sizeof(int), 8, inthash, intcmp, NULL);

    int keys[]   = { 1, 2, 4,  8, 16,  32,  64,  128,  256 };
    int values[] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561 };
    size_t n = sizeof(keys)/sizeof(keys[0]);
    for(size_t i = 0; i < n; ++i) {
        ds_hashmap_insert(&map, &keys[i], &values[i], NULL);
    }
    for(size_t i = 0; i < n; ++i) {
        void *v = ds_hashmap_at(&map, &keys[i]);
        TEST(!v, "Hash map does not contain key we inserted.");
        TEST(*(int*)v != values[i], "Value stored at key is not what we expected.");
    }

    ds_delete_hashmap(&map, NULL);

    return PASS;
}

TESTFN(ds_hashmap_remove) {
    ds_alloc = ds_malloc;

    ds_hashmap map = ds_new_hashmap(sizeof(int), sizeof(int), 8, inthash, intcmp, NULL);

    int keys[]   = { 1, 2, 4,  8, 16,  32,  64,  128,  256 };
    int values[] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561 };
    size_t n = sizeof(keys)/sizeof(keys[0]);
    for(size_t i = 0; i < n; ++i) {
        ds_hashmap_insert(&map, &keys[i], &values[i], NULL);
    }
    for(size_t i = 0; i < n; ++i) {
        int r = ds_hashmap_remove(&map, &keys[i]);
        TEST(!r, "Hash map does not contain key we inserted.");
        TEST(ds_hashmap_contains(&map, &keys[i]), "Hash map still contains removed key.");
    }

    ds_delete_hashmap(&map, NULL);

    return PASS;
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    const char *msg;

    // Array
    DOTEST(ds_malloc);
    DOTEST(ds_new_darray);
    DOTEST(ds_delete_darray);
    DOTEST(ds_darray_resize);
    DOTEST(ds_darray_push);
    DOTEST(ds_darray_at);
    DOTEST(ds_darray_concat);
    DOTEST(ds_darray_pop);
    DOTEST(ds_darray_insert);
    DOTEST(ds_darray_insert_array);
    DOTEST(ds_darray_remove);
    DOTEST(ds_darray_remove_range);
    DOTEST(ds_darray_subarray);

    // Hash Set
    DOTEST(ds_new_hashset);
    DOTEST(ds_delete_hashset);
    DOTEST(ds_hashset_insert);
    DOTEST(ds_hashset_contains);
    DOTEST(ds_hashset_remove);

    // Hash Map
    DOTEST(ds_new_hashmap);
    DOTEST(ds_delete_hashmap);
    DOTEST(ds_hashmap_insert);
    DOTEST(ds_hashmap_contains);
    DOTEST(ds_hashmap_at);
    DOTEST(ds_hashmap_remove);

    return 0;
}

