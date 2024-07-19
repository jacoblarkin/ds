#include "ds.h"

#include <stdio.h>

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define RESET "\033[0m"

#define PASS NULL
#define TEST(fn) const char *test_##fn()
#define DOTEST(fn) (msg = test_##fn()) ? printf(RED "FAIL" RESET " (" #fn "): %s", msg) \
			 : puts(GREEN "PASS" RESET " (" #fn ")")

TEST(ds_malloc) {
    ds_alloc = ds_malloc;

    int *tmp = ds_alloc(NULL, NULL, 0, 4);
    if(!tmp) return "Failed to allocate memory.";
    *tmp = 4;
    tmp = ds_alloc(NULL, tmp, 4, 8);
    if(!tmp) return "Failed to reallocate memory.";
    if(*tmp != 4) return "Failed to copy existing data when reallocating.";
    tmp = ds_alloc(NULL, tmp, 8, 0);
    if(tmp) return "Failed to free memory or failed to return NULL after freeing memory.";

    return PASS;
}

int main(int argc, char **argv) {
    const char *msg;
    DOTEST(ds_malloc);

    return 0;
}
