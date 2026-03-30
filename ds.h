#ifndef DS_H
#define DS_H

#include <stddef.h>
#include <stdint.h>

#define DSVERSION_MAJOR 0
#define DSVERSION_MINOR 3
#define DSVERSION_PATCH 0

// Allocator type
// Assumptions:
//   If to_free is NULL, then will allocate a new object with size new_ 
//   and old will be ignored. A pointer to the newly allocated memory is returned.
//
//   If to_free is not NULL and new_ is 0, then will free to_free and return NULL. 
//   Old should be the size of to_free
//
//   If to_free is not NULL and new_ is not 0, then will allocate a new buffer of 
//   size new_ and copy old bytes from to_free to new buffer. to_free is freed and 
//   a pointer to the new buffer is returned. The new buffer is allowed to overlap 
//   with to_free.
typedef void *(*ds_allocator)(void *context, void *to_free, size_t old, size_t new_);
// Allocator used by DS functions.
// Must be set before using DS functions which allocate/free memory.
extern ds_allocator ds_alloc;
// Default allocator using malloc/realloc/free from c standard library.
// To use add `ds_alloc = ds_malloc;`
void *ds_malloc(void *context, void *to_free, size_t old, size_t new_);

// Dynamic array
// length and capacity are self explanatory
// size is the size of an element of the array
// data is pointer to buffer of size >=(size*capacity)
typedef struct {
    size_t length, capacity, size;
    void* data;
} ds_darray;

// Access ds_darray as if it is a c array (i.e., using [])
// a is the array (as a value, not a pointer) and T is the type
#define DS_DARRAY_AS(a, T) ((T*)(a).data)

// Create a new ds_darray with a certain element size and capacity
// alloc_context is the context pointer passed to ds_alloc
// da_alloc must be initialized before calling this function
ds_darray ds_new_darray(size_t size, size_t capacity, void *alloc_context);

// Free data pointer for a ds_darray
// alloc_context is the context pointer passed to ds_alloc
// da_alloc must be initialized before calling this function
// returns value returned by ds_alloc
// If return value is NULL, sets length, capacity, size to 0 and data to NULL
void *ds_delete_darray(ds_darray *array, void *alloc_context);

// Return pointer to index-th element of data pointer
// (returns data + size * index)
// returns NULL if index > length - 1
void *ds_darray_at(ds_darray array, size_t index);

// Return pointer to index-th element of data pointer
// returns data + size * index regardless of array length
void *ds_darray_at_unchecked(ds_darray array, size_t index);

// Resize darray to have capacity new_capacity
// returns new_capacity if successful, returns 0 if allocation failed
// alloc_context is the context pointer passed to ds_alloc
// da_alloc must be initialized before calling this function
size_t ds_darray_resize(ds_darray *array, size_t new_capacity, void *alloc_context);

// Push elem on the back of the array and returns 1 if successful or 0 if failed
// calls resize if capacity is not large enough to fit elem, grows by doubling capacity
// size bytes from elem will be memcpy'd to array->data[array->length]
// alloc_context is the context pointer passed to ds_alloc
// da_alloc must be initialized before calling this function
int ds_darray_push(ds_darray *array, void *elem, void *alloc_context);

// Concatenate 2 arrays. Save the result in dst (i.e., the front array).
// Resizes dst to fit both its original elements and all of array.
// Returns 1 on success and 0 on failure (i.e., allocation failure or sizes different).
int ds_darray_concat(ds_darray *dst, ds_darray *array, void *alloc_context);

// Pop element off the back of the array and fill dst with value.
// If dst is NULL, element is popped, but dst is not filled.
// Returns 1 if pop successful (i.e., array->length > 0) and 0 if array is empty.
int ds_darray_pop(ds_darray *array, void *dst);

// Insert an element into array at index idx.
// Pushed elements in range [idx, length) back by 1.
// Will auto resize if needed.
// If idx > array->length operation fails. If idx == array->length equivalent to push.
// Returns 1 if successful, 0 if failed.
int ds_darray_insert(ds_darray *array, size_t idx, void *elem, void *alloc_context);

// Insert an array (elems) into another array (dst) at index idx in dst.
// Pushed elements in range [idx, length) back by elems.length.
// Will auto resize if needed.
// If idx > array->length operation fails. If idx == array->length equivalent to concat.
// Returns 1 if successful, 0 if failed.
int ds_darray_insert_array(ds_darray *dst, size_t idx, ds_darray *elems, void *alloc_context);

// Remove an element from the array at index idx.
// Saves removed element in dst unless dst is NULL.
// Returns 1 if removal successful (idx < array->length) and 0 otherwise.
int ds_darray_remove(ds_darray *array, size_t idx, void *dst);

// Remove multple elements from the array in the half open range [beg, end).
// Saves removed elements in dst unless dst is NULL. Will resize dst if needed.
// Returns 1 if removal successful (end <= array->length and no allocation failures)
// and 0 otherwise.
int ds_darray_remove_range(ds_darray *array, size_t beg, size_t end, ds_darray *dst, void *alloc_context);

// Create a new array which is a subarray of the given array.
// Copies elements in half open range [beg, end) into new array without 
// removing them from the original array.
// If allocation fails, returned array will be empty.
ds_darray ds_darray_subarray(ds_darray *array, size_t beg, size_t end, void *alloc_context);

