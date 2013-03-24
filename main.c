#include <stdlib.h>
#include <stdio.h>
#include "cf.h"


int main(int argc, char *argv[])
{
    int i;
    printf("argc = %i\n", argc);
    for (i=0; i<argc; ++i){
        printf("  argv[%i]=%s\n", i, argv[i]);
    }

    cf00_string_allocator str_alloc;
    cf00_init_string_allocator(&str_alloc);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_string *s0 = cf00_allocate_string(&str_alloc);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf_00_str_resize(s0, 7);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf_00_str_resize(s0, 27);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf_00_str_resize(s0, 77);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf_00_str_resize(s0, 157);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_string *s1 = cf00_allocate_string(&str_alloc);
    cf00_string *s2 = cf00_allocate_string(&str_alloc);
    cf_00_str_resize(s1, 7);
    cf_00_str_resize(s2, 7);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_free_string(s0);
    cf00_free_string(s1);
    cf00_free_string(s2);
    cf00_str_alloc_debug_dump(&str_alloc);

    cf00_clear_string_allocator(&str_alloc);

    cf00_str_alloc_debug_dump(&str_alloc);

    return EXIT_SUCCESS;
}
