#include "ds.h"

#include <string.h>
#include <stdlib.h>

ds_allocator ds_alloc = NULL;

void *ds_malloc(void *context, void *to_free, size_t old, size_t new) {
    (void)context;
    (void)old;
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

int ds_darray_concat(ds_darray *dst, ds_darray *array, void *alloc_context) {
    if(dst->size != array->size) return 0;
    if(dst->capacity < dst->length + array->length)
        if(!ds_darray_resize(dst, dst->length+array->length, alloc_context)) return 0;
    memcpy(ds_darray_at_unchecked(*dst, dst->length), array->data, 
            array->size * array->length);
    dst->length += array->length;
    return 1;
}

int ds_darray_pop(ds_darray *array, void *dst) {
    if(!array->length) return 0;
    array->length -= 1;
    if(dst) memcpy(dst, ds_darray_at_unchecked(*array, array->length), array->size);
    return 1;
}

int ds_darray_insert(ds_darray *array, size_t idx, void *elem, void *alloc_context) {
    if(idx > array->length) return 0;
    if(idx == array->length) return ds_darray_push(array, elem, alloc_context);
    if(array->capacity < array->length + 1)
        if(!ds_darray_resize(array, array->capacity * 2, alloc_context))
            return 0;
    memmove(ds_darray_at_unchecked(*array, idx+1), ds_darray_at_unchecked(*array, idx), 
            array->size * (array->length - idx));
    memcpy(ds_darray_at_unchecked(*array, idx), elem, array->size);
    array->length += 1;
    return 1;
}

int ds_darray_insert_array(ds_darray *dst, size_t idx, ds_darray *elems, void *alloc_context) {
    if(dst->size != elems->size) return 0;
    if(idx > dst->length) return 0;
    if(idx == dst->length) return ds_darray_concat(dst, elems, alloc_context);
    if(dst->capacity < dst->length + elems->length)
        if(!ds_darray_resize(dst, dst->length + elems->length, alloc_context))
            return 0;
    memmove(ds_darray_at_unchecked(*dst, idx + elems->length),
            ds_darray_at_unchecked(*dst, idx), elems->size * (dst->length - idx));
    memcpy(ds_darray_at_unchecked(*dst, idx), elems->data, elems->size * elems->length); 
    dst->length += elems->length;
    return 1;
}

int ds_darray_remove(ds_darray *array, size_t idx, void *dst) {
    if(idx >= array->length) return 0;
    if(dst) memcpy(dst, ds_darray_at_unchecked(*array, idx), array->size);
    array->length -= 1;
    if(idx < array->length)
        memmove(ds_darray_at_unchecked(*array, idx),
                ds_darray_at_unchecked(*array, idx+1),
                array->size * (array->length - idx));
    return 1;
}

int ds_darray_remove_range(ds_darray *array, size_t beg, size_t end, ds_darray *dst, void *alloc_context) {
    if(beg >= array->length || end > array->length || end <= beg) return 0;
    if(dst) {
        ds_darray tmp = (ds_darray){
            .length = end - beg,
            .capacity = end - beg,
            .size = array->size,
            .data = ds_darray_at_unchecked(*array, beg),
        };
        ds_darray_concat(dst, &tmp, alloc_context);
    }
    if(end < array->length)
        memmove(ds_darray_at_unchecked(*array, beg), ds_darray_at_unchecked(*array, end),
                array->size * (array->length - end));
    array->length -= end - beg;
    return 1;

}

ds_darray ds_darray_subarray(ds_darray *array, size_t beg, size_t end, void *alloc_context) {
    if(beg >= array->length || end > array->length || end <= beg) return (ds_darray){0};
    ds_darray ret = ds_new_darray(array->size, end - beg, alloc_context);
    if(!ret.data) return ret;
    ret.length = end - beg;
    memcpy(ret.data, ds_darray_at_unchecked(*array, beg), array->size * (end - beg));
    return ret;
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
    return ret ? ret : ret2;
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

ds_hashmap ds_new_hashmap(size_t keysize, size_t valuesize, size_t capacity, 
        uint64_t (*keyhash)(void*), int (*keycmp)(void*, void*), void *alloc_context) {
    return (ds_hashmap) {
        .keys = ds_new_hashset(keysize, capacity, keyhash, keycmp, alloc_context),
        .values = ds_alloc(alloc_context, NULL, 0, capacity * valuesize),
        .valuesize = valuesize,
    };
}

void *ds_delete_hashmap(ds_hashmap *map, void *alloc_context) {
    void *ret = ds_delete_hashset(&map->keys, alloc_context);
    if(ret) return ret;
    ret = ds_alloc(alloc_context, map->values, 
            map->valuesize * map->keys.data.capacity, 0);
    if(ret) return ret;
    map->values = NULL;
    map->valuesize = 0;
    return NULL;
}

static inline
void *ds_hashmap_resize(ds_hashmap *map, size_t new_size, void *alloc_context) {
    ds_darray new_keys = ds_new_darray(map->keys.data.size, new_size, alloc_context);
    if(!new_keys.data) return NULL;
    size_t old_cap = map->keys.data.capacity;
    uint64_t *new_hashes = ds_alloc(alloc_context, NULL, old_cap * sizeof(uint64_t),
                                     new_size * sizeof(uint64_t));
    if(!new_hashes) { ds_delete_darray(&new_keys, alloc_context); return NULL; }
    uint64_t *new_values = ds_alloc(alloc_context, NULL, old_cap * map->valuesize,
                                     new_size * map->valuesize);
    if(!new_values) { 
        ds_delete_darray(&new_keys, alloc_context); 
        ds_alloc(alloc_context, new_hashes, new_size * sizeof(uint64_t), 0); 
        return NULL; 
    }
    ds_darray old_keys = map->keys.data;
    map->keys.data = new_keys;
    uint64_t *old_hashes = map->keys.hashes;
    map->keys.hashes = new_hashes;
    void *old_values = map->values;
    map->values = new_values;
    for(size_t i = 0; i < old_cap; ++i) {
        if(old_hashes[i] == 0 || old_hashes[i] == UINT64_MAX) continue;
        void *key = ds_darray_at_unchecked(old_keys, i);
        uint64_t hash = map->keys.hash(key);
        size_t loc = ds_hashset_get_location(&map->keys, hash, key);
        memcpy((uint8_t*)map->keys.data.data + map->keys.data.size * loc,
               (uint8_t*)old_keys.data + old_keys.size * i, map->keys.data.size);
        map->keys.hashes[loc] = hash;
        memcpy((uint8_t*)map->values + map->valuesize * loc,
               (uint8_t*)old_values + map->valuesize * i, map->valuesize);
        map->keys.data.length++;
    }
    void *ret = ds_delete_darray(&old_keys, alloc_context);
    void *ret2 = ds_alloc(alloc_context, old_hashes, old_cap * sizeof(uint64_t), 0);
    void *ret3 = ds_alloc(alloc_context, old_values, old_cap * map->valuesize, 0);
    return ret ? ret : ret2 ? ret2 : ret3;
}

void *ds_hashmap_insert(ds_hashmap *map, void *key, void *value, void *alloc_context) {
    if(map->keys.data.length >= 3 * map->keys.data.capacity / 4)
        ds_hashmap_resize(map, map->keys.data.capacity * 2, alloc_context);
    uint64_t hash = map->keys.hash(key);
    size_t loc = ds_hashset_get_location(&map->keys, hash, key);
    int newinsertion = map->keys.hashes[loc] == 0;
    memcpy((uint8_t*)map->keys.data.data + map->keys.data.size * loc, 
            key, map->keys.data.size);
    map->keys.hashes[loc] = hash;
    if(newinsertion) map->keys.data.length++;
    memcpy((uint8_t*)map->values + map->valuesize * loc, value, map->valuesize);
    return (uint8_t*)map->values + map->valuesize * loc;
}

int ds_hashmap_contains(ds_hashmap *map, void *key) {
    uint64_t hash = map->keys.hash(key);
    size_t loc = ds_hashset_get_location(&map->keys, hash, key);
    return map->keys.hashes[loc] != 0 && map->keys.hashes[loc] != UINT64_MAX;
}

void *ds_hashmap_at(ds_hashmap *map, void *key) {
    uint64_t hash = map->keys.hash(key);
    size_t loc = ds_hashset_get_location(&map->keys, hash, key);
    return map->keys.hashes[loc] != 0 && map->keys.hashes[loc] != UINT64_MAX
        ? map->values + map->valuesize * loc
        : NULL;
}

int ds_hashmap_remove(ds_hashmap *map, void *key) {
    uint64_t hash = map->keys.hash(key);
    size_t loc = ds_hashset_get_location(&map->keys, hash, key);
    if(map->keys.hashes[loc] == 0 || map->keys.hashes[loc] == UINT64_MAX)
        return 0;
    map->keys.hashes[loc] = UINT64_MAX;
    return 1;
}