// Hash Set
// A hash set is just a dynamic array with a hash function to determine where
// in the array to store a given value.
// Open addressing with linear probing is used to deal with collision.
// Because of this, the hash function should never return 0 (the empty sentinel)
// or UINT64_MAX (the zombie sentinel).
// Two functions are needed for the hash set to operate, a hash function which
// takes a void* to the data to hash, and a comparison function which compares
// two void* that point to the elements to compare.
// The comparison function should return 0 if the values are equal and a non-zero
// int otherwise.
// Finally, there is a uint64_t* to an array of hashes corresponding to the values
// stored in the data ds_darray.
typedef struct {
    ds_darray data;
    uint64_t *hashes;
    uint64_t (*hash)(void*);
    int (*cmp)(void*, void*);
} ds_hashset;

// Create a new hash set with a given element size, initial capacity,
// hash function, comparison function, and allocation context.
// ds_alloc must be initialized before calling this function.
ds_hashset ds_new_hashset(size_t size, size_t capacity, uint64_t (*hash)(void*),
		          int (*cmp)(void*, void*), void *alloc_context);

// Delete a ds_hashset with a given allocation context.
// Returns result of ds_alloc, i.e., NULL if deletion successful, result for deleting
// hashes if deletion of data successful but not hashes, result of deleting data if
// deletion unsuccesful.
// If deletion sucessful, data size, length, capacity set to 0 and hashes and
// data.data set to NULL.
// ds_alloc must be initialized before calling this.
void *ds_delete_hashset(ds_hashset *set, void *alloc_context);

// Insert a new element into the set by hashing the value at elem.
// Will resize data and hashes if needed (i.e., if length >= 0.7 * capacity).
// If there is a conflict, open addressing with linear probing is used.
// returns a pointer to the element in the set if successful, NULL if failed.
// If set already contains element, returns pointer to existing element.
// ds_alloc must be initialized if resizing is tried.
void *ds_hashset_insert(ds_hashset *set, void *elem, void *alloc_context);

// Check if hashset contains element elem.
// Hashes elem using hash and compares values using cmp.
// Returns 1 if it does contain the element and 0 otherwise.
// Does not allocate or free memeory.
int ds_hashset_contains(ds_hashset *set, void *elem);

// Remove element elem from hashset
// Sets hash in hashes to UINT64_MAX as zombie sentinel.
// Returns 1 if value found and removed, and 0 if value is not in set.
// Does not allocate or free memory.
int ds_hashset_remove(ds_hashset *set, void *elem);

// Hashmap
// Hashset of keys
// values is array of values with locations corresponding to locations of keys
// Open addressing with linear probing is used to deal with collision.
// Because of this, the hash function should never return 0 (the empty sentinel)
// or UINT64_MAX (the zombie sentinel).
// Two functions are needed for the hash map to operate, a hash function which
// takes a void* to the key to hash, and a comparison function which compares
// two void* that point to the keys to compare.
// The comparison function should return 0 if the keys are equal and a non-zero
// int otherwise.
typedef struct {
    ds_hashset keys;
    void *values;
    size_t valuesize;
} ds_hashmap;

// Create a new hash map with given key and value sizes, initial capacity,
// hash function, comparison function, and allocation context.
// hash function and comparison function should operate on keys
// ds_alloc must be initialized before calling this function.
ds_hashmap ds_new_hashmap(size_t keysize, size_t valuesize, size_t capacity, 
        uint64_t (*keyhash)(void*), int (*keycmp)(void*, void*), void *alloc_context);

// Delete a ds_hashmap with a given allocation context.
// Returns result of ds_alloc, i.e., NULL if deletion successful, result for deleting
// values if deletion of keys successful but not values, result of deleting keys if
// deletion unsuccesful.
// If deletion sucessful, keys size, length, capacity, and values size set to 0,
// and keys hashes, keys.data, and values set to NULL.
// ds_alloc must be initialized before calling this.
void *ds_delete_hashmap(ds_hashmap *map, void *alloc_context);

// Insert a new value into the map by hashing the given key
// Set will resize if needed, and values will be resized accordingly
// If there is a conflict, open addressing with linear probing is used.
// returns a pointer to the value in the map if successful, NULL if failed.
// If map already contains keys, replaces value with new value
// ds_alloc must be initialized if resizing is tried.
void *ds_hashmap_insert(ds_hashmap *map, void *key, void *value, void *alloc_context);

// Check if hashmap contains key 
// Hashes key using data.hash and compares values using data.cmp.
// Returns 1 if it does contain the element and 0 otherwise.
// Does not allocate or free memeory.
int ds_hashmap_contains(ds_hashmap *map, void *key);

// Returns value corresponding to key if map contains key
// Return NULL if map does not contain key
// Does not allocate or free memeory.
void *ds_hashmap_at(ds_hashmap *map, void *key);

// Remove element elem from hashmap
// Sets hash in data.hashes to UINT64_MAX as zombie sentinel.
// Returns 1 if key found and removed, and 0 if key is not in map.
// Does not allocate or free memory.
int ds_hashmap_remove(ds_hashmap *map, void *key);

#endif

