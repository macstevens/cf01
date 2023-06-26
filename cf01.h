/** cf01.h

This file is C++ source code for including C-flat features into a C++ program


Copyright (c) 2021 Mac Stevens <stevensm@earthlink.net> <www.macstevens.net>

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

Reference: https://opensource.org/licenses/ISC
*/

#ifndef CF01_H
#define CF01_H

#include <iostream>
#include <limits>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <string>


/* Call CF01_SET_JRNL_WRITE_MODE_(OFF/ON_ERROR/ON) early in the startup sequence.
*/
#define CF01_SET_JRNL_WRITE_MODE( _jrnl_write_mode ) {              \
    cf01_auto_assert_wksp::get_instance()->set_jrnl_write_mode(     \
        _jrnl_write_mode, __FILE__, __LINE__, __FUNCTION__ );       \
    }

/* set mode = OFF
    disables hash consistency check
    disables saving auto-assertion results
*/
#define CF01_SET_JRNL_WRITE_MODE_OFF()                              \
    CF01_SET_JRNL_WRITE_MODE( CF01_JRNL_WRITE_MODE_OFF )

/* set mode = ON-ERROR
    disables hash consistency check
    enables saving auto-assertion results
*/
#define CF01_SET_JRNL_WRITE_MODE_ON_ERROR()                         \
    CF01_SET_JRNL_WRITE_MODE( CF01_JRNL_WRITE_MODE_ON_ERROR )

/* set mode = ON
    enables hash consistency check
    enables saving auto-assertion results
*/
#define CF01_SET_JRNL_WRITE_MODE_ON()                               \
    CF01_SET_JRNL_WRITE_MODE( CF01_JRNL_WRITE_MODE_ON )

/* call at beginning of function */
#define CF01_AA_INCR_CALL_DEPTH() \
    { cf01_auto_assert_wksp::get_instance()->incr_call_depth(       \
        __FILE__, __LINE__, __FUNCTION__ ); }

/* call at end of function */
#define CF01_AA_DECR_CALL_DEPTH() \
    {cf01_auto_assert_wksp::get_instance()->decr_call_depth();}

/* Place CF01_AUTO_ASSERT(_condition) wherever an invariant function
  would go.
_condition - boolean value or function.  TRUE=> data is ok, FALSE=>some error
*/
#define CF01_AUTO_ASSERT( _condition ) {                                   \
    cf01_auto_assert_wksp::get_instance()->incr_p_count();                 \
    if (cf01_auto_assert_wksp::get_instance()->should_run_aasrt()){        \
        bool _result = (_condition);                                       \
        cf01_auto_assert_wksp::get_instance()->report_aasrt_result(        \
            _result, __FILE__, __LINE__, __FUNCTION__, #_condition );      \
        }                                                                  \
    }

/* true => should run extra debug code */
#define CF01_AA_SHOULD_RUN_XDBG( _dbg_lvl ) \
    (cf01_auto_assert_wksp::get_instance()->should_run_xdbg(_dbg_lvl))

/* run extra debug assertion, if debug level meets criteria */
#define CF01_AA_XDBG_ASSERT( _condition, _dbg_lvl ){                    \
    if(CF01_AA_SHOULD_RUN_XDBG(_dbg_lvl)){                              \
        bool _result = (_condition);                                    \
        cf01_auto_assert_wksp::get_instance()->report_aasrt_result(     \
            _result, __FILE__, __LINE__, __FUNCTION__, #_condition);    \
        }                                                               \
    }

