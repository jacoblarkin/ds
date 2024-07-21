#ifndef DS_H
#define DS_H

#include <stddef.h>

#define DSVERSION_MAJOR 0
#define DSVERSION_MINOR 1
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

#endif
