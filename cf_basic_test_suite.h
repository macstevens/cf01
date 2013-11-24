/** cf_unit_test.h

This file is C source code for unit tests testing core cf code.
*/

#ifndef CF_BASIC_TEST_SUITE_H
#define CF_BASIC_TEST_SUITE_H

#include <stdio.h>

#include "cf.h"
#include "cf_unit_test.h"


int cf00_run_basic_test_suite_main(int argc, char *argv[]);

void cf00_run_string_test(cf00_unit_test_data *d);

void cf00_run_str_vec_test(cf00_unit_test_data *d);



#endif /* CF_BASIC_TEST_SUITE_H */