/* Place CF01_HASH_CONSISTENCY_CHECK(_hash) in code that should produce
repeatable results.

_hash - hash value or function.  type = unsigned integer, 64-bits or smaller

Has function should be computed from current state of algorithm, current
data, etc.  Subsequent runs with same input data should generate identical
sequence of hash values.
*/
#define CF01_HASH_CONSISTENCY_CHECK( _hash ) {                             \
    if (cf01_auto_assert_wksp::get_instance()->should_run_hc_check()){     \
        cf01_uint64 _h = static_cast<cf01_uint64>( _hash );                \
        cf01_auto_assert_wksp::get_instance()->hash_consistency_check(     \
            _h, __FILE__, __LINE__, __FUNCTION__, #_hash );                \
        }                                                                  \
    cf01_auto_assert_wksp::get_instance()->incr_hc_chk_index();            \
    }

#define CF01_AA_ERR_BUF() \
    (cf01_auto_assert_wksp::get_instance()->get_utility_err_buf())

#define CF01_AA_ERR_BUF_CAPACITY() \
    (cf01_auto_assert_wksp::get_instance()->get_utility_err_buf_capacity())

#define CF01_AA_ERR_BUF_POS_PTR() \
    (cf01_auto_assert_wksp::get_instance()->get_utility_err_buf_pos_ptr())

#define CF01_AA_WKSP_OUTPUT( _os ) \
    (cf01_auto_assert_wksp::get_instance()->ostream_output(_os))


typedef int8_t    cf01_int8;
typedef uint8_t   cf01_uint8;
typedef int16_t   cf01_int16;
typedef uint16_t  cf01_uint16;
typedef int32_t   cf01_int32;
typedef uint32_t  cf01_uint32;
typedef int64_t   cf01_int64;
typedef uint64_t  cf01_uint64;
typedef double    cf01_float64;

typedef cf01_uint64 cf01_depth_t;

enum cf01_jrnl_write_mode {
    CF01_JRNL_WRITE_MODE_OFF = 0,
    CF01_JRNL_WRITE_MODE_ON_ERROR = 1,
    CF01_JRNL_WRITE_MODE_ON = 2,
    CF01_JRNL_WRITE_MODE_COUNT,
    
    CF01_JRNL_WRITE_MODE_DEFAULT = CF01_JRNL_WRITE_MODE_ON_ERROR
    };

/* debug levels
  level 0 => code is always executed
  level 1 => 10% of the code can be run without hampering run time too much
  level 2 => 1%
  level 3 => 0.1%
*/
enum cf01_aa_debug_level {
    CF01_AA_DEBUG_LEVEL_0 = 0,
    CF01_AA_DEBUG_LEVEL_1 = 1,
    CF01_AA_DEBUG_LEVEL_2 = 2,
    CF01_AA_DEBUG_LEVEL_3 = 3
    };

/* at a given call index range and given depth range, criteria:
run invariant when (call_idx % div) == mod */
struct cf01_call_idx_range_crit
{
public:
    cf01_uint64    m_call_idx_range_end; /* (range max value) + 1 */
    cf01_uint64    m_call_idx_div; /* divisor */
    cf01_uint64    m_call_idx_mod; /* remainder */
};



#define CF01_AASRT_CALL_DEPTH_COUNT (64)
#define CF01_AASRT_CALL_IDX_RANGE_COUNT (16)

/* auto-assert assertion result */
struct cf01_aasrt_result
{
public:
    /* true => assertion was executed */
    bool m_done;

    /* true => assertion passed */
    bool m_pass;

    /* name of source code file where assertion was called */
    std::string m_file_name;

    /* name of function where assertion was called */
    std::string m_func_name;

    /* line of file where assertion was called */
    int m_line_num;

    /* call depth when assertion was executed */
    cf01_depth_t m_call_depth; 

    /* total number of standard (not extra debug) auto-assertions
    ("points") either skipped or executed at given depth.
    This includes the assertion whose result this struct represents. */
    cf01_uint64 m_aasrt_p_count[CF01_AASRT_CALL_DEPTH_COUNT]; 

    /* transition count for each call depth, when assertion was executed */
    cf01_uint64 m_curr_t_count[CF01_AASRT_CALL_DEPTH_COUNT];

