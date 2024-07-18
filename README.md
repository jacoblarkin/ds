# Data Structures

A data structures library written in c.

# Dynamic arrays

Dynamic array implementation which saves size of elements and uses `memcpy` or `memmove` to add elements to the array.
Requries `ds_alloc` to be set prior to use as it uses this internally to allocate and free memory.
Will realloc (using `ds_alloc`) when more space is needed.

Currently limited API, only supporting push (but not pop or push_front/pop_front) and not insertion at an arbitrary point.
Access using at function is bounds checked.

# Testing

Build tests with 
```sh
cc -o test *.c
```
