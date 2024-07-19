#include "ds.h"

#include <string.h>
#include <stdlib.h>

ds_allocator ds_alloc = NULL;

void *ds_malloc(void *context, void *to_free, size_t old, size_t new) {
    if(to_free == NULL && new) return malloc(new);
    if(to_free && new) return realloc(to_free, new);
    if(to_free && !new) free(to_free);
    return NULL;
}

ds_darray ds_new_darray(size_t size, size_t capacity, void *alloc_context) {
    ds_darray ret = (ds_darray){ .length = 0, .capacity = capacity, 
                                 .size = size, .data = NULL };
    if(size != 0 && capacity != 0) 
        ret.data = ds_alloc(alloc_context, NULL, 0, size * capacity);
    return ret;
}

void *ds_delete_darray(ds_darray array, void *alloc_context) {
    return ds_alloc(alloc_context, array.data, array.size * array.capacity, 0);
}

void *ds_darray_at(ds_darray array, size_t index) {
    return index < array.length ? 
           (void*)((unsigned char*)array.data + array.size * index) : 
           NULL;
}

size_t ds_darray_resize(ds_darray *array, size_t new_capacity, void *alloc_context) {
    array->data = ds_alloc(alloc_context, array->data, array->capacity * array->size,
            new_capacity * array->size);
    array->capacity = new_capacity;
    return array->data ? new_capacity : 0;
}

int ds_darray_push(ds_darray *array, void *elem, void *alloc_context) {
    if(array->length >= array->capacity) {
        size_t cap = ds_darray_resize(array, array->capacity * 2, alloc_context);
        if(cap == 0)
            return 0;
    }
    void *dst = (unsigned char*)array->data + array->size * array->length;
    void *ret = memcpy(dst, elem, array->size);
    array->length++;
    return ret == dst;
}