    /* error message generated by assertion */
    std::string m_err_msg;

public:
    cf01_aasrt_result();
   ~cf01_aasrt_result();
};


struct cf01_hc_chk_cmd_info
{
public:
    /* true => this struct has been initialized with non-default values */
    bool m_initialized;

    /* check index, should match cf01_hc_chk_rcrd */
    cf01_uint64 m_check_index;

    /* line number where hash consistency check was called */
    int m_line_num;

    /* source code file where hash consistency check was called */
    #define CF01_HC_CHK_CMD_INFO_FILE_NAME_BUF_SIZE (255)
    char m_file_name[CF01_HC_CHK_CMD_INFO_FILE_NAME_BUF_SIZE];

    /* function where hash consistency check was called */
    #define CF01_HC_CHK_CMD_INFO_FUNC_NAME_BUF_SIZE (255)
    char m_func_name[CF01_HC_CHK_CMD_INFO_FUNC_NAME_BUF_SIZE];

    /* string containing hash value or function returning hash value */
    #define CF01_HC_CHK_CMD_INFO_HASH_CMD_STR_BUF_SIZE (255)
    char m_hash_cmd_str[CF01_HC_CHK_CMD_INFO_HASH_CMD_STR_BUF_SIZE];

public:
    void reset(){
        m_initialized = false;
        m_check_index = 0;
        m_line_num = 0;
        memset(m_file_name, 0, sizeof(m_file_name));
        memset(m_func_name, 0, sizeof(m_func_name));
        memset(m_hash_cmd_str, 0, sizeof(m_hash_cmd_str));
        }

    void init(const char *file_name, const int line_num, const char *function,
        const char *hash_cmd_str, const cf01_uint64& check_index );
};

enum cf01_hc_chk_rcrd_type{
    CF01_HC_CHK_RCRD_TYPE_NONE = 0, /* on free chain */
    CF01_HC_CHK_RCRD_TYPE_GUIDE,
    CF01_HC_CHK_RCRD_TYPE_DONE_THIS_RUN,
    CF01_HC_CHK_RCRD_TYPE_DONE_PREV_RUN,

    CF01_HC_CHK_RCRD_TYPE_COUNT
};


/* hash consistency check record 

One record for each check done
Forms singly-linked list
*/
struct cf01_hc_chk_rcrd
{
public:
    /* type */
    cf01_hc_chk_rcrd_type m_rcrd_type;

    /* true => check result does not match previous run */
    bool m_mismatch_prev_run;

    /* time stamp */
    time_t m_time_stamp;

    /* check index. separate check index sequence for each depth */
    cf01_uint64 m_check_index;

    /* hash value  */
    cf01_uint64 m_hash;

    /* info for where hash check was called.  This should only be initialized
    (non-NULL) for the first failed hash check at this depth. */
    cf01_hc_chk_cmd_info *m_cmd_info;

    /* next record in list */
    cf01_hc_chk_rcrd *m_next;
public:
    void reset(){
        m_rcrd_type = CF01_HC_CHK_RCRD_TYPE_NONE;
        m_mismatch_prev_run = false;
        m_time_stamp = 0;
        m_check_index = 0;
        m_hash = 0;
        m_cmd_info = NULL;
        m_next = NULL;
        }

    void init( const cf01_hc_chk_rcrd& master ){
        m_rcrd_type = master.m_rcrd_type;
        m_mismatch_prev_run = master.m_mismatch_prev_run;
        m_time_stamp = master.m_time_stamp;
        m_check_index = master.m_check_index;
        m_cmd_info = NULL;
        m_hash = master.m_hash;
        }

    void ostream_output(std::ostream& os, const std::string& line_prefix) const;
};


struct cf01_hc_chk_depth_rcrd
{
public:
    /* check index. separate check index sequence for each depth */
    cf01_uint64 m_check_index;

    /* list of unpublished records of checks done this run */
    cf01_hc_chk_rcrd *m_unpub_rcrds;

