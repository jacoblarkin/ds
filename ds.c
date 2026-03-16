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

void *ds_delete_darray(ds_darray *array, void *alloc_context) {
    void *ret = ds_alloc(alloc_context, array->data, array->size * array->capacity, 0);
    if(!ret) {
        array->length = 0;
	array->capacity = 0;
	array->size = 0;
	array->data = NULL;
    }
    return ret;
}

void *ds_darray_at_unchecked(ds_darray array, size_t index) {
    return (void*)((unsigned char*)array.data + array.size * index);
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

ds_hashset ds_new_hashset(size_t size, size_t capacity, uint64_t (*hash)(void*),
		          int (*cmp)(void*, void*), void *alloc_context) {
    return (ds_hashset){
      .data = ds_new_darray(size, capacity, alloc_context),
      .hashes = ds_alloc(alloc_context, NULL, 0, sizeof(uint64_t) * capacity),
      .hash = hash,
      .cmp = cmp,
    };
}

void *ds_delete_hashset(ds_hashset *set, void *alloc_context) {
    size_t len = sizeof(uint64_t) * set->data.capacity;
    void *a = ds_delete_darray(&set->data, alloc_context);
    if(!set->data.data) {
        set->hashes = ds_alloc(alloc_context, set->hashes, len, 0); 
        set->hash = NULL;
        set->cmp = NULL;
        return set->hashes;
    }
    return a;
}

static inline
size_t ds_hashset_get_location(ds_hashset *set, uint64_t hash, void *elem) {
    size_t loc = hash % set->data.capacity;
    size_t tomb = UINT64_MAX;
    while(set->hashes[loc] != 0) {
        if(set->hashes[loc] == UINT64_MAX) {
            if(tomb == UINT64_MAX) tomb = loc;
        } else if(hash == set->hashes[loc] 
                && set->cmp(elem, ds_darray_at_unchecked(set->data, loc)) == 0) {
            return loc;
        }
        loc = (loc + 1) % set->data.capacity;
    }
    return tomb == UINT64_MAX ? loc : tomb;
}

static inline
void *ds_hashset_resize(ds_hashset *set, size_t new_size, void *alloc_context) {
    ds_darray new_data = ds_new_darray(set->data.size, new_size, alloc_context);
    if(!new_data.data) return NULL;
    size_t old_cap = set->data.capacity;
    uint64_t *new_hashes = ds_alloc(alloc_context, NULL, old_cap * sizeof(uint64_t),
                                     new_size * sizeof(uint64_t));
    if(!new_hashes) { ds_delete_darray(&new_data, alloc_context); return NULL; }
    ds_darray old_data = set->data;
    set->data = new_data;
    uint64_t *old_hashes = set->hashes;
    set->hashes = new_hashes;
    for(size_t i = 0; i < old_cap; ++i) {
        if(old_hashes[i] == 0 || old_hashes[i] == UINT64_MAX) continue;
        void *val = ds_darray_at_unchecked(old_data, i);
        uint64_t hash = set->hash(val);
        size_t loc = ds_hashset_get_location(set, hash, val);
        memcpy((uint8_t*)set->data.data + set->data.size * loc,
               (uint8_t*)old_data.data + old_data.size * i, set->data.size);
        set->hashes[loc] = hash;
        set->data.length++;
    }
    void *ret = ds_delete_darray(&old_data, alloc_context);
    void *ret2 = ds_alloc(alloc_context, old_hashes, old_cap * sizeof(uint64_t), 0);
    return !ret ? ret : ret2;
}

void *ds_hashset_insert(ds_hashset *set, void *elem, void *alloc_context) {
    if(set->data.length >= 3 * set->data.capacity / 4)
        ds_hashset_resize(set, set->data.capacity * 2, alloc_context);
    uint64_t hash = set->hash(elem);
    size_t loc = ds_hashset_get_location(set, hash, elem);
    int newinsertion = set->hashes[loc] == 0;
    memcpy((uint8_t*)set->data.data + set->data.size * loc, elem, set->data.size);
    set->hashes[loc] = hash;
    if(newinsertion) set->data.length++;
    return (uint8_t*)set->data.data + set->data.size * loc;
}

int ds_hashset_contains(ds_hashset *set, void *elem) {
    uint64_t hash = set->hash(elem);
    size_t loc = ds_hashset_get_location(set, hash, elem);
    return set->hashes[loc] != 0 && set->hashes[loc] != UINT64_MAX;
}

int ds_hashset_remove(ds_hashset *set, void *elem) {
    uint64_t hash = set->hash(elem);
    size_t loc = ds_hashset_get_location(set, hash, elem);
    if(set->hashes[loc] == 0 || set->hashes[loc] == UINT64_MAX)
        return 0;
    set->hashes[loc] = UINT64_MAX;
    return 1;
}

