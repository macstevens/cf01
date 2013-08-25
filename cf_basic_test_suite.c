/** cf_basic_test_suite.c

*/

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cf.h"
#include "cf_unit_test.h"

/* STATIC FUNCTIONS */






/* EXTERN FUNCTIONS */






/*
UNIT TESTS



void cf00_run_unit_test(func *, randomseed, cf00_unit_test_data, iteration count)


array
  unit test func,  "name",  iteration count
  unit test func,  "name",  iteration count
  unit test func,  "name",  iteration count






*/


int cf00_run_basic_test_suite(int argc, char *argv[])
{

    int i;
    printf("argc = %i\n", argc);
    for (i=0; i<argc; ++i){
        printf("  argv[%i]=%s\n", i, argv[i]);
    }

    cf00_string_allocator str_alloc;
    cf00_str_alloc_init(&str_alloc);
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
    printf("compare(s0,s2)=%i\n", cf00_str_compare(s0,s2));
    cf00_str_alloc_debug_dump(&str_alloc);

    cf00_str_vec *sv0 = cf00_allocate_str_vec(&str_alloc);
    cf00_str_vec *sv1 = cf00_allocate_str_vec(&str_alloc);
    printf("compare(sv0,sv1)=%i\n", cf00_str_vec_compare(sv0,sv1));
    cf00_str_vec_push_back_copy(sv0, s0);
    cf00_str_vec_push_back_copy(sv0, s1);
    cf00_str_vec_push_back_copy(sv0, s2);
    cf00_str_vec_push_back_copy(sv0, s0);
    cf00_str_vec_push_back_copy(sv0, s1);
    cf00_str_vec_push_back_copy(sv0, s2);
    printf("compare(sv0,sv1)=%i\n", cf00_str_vec_compare(sv0,sv1));
    cf00_str_vec_push_back_copy(sv1, s0);
    cf00_str_vec_push_back_copy(sv1, s1);
    cf00_str_vec_push_back_copy(sv1, s2);
    cf00_str_vec_push_back_copy(sv1, s0);
    cf00_str_vec_push_back_copy(sv1, s1);
    cf00_str_vec_push_back_copy(sv1, s2);
    printf("compare(sv0,sv1)=%i\n", cf00_str_vec_compare(sv0,sv1));
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_str_vec_resize(sv0, 15);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_str_vec_resize(sv0, 31);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_str_vec_resize(sv0, 63);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_str_vec_resize(sv0, 127);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_str_vec_clear(sv0);
    cf00_str_alloc_debug_dump(&str_alloc);


    cf00_free_string(s0);
    cf00_free_string(s1);
    cf00_free_string(s2);
    cf00_str_alloc_debug_dump(&str_alloc);
    cf00_str_alloc_clear(&str_alloc);
    cf00_str_alloc_debug_dump(&str_alloc);  

{
    char msg_buf[25];
    *msg_buf = 0x0;
    cf00_str_verify_data(NULL, msg_buf, sizeof(msg_buf));
    printf("msg_buf = %s\n\n", msg_buf);
}

    return EXIT_SUCCESS;
}