    /* list of records of checks done previously and guide records.  This list
      is consumed as checks are done such that the start of the list is
      relevant for determining which check to run next. 
      */
    cf01_hc_chk_rcrd *m_guide_rcrds;

    /* deadline for running next check */
    time_t m_next_check_deadline;

    /* run next check when index reaches this value */
    cf01_uint64 m_next_check_index;

    /* record for first hash consistency check failure */
    cf01_hc_chk_cmd_info m_first_fail_cmd_info;

    /* record for debugging infinite loops */
    cf01_hc_chk_cmd_info m_misc_cmd_info;
};



/* todo: future improvements:
    distinguish between 1) an assertion that checks a subset of conditions and
    2) an assertion that is equivalent to a comprehensive set of checks (invariant)  */
class cf01_auto_assert_wksp
{
private:
    /* journal write mode */
    cf01_jrnl_write_mode m_jrnl_write_mode;

    /* true => journal has been written to, this run */
    bool m_jrnl_write_started; 

    /* current call depth, restricted to array range */
    cf01_depth_t m_call_depth; 

    /* current call depth */
    cf01_depth_t m_call_depth_unlimited; 

    /* total number of standard (not extra debug) assertions ("points")
    reached at each depth.
    This includes all those assertions that were either skipped or executed
    plus, if any, the current assertion which is about to be skipped or
    executed or is currently being skipped or executed. */
    cf01_uint64 m_curr_p_count[CF01_AASRT_CALL_DEPTH_COUNT]; 

    /* transition is the action of incrementing call depth */
    /*current transition count for each call depth 

    for each call depth, the number of transitions down
    to that depth.  i.e., the number of function calls
    resulting in going down to that depth */
    cf01_uint64 m_curr_t_count[CF01_AASRT_CALL_DEPTH_COUNT]; 

    /* A: Standard Assertions

    These are assertions that are executed on a fixed schedule, regardless
    of debug level */

    /* control criteria, determining whether to run standard (non extra debug) 
    auto assertion */
    cf01_call_idx_range_crit m_depth_call_idx_range_crit
                               [CF01_AASRT_CALL_DEPTH_COUNT]
                               [CF01_AASRT_CALL_IDX_RANGE_COUNT];

    /* for each call depth d, the index i for m_depth_call_idx_range_crit[d][i]
    specifying the current control criteria that should be used */
    cf01_uint8 m_idx_crit_idx[CF01_AASRT_CALL_DEPTH_COUNT]; 



    /* B: extra debug assertions and supporting code 

    These are assertions and supporting code that are only executed if
    the debug level is high enough according to the following if statement:

    if( m_curr_t_count[m_call_depth] >= m_xdbg_ctrl_t_count[m_call_depth]
        && dbg_lvl <= m_xdbg_ctrl_dbg_lvl[m_call_depth] )
        {
        should execute extra debug assertion or code.
        } 
    */

    /* controls when extra debug code and assertions will be executed.
    This is the minimum total transition count at each call depth.
    If total transition count is below this value, then no extra debug
    code or assertions are run.  If total transition count is at or above
    this value, then extra debug code with debug level at or below
     m_xdbg_ctrl_dbg_lvl[d] is run. */
    cf01_uint64 m_xdbg_ctrl_t_count[CF01_AASRT_CALL_DEPTH_COUNT];

    /* controls when extra debug code and assertions will be executed.
    Extra debug code and assertions with debug level at or below
    m_xdbg_ctrl_dbg_lvl[d] will be executed when transition count is at or 
    above m_xdbg_ctrl_t_count[d] */
    cf01_uint8 m_xdbg_ctrl_dbg_lvl[CF01_AASRT_CALL_DEPTH_COUNT]; 

    /* info for last assertion that was run and passed */
    cf01_aasrt_result m_aasrt_result_last_passed;

