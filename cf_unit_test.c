/** cf_unit_test.c

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






void cf00_init_unit_test_data(cf00_unit_test_data *d, const uint32 rand_seed)
{

}

uint32 *cf00_test_allocate_temp_uint32(const char *var_name)
{
return 0;
}


void cf00_test_assert(cf00_unit_test_data *d, const boolean condition,
    const char *condition_str, const char *file_name, const int line_number)
{
    /* increment d assertion count */

    if (!condition)
    {
    /* increment d assertion failure count */

    /* log error */
    printf("ASSERTION FAILED  %s [%i]: %s\n", file_name, line_number,
        condition_str);
    }
}

uint32 cf00_test_rand_uint32(cf00_unit_test_data *d)
{
return 0;
}

boolean cf00_test_rand_boolean(cf00_unit_test_data *d)
{
return (boolean)0;
}

float64 cf00_test_rand_float64(cf00_unit_test_data *d)
{
return 0.0;
}

void cf00_test_rand_str_buf(cf00_unit_test_data *d, char *str_buf,
    const size_t len)
{

}

void cf00_test_rand_str_buf_abc123(cf00_unit_test_data *d, char *str_buf,
    const size_t len)
{

}

void cf00_test_rand_str_buf_subset(cf00_unit_test_data *d, const char *superset,
    const size_t superset_len, char *str_buf, const size_t len)
{

}






int cf00_run_unit_test_suite_main(int argc, char *argv[],
    const cf00_unit_test_suite *unit_test_suite)
{
    int result = EXIT_SUCCESS;
    uint32 random_seed = 0;
    uint32 iteration_count = 1;

    /* parse random seed and iteration count from argc and argv */

    result = cf00_run_unit_test_suite(unit_test_suite, random_seed,
        iteration_count);

    return result;
}


int cf00_run_unit_test_suite(const cf00_unit_test_suite *unit_test_suite,
    const uint32 random_seed, const uint32 iteration_count)
{
    /* 
    global random seed = random_seed
    iteration random seed = updated at start of each iteration
    current random number = updated every request for random number

    need the ability to quickly repeat any unit test given a random seed
    */


    uint32 i;
    uint32 test_idx = 0;
    cf00_unit_test_data d;
    const cf00_unit_test *current_unit_test = NULL;

    printf("RUN TEST SUITE START\n");
    if (NULL != unit_test_suite->m_suite_name)
        {
        printf("%s\n", unit_test_suite->m_suite_name);
        }
    printf("TEST_SUITE_RANDOM_SEED=%i\n", (int)random_seed);
    printf("ITERATION_COUNT=%i\n", (int)iteration_count);

    cf00_init_unit_test_data(&d, random_seed);

    for (test_idx = 0; test_idx < unit_test_suite->m_test_count; ++test_idx)
    {
        current_unit_test = &((unit_test_suite->m_tests)[test_idx]);

        /* record start time */
        
        for (i = 0; i < iteration_count; ++i)
        {
            /* clear temp variables   */
        
            /* cf00_reset_unit_test_rand_gen(), but do not reset
            global random seed or any other parameters      */
     
       
     
            /* run one iteration */
            (*(current_unit_test->m_func))(&d);
        }

        /* record stop time */
   
        /* record time for current unit test */

    }


    printf("RUN TEST SUITE END\n");
    
    /* print summary */

   
    // deconstruct unit test data

    return EXIT_SUCCESS; /* return total error count */   

}



