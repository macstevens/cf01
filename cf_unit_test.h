/** cf_unit_test.h

This file is C source code for unit tests testing core cf code.
*/

#ifndef CF_UNIT_TEST_H
#define CF_UNIT_TEST_H

#include <stdio.h>

#include "cf.h"


typedef struct cf00_unit_test_data
{
    // pseudo-random number generator    https://sites.google.com/site/murmurhash/

    // temp variables type *, name

    // uint32 error_count
    // uint32 assertion count
    // time used for each function
} cf00_unit_test_data;

uint32 *cf00_test_allocate_temp_uint32(const char *var_name);

void cf00_test_assert(cf00_unit_test_data *d, const boolean condition,
    const char *condition_str, const char *file_name, const int line_number);

#define CF00_TEST_ASSERT(_unit_test_data, _condition) cf00_test_assert( \
    _unit_test_data, _condition, #_condition, __FILE__, __LINE__)

uint32 cf00_test_rand_uint32(cf00_unit_test_data *d);

boolean cf00_test_rand_boolean(cf00_unit_test_data *d);

float64 cf00_test_rand_float64(cf00_unit_test_data *d);

#endif





