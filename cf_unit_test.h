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


run_unit_test_suite(cf00_unit_test_data *d, unit_test_func_ptr_array func_array,
    const size_t func_count);

*/


typedef struct cf00_unit_test_data
{
    // test suite random seed
    // current iteration start random seed
    // pseudo-random number generator    https://sites.google.com/site/murmurhash/

    // temp variables type *, name

    uint32 m_total_assertion_count; // uint32 assertion count
    uint32 m_total_error_count;  // uint32 error_count

    // time used for each unit test
    // assertion count for each unit test
    // error count for each unit test

    // current unit test
    uint32 m_current_iteration_idx; // current iteration number
    uint32 m_total_iteration_count; // total iteration count    

    // callback function for logging
} cf00_unit_test_data;

void cf00_init_unit_test_data(cf00_unit_test_data *d, const uint32 rand_seed);
// deconstruct test data
uint32 *cf00_test_allocate_temp_uint32(const char *var_name);
void cf00_test_assert(cf00_unit_test_data *d, const boolean condition,
    const char *condition_str, const char *file_name, const int line_number);
uint32 cf00_test_rand_uint32(cf00_unit_test_data *d);
boolean cf00_test_rand_boolean(cf00_unit_test_data *d);
float64 cf00_test_rand_float64(cf00_unit_test_data *d);
void cf00_test_rand_str_buf(cf00_unit_test_data *d, char *str_buf,
    const size_t len);
void cf00_test_rand_str_buf_abc123(cf00_unit_test_data *d, char *str_buf,
    const size_t len);
void cf00_test_rand_str_buf_subset(cf00_unit_test_data *d, const char *superset,
    const size_t superset_len, char *str_buf, const size_t len);

#define CF00_TEST_ASSERT(_unit_test_data, _condition) cf00_test_assert( \
    _unit_test_data, _condition, #_condition, __FILE__, __LINE__)



typedef void (*cf00_unit_test_func_ptr)(cf00_unit_test_data *d);

typedef struct cf00_unit_test
{
    const char *m_func_name; 
    cf00_unit_test_func_ptr m_func;
} cf00_unit_test;

typedef struct cf00_unit_test_suite
{
    const char *m_suite_name;
    const cf00_unit_test *m_tests;
    size_t m_test_count;
} cf00_unit_test_suite;


int cf00_run_unit_test_suite_main(int argc, char *argv[],
    const cf00_unit_test_suite *unit_test_suite);

int cf00_run_test_suite(const cf00_unit_test_suite *unit_test_suite,
    const uint32 random_seed, const uint32 iteration_count);


/*
void cf00_run_unit_test(func *, randomseed, cf00_unit_test_data, iteration count)
*/




#endif





