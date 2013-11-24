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


void init_unit_test_rand_data(cf00_unit_test_rand_data *rd,
    const uint32 rand_seed)
{
    if (NULL != rd)
    {
        rd->m_w = rand_seed;
        rd->m_z = 0xFFFFFFFF ^ rand_seed;
    }
}

uint32 advance_unit_test_rand_data(cf00_unit_test_rand_data *rd)
{
    uint32 result = 0;
    if (NULL != rd)
    {
        /* bassed on multiply-with-carry formula, by George Marsaglia,
        http://en.wikipedia.org/wiki/Random_number_generation 
        https://groups.google.com/forum/#!topic/sci.crypt/yoaCpGWKEk0 */
        if (0 == rd->m_w || 0x464FFFFF == rd->m_w)
            {
            ++(rd->m_w);
            }
        if (0 == rd->m_z || 0x9068FFFF == rd->m_z)
            {
            ++(rd->m_z);
            }
        rd->m_z = 36969 * ((rd->m_z) & 65535) + ((rd->m_z) >> 16);
        rd->m_w = 18000 * ((rd->m_w) & 65535) + ((rd->m_w) >> 16);
        result = ((rd->m_z) << 16) + rd->m_w;
    }
    return result;
}


#if 0
    const struct cf00_unit_test_suite *m_unit_test_suite;
    cf00_indiv_unit_test_data *m_indiv_test_data; /* array */

    uint32 m_random_seed;           /* test suite random seed */
    uint32 m_current_random_seed;   /* current iteration start random seed */
    cf00_unit_test_rand_data m_rng; /* pseudo-random number generator */ 

    // temp variables type *, name

    uint32 m_total_assertion_count;
    uint32 m_total_error_count;

    uint32 m_current_unit_test_idx;
    uint32 m_current_iteration_idx;
    uint32 m_total_iteration_count;    

    // callback function for logging
} cf00_unit_test_data;

void cf00_init_unit_test_data(cf00_unit_test_data *d,
    const cf00_unit_test_suite *test_suite, const uint32 rand_seed);
void cf00_free_unit_test_data(cf00_unit_test_data *d);
#endif

void cf00_init_unit_test_data(cf00_unit_test_data *d,
    const cf00_unit_test_suite *test_suite, const uint32 rand_seed,
    const uint32 iteration_count)
{
    size_t i;
    cf00_indiv_unit_test_data *indiv_unit_test_data;
    if (NULL != d)
    {
        memset(d, 0, sizeof(*d));
        d->m_unit_test_suite = test_suite;
        d->m_indiv_test_data = (cf00_indiv_unit_test_data *)malloc(
            (test_suite->m_test_count) * sizeof(cf00_indiv_unit_test_data));
        indiv_unit_test_data = d->m_indiv_test_data;
        for (i = 0; i < test_suite->m_test_count; ++i, ++indiv_unit_test_data)
        {
            indiv_unit_test_data->m_unit_test_idx = i;
            indiv_unit_test_data->m_assertion_count = 0;
            indiv_unit_test_data->m_error_count = 0;
            indiv_unit_test_data->m_elapsed_time = 0;
        }
        d->m_random_seed = rand_seed;
        d->m_current_random_seed = rand_seed;
        init_unit_test_rand_data(&(d->m_rng), rand_seed);

        d->m_total_assertion_count = 0;
        d->m_total_error_count = 0;
        d->m_total_elapsed_time = 0;

        d->m_current_unit_test_idx;
        d->m_current_iteration_idx;
        d->m_total_iteration_count = iteration_count;    
    }
}

void cf00_free_unit_test_data(cf00_unit_test_data *d)
{
    if (NULL != d)
    {
        free(d->m_indiv_test_data);
    }
}

uint32 *cf00_test_allocate_temp_uint32(const char *var_name)
{
return 0;
}


void cf00_test_assert(cf00_unit_test_data *d, const boolean condition,
    const char *condition_str, const char *file_name, const int line_number)
{
    /* increment d assertion count */
    if (NULL != d)
    {
        ++(d->m_total_assertion_count);
        ++((d->m_indiv_test_data)[
            d->m_current_unit_test_idx].m_assertion_count);
    }

    if (!condition)
    {
        /* increment d assertion failure count */
        if (NULL != d)
        {
            ++(d->m_total_error_count);
            ++((d->m_indiv_test_data)[
                d->m_current_unit_test_idx].m_error_count);
        }

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

    time_t start_time;
    time_t stop_time;
    time_t elapsed_time;
    cf00_unit_test_data d;
    const cf00_unit_test *current_unit_test = NULL;
    cf00_indiv_unit_test_data *indiv_unit_test_data = NULL;


    printf("RUN TEST SUITE START\n");
    if (NULL != unit_test_suite->m_suite_name)
        {
        printf("%s\n", unit_test_suite->m_suite_name);
        }
    printf("TEST_SUITE_RANDOM_SEED=%i\n", (int)random_seed);
    printf("ITERATION_COUNT=%i\n", (int)iteration_count);

    cf00_init_unit_test_data(&d, unit_test_suite, random_seed, iteration_count);
    indiv_unit_test_data = d.m_indiv_test_data;

    for (d.m_current_unit_test_idx = 0; 
        d.m_current_unit_test_idx < unit_test_suite->m_test_count;
        ++d.m_current_unit_test_idx, ++indiv_unit_test_data)
    {
        current_unit_test = 
            &((unit_test_suite->m_tests)[d.m_current_unit_test_idx]);

        /* record start time */
        start_time = time(NULL);
        
        for (d.m_current_iteration_idx = 0; 
            d.m_current_iteration_idx < iteration_count;
            ++d.m_current_iteration_idx)
        {
            /* clear temp variables */
        
            /* cf00_reset_unit_test_rand_gen(), but do not reset
            global random seed or any other parameters      */
     
       
     
            /* run one iteration */
            (*(current_unit_test->m_func))(&d);
        }

        /* record stop time */
        stop_time = time(NULL);

        /* record time for current unit test */
        indiv_unit_test_data->m_elapsed_time = stop_time - start_time;
        d.m_total_elapsed_time += indiv_unit_test_data->m_elapsed_time; 
    
        /* report unit test results */
        printf("TEST:%s  ASSERTIONS:%i  ERRORS:%i  TIME:%i\n",
            current_unit_test->m_func_name, 
            (int)(indiv_unit_test_data->m_assertion_count),
            (int)(indiv_unit_test_data->m_error_count),
            (int)(indiv_unit_test_data->m_elapsed_time));
              
    }


    printf("RUN TEST SUITE END\n");
    
    /* print summary */

   
    // deconstruct unit test data
    cf00_free_unit_test_data(&d);

    return EXIT_SUCCESS; /* return total error count */   

}



