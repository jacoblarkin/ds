# Data Structures

A data structures library written in c.

# Dynamic arrays

Dynamic array implementation which saves size of elements and uses `memcpy` or `memmove` to add elements to the array.
Requries `ds_alloc` to be set prior to use as it uses this internally to allocate and free memory.
Will realloc (using `ds_alloc`) when more space is needed.

Currently limited API, only supporting push (but not pop or push_front/pop_front) and not insertion at an arbitrary point.
Access using at function is bounds checked.
Unchecked access also allowed in a separate function.

## TODO
- [ ] Insertion at arbitrary points
- [ ] Pop from back

# Hash sets

Hash set implementation which saves elements in a dynamic array at a location calculated by some hash function.
Similar to dynamic arrays, requires `ds_alloc` to be set prior to use since it uses a dynamic array under the hood.
Saves objects contiguously in dynamic array with a separate (`hashset` managed) array for the hashes.
Uses open addressing for hash conflict resolution.

Basic API implemented including insertion, removal, and checking for containment.
User must provide both a hash function and a comparison function with signatures
```c
uint64_t hash(void*); // Values 0 and UINT64_MAX reserved
int cmp(void*, void*); // Returns 0 when objects equal
```
Hash value `0` is reserved for the empty sentinel and hash value `UINT64_MAX` is reserved for the zombie sentinel.

## TODO
- [ ] Optimize case where capacity is power of 2

# Testing

Build tests with 
```sh
cc -o test *.c
```