    /* info for first assertion that failed */
    cf01_aasrt_result m_aasrt_result_first_failed;

    /* utility buffer */
    #define CF01_AA_UTILITY_ERR_BUF_CAPACITY (2047)
    char m_utility_err_buf[CF01_AA_UTILITY_ERR_BUF_CAPACITY];
    size_t m_utility_err_buf_pos;

    /* hash consistency check */
    time_t m_start_time;
    time_t m_publish_deadline;
    cf01_hc_chk_depth_rcrd m_hc_chk_depth_rcrds[CF01_AASRT_CALL_DEPTH_COUNT];
    #define CF01_AA_HC_CHK_RCRD_POOL_SZ (2047)
    cf01_hc_chk_rcrd m_hc_chk_rcrd_pool[CF01_AA_HC_CHK_RCRD_POOL_SZ];
    cf01_hc_chk_rcrd *m_hc_chk_rcrd_free_chain;

    static cf01_auto_assert_wksp m_instance; /* singleton */
    static const char * const m_jrnl_file_name;

public:
    static cf01_auto_assert_wksp *get_instance() { return &m_instance; }

    cf01_auto_assert_wksp();
   ~cf01_auto_assert_wksp();

    /* initialize */
    void set_jrnl_write_mode( const cf01_jrnl_write_mode& m,
        const char *file_name = NULL, const int line_num = 0,
        const char *function = NULL);
    void init_aa_default();
    void init_hc_chk();
    void init_from_journal_file();
    void add_aasrt_call_idx_range_crit( const cf01_depth_t& depth,
        const cf01_call_idx_range_crit* idx_range_crit, bool *result,
        std::string *err_str );
    void complete_depth_call_idx_range_crit_table();
    void add_hc_chk_rcrd( const cf01_depth_t& depth, 
        const cf01_hc_chk_rcrd* hc_chk_rcrd, bool *result,
        std::string *err_str );

    /* initialize with default values */
    /* initialize from journal file */
    /* save m_aasrt_result_first_failed to journal file */
    /* save m_aasrt_result_last_passed to journal file */


    /* Increment call depth. call at beginning of function */
    void incr_call_depth( const char *file_name = NULL, const int line_num = 0,
        const char *function = NULL ){
        if( m_call_depth_unlimited < std::numeric_limits<cf01_uint64>::max() ){
            ++m_call_depth_unlimited; }
        m_call_depth =
            (m_call_depth_unlimited < (CF01_AASRT_CALL_DEPTH_COUNT-1)) ?
            m_call_depth_unlimited : (CF01_AASRT_CALL_DEPTH_COUNT-1);
        ++(m_curr_t_count[m_call_depth]);
        if( m_call_depth_unlimited > (CF01_AASRT_CALL_DEPTH_COUNT-1) ){
            static cf01_depth_t max_d = 0;
            if( m_call_depth_unlimited > max_d ){    
                max_d = m_call_depth_unlimited;
                static int printf_count = 0;
                static const int max_printf_count = 64;
                if( printf_count < max_printf_count ){
                    std::cout << "CF01 incr_call_depth() m_call_depth_unlimited = "
                        << m_call_depth_unlimited << " > "
                        << (CF01_AASRT_CALL_DEPTH_COUNT-1)
                        << "  file="
                        << ( ( NULL == file_name ) ? "" : file_name )
                        << "  line_num=" << line_num
                        << "  function=" << (( NULL == function ) ? "" : function)
                        << "\n";
                    ++printf_count;
                    }
                }
            }
        }

    /* Decrement call depth. call at end of function */
    void decr_call_depth() {
        m_call_depth_unlimited = (m_call_depth_unlimited > 0) ?
            m_call_depth_unlimited - 1 : 0; 
        m_call_depth =
            (m_call_depth_unlimited < (CF01_AASRT_CALL_DEPTH_COUNT-1)) ?
            m_call_depth_unlimited : (CF01_AASRT_CALL_DEPTH_COUNT-1);
        }

