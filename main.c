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
    cf00_str_resize(s0, 7);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_str_resize(s0, 27);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_str_resize(s0, 77);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_str_resize(s0, 157);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_string *s1 = cf00_allocate_string(&str_alloc);
    cf00_string *s2 = cf00_allocate_string(&str_alloc);
    cf00_str_resize(s1, 7);
    cf00_str_resize(s2, 7);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_str_clear(s0);
    cf00_str_clear(s1);
    cf00_str_clear(s2);
    cf00_str_alloc_debug_dump(&str_alloc);

    cf00_str_assign_from_char_ptr(s0, "this is string 0");
    printf("A\n");
    cf00_str_assign(s1, s0);
    printf("B\n");
    cf00_str_assign_from_char_ptr(s0, "this is string 0");
    printf("C\n");
    cf00_str_append_char_buf(s0, " with an extra bit");
    printf("D\n");
    cf00_str_append(s2, s1);
    printf("s0=%s\n", s0->m_char_buf);
    printf("s0->m_length=%i\n", (int)(s0->m_length));
    printf("s0->m_capacity=%i\n", (int)(s0->m_capacity));
    printf("s1=%s\n", s1->m_char_buf);
    printf("s1->m_length=%i\n", (int)(s1->m_length));
    printf("s1->m_capacity=%i\n", (int)(s1->m_capacity));
    printf("s2=%s\n", s2->m_char_buf);
    printf("s2->m_length=%i\n", (int)(s2->m_length));
    printf("s2->m_capacity=%i\n", (int)(s2->m_capacity));
    printf("compare(s0,s1)=%i\n", cf00_str_compare(s0,s1));
    printf("compare(s1,s0)=%i\n", cf00_str_compare(s1,s0));
    printf("compare(s1,s2)=%i\n", cf00_str_compare(s1,s2));
    printf("compare(s2,s1)=%i\n", cf00_str_compare(s2,s1));
    printf("compare(s2,s0)=%i\n", cf00_str_compare(s2,s0));
    printf("compare(s0,s2)=%i\n", cf00_str_compare(s0,s1));
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_free_string(s0);
    cf00_free_string(s1);
    cf00_free_string(s2);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_clear_string_allocator(&str_alloc);
    cf00_str_alloc_debug_dump(&str_alloc);   

    return EXIT_SUCCESS;
}
