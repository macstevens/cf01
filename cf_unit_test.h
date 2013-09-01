/** cf_unit_test.h

This file is C source code for unit tests testing core cf code.
*/

#ifndef CF_UNIT_TEST_H
#define CF_UNIT_TEST_H

#include <stdio.h>

#include "cf.h"



/*
main(argc, argv)
  cf00_unit_test_main(argc, argv, test_suite)
    cf00_run_test_suite(iteration_count, rand_seed, test_suite)














suite??
 +-unit test
   +-unit test top level function <-- each is called with same random seed
     +subroutine



iteration number
iteration count factor



array of unit test functions

typedef void (*cf00_unit_test_func_ptr)(cf00_unit_test_data *d);

typedef unit_test_func_ptr *unit_test_func_ptr_array;

typedef struct cf00_unit_test
{
    cf00_unit_test_func_ptr m_func;
    const char *m_func_name; 
    uint32 m_iteration_count_factor;
};

typedef struct cf00_unit_test_suite
{
    const char *m_suite_name;
    cf00_unit_test *m_tests;
    size_t m_element_count;
}


run_unit_test_suite(cf00_unit_test_data *d, unit_test_func_ptr_array func_array,
    const size_t func_count);

*/


typedef struct cf00_unit_test_data
{
    // pseudo-random number generator    https://sites.google.com/site/murmurhash/

    // temp variables type *, name

    // uint32 error_count
    // uint32 assertion count
    // time used for each function

    // current iteration number
    // total iteration count    

    // callback function for logging
} cf00_unit_test_data;

void cf00_init_unit_test_data(cf00_unit_test_data *d, const uint32 rand_seed);

uint32 *cf00_test_allocate_temp_uint32(const char *var_name);

void cf00_test_assert(cf00_unit_test_data *d, const boolean condition,
    const char *condition_str, const char *file_name, const int line_number);

#define CF00_TEST_ASSERT(_unit_test_data, _condition) cf00_test_assert( \
    _unit_test_data, _condition, #_condition, __FILE__, __LINE__)

uint32 cf00_test_rand_uint32(cf00_unit_test_data *d);

boolean cf00_test_rand_boolean(cf00_unit_test_data *d);

float64 cf00_test_rand_float64(cf00_unit_test_data *d);

void cf00_test_rand_str_buf(cf00_unit_test_data *d, char *str_buf,
    const size_t len);

void cf00_test_rand_str_buf_abc123(cf00_unit_test_data *d, char *str_buf,
    const size_t len);

void cf00_test_rand_str_buf_subset(cf00_unit_test_data *d, const char *superset,
    const size_t superset_len, char *str_buf, const size_t len);









#endif