    cf01_depth_t get_call_depth() const { return m_call_depth; }
    cf01_depth_t get_call_depth_unlimited() const {
        return m_call_depth_unlimited; }

    void incr_p_count();
    bool should_run_aasrt() const;
    bool should_run_xdbg(const cf01_uint8& dbg_lvl) const;
    void report_aasrt_result( const bool result, const char *file_name,
        const int line_num, const char *function, const char *condition_str);

    /* hash consistency check */
    void incr_hc_chk_index();
    bool should_run_hc_check() const{
        assert( m_call_depth < CF01_AASRT_CALL_DEPTH_COUNT );
        const cf01_hc_chk_depth_rcrd *hc_chk_depth_rcrd =
            &( m_hc_chk_depth_rcrds[m_call_depth] );
        bool should_run = false;
        if( hc_chk_depth_rcrd->m_check_index >= hc_chk_depth_rcrd->m_next_check_index ){
            should_run = true;
            }
        else{
            const time_t now = time(NULL);
            if( now >= hc_chk_depth_rcrd->m_next_check_deadline ){
                should_run = true;
                }
            }
        return should_run;
        }
    void hash_consistency_check( const cf01_uint64 hash, const char *file_name,
        const int line_num, const char *function, const char *hash_cmd_str);
    static cf01_uint64 default_hash( const cf01_uint64& start,
        const cf01_uint8 *bytes, const size_t& byte_count );

    /* utility buffer */
    char *get_utility_err_buf() { return &(m_utility_err_buf[0]); }
    size_t get_utility_err_buf_capacity() const {
        return CF01_AA_UTILITY_ERR_BUF_CAPACITY; }
    size_t *get_utility_err_buf_pos_ptr() { return &m_utility_err_buf_pos; }
    void clear_utility_err_buf() {
        m_utility_err_buf_pos=0; m_utility_err_buf[0]=0x0; }

    std::ostream& ostream_output(std::ostream& os) const;
    int verify_data( std::string *err_str ) const;

private:
    void write_journal_start();
    void write_journal_end();
    void write_journal_start_end( const std::string& start_end_tag);

    /* auto-assert */
    void save_aasrt_result_to_file(const cf01_aasrt_result *r);

    /* hash consistency check */
    cf01_hc_chk_rcrd *alloc_hc_chk_rcrd();
    void free_hc_chk_rcrd(cf01_hc_chk_rcrd *rcrd);
    void free_hc_chk_rcrd_chain(cf01_hc_chk_rcrd *rcrd);
    static void reverse_hc_chk_rcrd_list( cf01_hc_chk_rcrd **start );
    static bool is_hc_chk_rcrd_list_fwd_sequence( const cf01_hc_chk_rcrd *r );
    static bool is_hc_chk_rcrd_list_rev_sequence( const cf01_hc_chk_rcrd *r );
    void clear_hc_chk_depth_rcrds();
    static cf01_uint64 calc_next_check_index(
        const cf01_uint64& curr_check_index );
    static time_t calc_next_check_deadline(
        const cf01_uint64& curr_check_index, const time_t& now );
    void publish_hc_chk_rcrds();
    void publish_hc_chk_rcrd(const cf01_depth_t& depth,
        const cf01_hc_chk_rcrd *r);
};

template <typename Obj>
cf01_uint64 cf01_obj_hash( const cf01_uint64 &h_in, const Obj& obj ){
    const cf01_uint64 h = cf01_auto_assert_wksp::default_hash( h_in,
        reinterpret_cast<const cf01_uint8 *>(&obj), sizeof(obj) );
    return h;
}

std::string cf01_ctime( const time_t& t );
std::string cf01_uint64_to_hex_str( const cf01_uint64& u );
cf01_uint64 cf01_hex_str_to_uint64( const std::string& s );


#endif /* CF01_H */
