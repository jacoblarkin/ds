#include "ds.h"

#include <stdlib.h>

ds_allocator ds_alloc = NULL;

void *ds_malloc(void *context, void *to_free, size_t old, size_t new) {
    if(to_free == NULL && new) return malloc(new);
    if(to_free && new) return realloc(to_free, new);
    if(to_free && !new) free(to_free);
    return NULL;
}
