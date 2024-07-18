#ifndef DS_H
#define DS_H

#include <stddef.h>

#define DSVERSION_MAJOR 0
#define DSVERSION_MINOR 0
#define DSVERSION_PATCH 1

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

#endif
