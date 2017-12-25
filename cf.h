/** cf.h

This file is C source code for parsing and manipulating a C-flat program
*/

#ifndef CF_H
#define CF_H

#include <stdio.h>

typedef void *void_ptr;
typedef unsigned char boolean;
typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;
typedef long long int64;
typedef unsigned long long uint64;
typedef double float64;


#define CF00_SINGLE_THREAD (1)

#if (defined(CF00_SINGLE_THREAD))
/* single-threaded */
#define CF00_MAX_INV_CALL_DEPTH_ST (63)
extern uint8 cf00_inv_call_depth_st;
extern uint64 cf00_invariant_count_st[CF00_MAX_INV_CALL_DEPTH_ST];

/* call at beginning of function */
#define CF00_INCR_INV_CALL_DEPTH_ST() { cf00_inv_call_depth_st = \
    (cf00_inv_call_depth_st < CF00_MAX_INV_CALL_DEPTH_ST) ? \
    cf00_inv_call_depth_st + 1 : CF00_MAX_INV_CALL_DEPTH_ST; }

/* call at end of function */
#define CF00_DECR_INV_CALL_DEPTH_ST() { cf00_inv_call_depth_st = \
    (cf00_inv_call_depth_st > 0) ? cf00_inv_call_depth_st - 1 : 0; }

#define CF00_INCR_INVARIANT_COUNT_AT_CALL_DEPTH_ST()  \
    ++(cf00_invariant_count_st[cf00_inv_call_depth_st])
#define CF00_SHOULD_RUN_INVARIANT(_file_name, _line_num, _func_name, _cost) \
    cf00_should_run_invariant_st(_file_name, _line_num, _func_name, _cost)
#define CF00_REPORT_INVARIANT_FAILURE(_file_name, _line_num, _func_name, _is_data_ok_str) \
    cf00_report_invariant_failure_st(_file_name, _line_num, _func_name, _is_data_ok_str)

#else
/* multi-threaded */
#define CF00_INCR_INV_CALL_DEPTH_ST()
#define CF00_DECR_INV_CALL_DEPTH_ST()
#define CF00_INCR_INVARIANT_COUNT_AT_CALL_DEPTH_ST()
#define CF00_SHOULD_RUN_INVARIANT(_file_name, _line_num, _func_name, _cost) \
    cf00_should_run_invariant_mt(_file_name, _line_num, _func_name, _cost)
#define CF00_REPORT_INVARIANT_FAILURE(_file_name, _line_num, _func_name, _is_data_ok_str) \
    cf00_report_invariant_failure_mt(_file_name, _line_num, _func_name, _is_data_ok_str)
#endif /* CF00_SINGLE_THREAD */

/* Place CF00_INVARIANT(_is_data_ok, _cost) wherever an invariant function
  would go.
_is_data_ok - boolean value or function.  TRUE=> data is ok, FALSE=>some error
_cost - uint32 value or function.  0=> very cheap or very important to run invariant
  high value => expensive to run invariant
*/
#define CF00_INVARIANT( _is_data_ok, _cost )                               \
    {                                                                      \
    CF00_INCR_INV_COUNT_AT_CALL_DEPTH_ST()                                 \
    if (CF00_SHOULD_RUN_INVARIANT(__FILE__, __LINE__, __FUNCTION__,_cost)) \
        {                                                                  \
        boolean result = _is_data_ok;                                      \
        if (!result)                                                       \
            {                                                              \
            CF00_REPORT_INVARIANT_FAILURE(                                 \
                __FILE__, __LINE__, __FUNCTION__, ##is_data_ok)            \
            }                                                              \
        }                                                                  \
    }


struct cf00_inv_crit_a_call_idx_range;
struct cf00_inv_crit_a_call_idx_range_allocator;
struct cf00_inv_crit_a_depth_range;
struct cf00_inv_crit_a_depth_range_allocator;
struct cf00_run_invariant_criteria_a;

typedef struct cf00_inv_crit_a_call_idx_range
              *cf00_inv_crit_a_call_idx_range_ptr;
typedef struct cf00_inv_crit_a_call_idx_range_allocator
              *cf00_inv_crit_a_call_idx_range_allocator_ptr;
typedef struct cf00_inv_crit_a_depth_range
              *cf00_inv_crit_a_depth_range_ptr;
typedef struct cf00_inv_crit_a_depth_range_allocator
              *cf00_inv_crit_a_depth_range_allocator_ptr;
typedef struct cf00_run_invariant_criteria_a
              *cf00_run_invariant_criteria_a_ptr;

/* at a given call index range and given depth range, criteria:
run invariant when (call_idx % div) == mod */
typedef struct cf00_inv_crit_a_call_idx_range
{
    cf00_inv_crit_a_call_idx_range_allocator_ptr    m_allocator;
    cf00_inv_crit_a_depth_range_ptr                 m_depth_range; /* owner */
    cf00_inv_crit_a_call_idx_range_ptr              m_prev;
    cf00_inv_crit_a_call_idx_range_ptr              m_next;
    uint64                                          m_call_idx_range_end; /* (range max value) + 1 */
    uint64                                          m_call_idx_div; /* divisor */
    uint64                                          m_call_idx_mod; /* remainder */
} cf00_inv_crit_a_call_idx_range;

#define CF00_INV_CRIT_A_CALL_IDX_RANGE_ALLOC_BLOCK_SIZE (128)
typedef struct cf00_inv_crit_a_call_idx_range_allocator
{
    struct cf00_inv_crit_a_call_idx_range   m_block[CF00_INV_CRIT_A_CALL_IDX_RANGE_ALLOC_BLOCK_SIZE];
    cf00_inv_crit_a_call_idx_range_ptr      m_free_chain;
} cf00_inv_crit_a_call_idx_range_allocator;


void cf00_inv_crit_a_call_idx_range_alloc_init(
    cf00_inv_crit_a_call_idx_range_allocator_ptr a);
void cf00_inv_crit_a_call_idx_range_alloc_clear(
    cf00_inv_crit_a_call_idx_range_allocator_ptr a);
cf00_inv_crit_a_call_idx_range_ptr cf00_allocate_inv_crit_a_call_idx_range(
    cf00_inv_crit_a_call_idx_range_allocator_ptr a);
void cf00_free_inv_crit_a_call_idx_range(cf00_inv_crit_a_call_idx_range_ptr r);
void cf00_inv_crit_a_call_idx_range_alloc_debug_dump(
    cf00_inv_crit_a_call_idx_range_allocator_ptr a);
uint64 cf00_inv_crit_a_call_idx_range_alloc_verify_data(
    const cf00_inv_crit_a_call_idx_range_allocator *a, char *err_msg,
    const size_t max_err_msg_len);


/* range of depths and criteria for running invariant for that depth range */
typedef struct cf00_inv_crit_a_depth_range
{
    cf00_inv_crit_a_depth_range_allocator_ptr   m_allocator;
    cf00_run_invariant_criteria_a_ptr           m_inv_crit_a; /* owner */
    cf00_inv_crit_a_depth_range_ptr             m_prev;
    cf00_inv_crit_a_depth_range_ptr             m_next;
    cf00_inv_crit_a_call_idx_range_ptr          m_call_idx_range_head;
    cf00_inv_crit_a_call_idx_range_ptr          m_call_idx_range_tail;
    uint64                                      m_depth_range_end; /* (depth range max value) + 1 */
} cf00_inv_crit_a_depth_range;

typedef struct cf00_inv_crit_a_depth_range_allocator
{
    struct cf00_inv_crit_a_depth_range  m_block[32];
    cf00_inv_crit_a_depth_range_ptr     m_free_chain;
} cf00_inv_crit_a_depth_range_allocator;


void cf00_inv_crit_a_depth_range_alloc_init(
    cf00_inv_crit_a_depth_range_allocator_ptr a);
void cf00_inv_crit_a_depth_range_alloc_clear(
    cf00_inv_crit_a_depth_range_allocator_ptr a);
cf00_inv_crit_a_depth_range_ptr cf00_allocate_inv_crit_a_depth_range(
    cf00_inv_crit_a_depth_range_allocator_ptr a);
void cf00_free_inv_crit_a_depth_range(cf00_inv_crit_a_depth_range_ptr r);
void cf00_inv_crit_a_depth_range_debug_dump(
    cf00_inv_crit_a_depth_range_allocator_ptr a);
uint64 cf00_inv_crit_a_depth_range_verify_data(
    const cf00_inv_crit_a_depth_range_allocator *a, char *err_msg,
    const size_t max_err_msg_len);

/*
m_depth_range_array
+---+---+---+ - +---+---+
| 0 | 1 | 2 |...| 63| 64|
+---+---+---+ - +---+---+   +-------------+
  |   |   |       |   +---->| depth range |
  |   |   |       +-------->+-------------+
  |   |   +-------------------------->+-------------+
  |   +---------->+-------------+     | depth range |
  +-------------->| depth range |     +-------------+
                  +-------------+
*/
typedef struct cf00_run_invariant_criteria_a
{
    cf00_inv_crit_a_call_idx_range_allocator m_call_idx_range_alloc;
    cf00_inv_crit_a_depth_range_allocator m_depth_range_alloc;
    cf00_inv_crit_a_depth_range_ptr m_depth_range_array[CF00_MAX_INV_CALL_DEPTH_ST];
} cf00_run_invariant_criteria_a;


void cf00_run_inv_crit_a_init(cf00_run_invariant_criteria_a_ptr c);
void cf00_run_inv_crit_a_clear(cf00_run_invariant_criteria_a_ptr c);
void cf00_run_inv_crit_a_debug_dump(const cf00_run_invariant_criteria_a *c);
uint64 cf00_run_inv_crit_a_verify_data(const cf00_run_invariant_criteria_a *c, char *err_msg,
    const size_t max_err_msg_len);


/* result for a particular depth */
typedef struct cf00_run_invariant_depth_result_a
{
    uint64 m_total_inv_count; /* number of invariants that were run + number of invariants that were skipped */
    uint64 m_inv_run_count;   /* number of invariants that were run */
    uint64 m_inv_fail_count;  /* number of invariants that were run and failed */
    uint64  m_fail_call_idx_array[16]; /* call idx for each of first 16 invariant failures */  
} cf00_run_invariant_depth_result_a;

/* results for all depths */
typedef struct cf00_run_invariant_result_a
{
    cf00_run_invariant_depth_result_a   m_depth_result_array[CF00_MAX_INV_CALL_DEPTH_ST];
} cf00_run_invariant_result_a;



typedef enum
{
    CF00_OT_OBJECT = 0,
    CF00_OT_PROCEDURE,

    CF00_OT_XYZABC,     /* template code */

    CF00_OT_COUNT
} cf00_managed_object_type;

typedef enum
{
    CF00_RPCT_NONE = 0,
    CF00_RPCT_ARRAY_3,
    CF00_RPCT_LIST,
    CF00_RPCT_TREE,

    CF00_RPCT_COUNT
} cf00_reverse_pointer_container_type;



/* TODO: add global allocator
  struct cf00_global_allocator {
    cf00_string_allocator *m_string_alloc;
    cf00_xyzabc_allocator *m_xyzabc_alloc;
  }

  Every allocator points to its parent global allocator */

struct cf00_string_allocator;

typedef struct cf00_string
{
    struct cf00_string_allocator *m_allocator;
    char *m_char_buf;
    uint32 m_length;
    uint32 m_capacity;
} cf00_string;

void cf00_str_init(cf00_string *s);
void cf00_str_clear(cf00_string *s);
int cf00_str_compare(const cf00_string *x, const cf00_string *y);
void cf00_str_resize(cf00_string *s, const uint32 new_sz);
void cf00_str_reserve(cf00_string *s, const uint32 new_cap);
void cf00_str_assign(cf00_string *dest, const cf00_string *src);
void cf00_str_assign_from_char_ptr(cf00_string *dest, const char *src);
void cf00_str_append(cf00_string *s, const cf00_string *addition);
void cf00_str_append_char_buf(cf00_string *s, const char *addition);
uint64 cf00_str_verify_data(const cf00_string *s, char *err_msg,
    const size_t max_err_msg_len);


typedef struct cf00_str_vec
{
    struct cf00_string_allocator *m_allocator;
    cf00_string **m_str_array;
    uint32 m_size;
    uint32 m_capacity;
} cf00_str_vec;

void cf00_str_vec_init(cf00_str_vec *sv);
void cf00_str_vec_clear(cf00_str_vec *sv);
int cf00_str_vec_compare(const cf00_str_vec *x, const cf00_str_vec *y);
void cf00_str_vec_resize(cf00_str_vec *sv, const uint32 new_sz);
void cf00_str_vec_reserve(cf00_str_vec *sv, const uint32 new_cap);
void cf00_str_vec_push_back(cf00_str_vec *sv, cf00_string *s);
void cf00_str_vec_push_back_copy(cf00_str_vec *sv, const cf00_string *s);
void cf00_str_vec_push_back_format(cf00_str_vec *sv, const char *fmtstr, ...);
uint64 cf00_str_vec_verify_data(const cf00_str_vec *sv, char *err_msg,
    const size_t max_err_msg_len);

typedef enum { CF00_ALLOC_BLOCK_SIZE_A = 0x1FE0 } cf00_alloc_block_sz;

typedef struct cf00_alloc_block_a
{
    uint8 m_raw_memory[CF00_ALLOC_BLOCK_SIZE_A];
    struct cf00_alloc_block_a *m_next_alloc_block;
} cf00_alloc_block_a; 

typedef struct cf00_string_allocator
{
    struct cf00_alloc_block_a *m_alloc_block_chain;
    char *m_free_chain_char_buf_16;
    char *m_free_chain_char_buf_32;
    char *m_free_chain_char_buf_64;
    char *m_free_chain_char_buf_128;
    char *m_free_chain_char_buf_256;
    char *m_free_chain_char_buf_512;
    cf00_string *m_free_chain_string;
    cf00_string **m_free_chain_string_ptr_array_4;
    cf00_string **m_free_chain_string_ptr_array_8;
    cf00_string **m_free_chain_string_ptr_array_16;
    cf00_string **m_free_chain_string_ptr_array_32;
    cf00_str_vec *m_free_chain_str_vec;
} cf00_string_allocator;

void cf00_str_alloc_init(cf00_string_allocator *a);
void cf00_str_alloc_clear(cf00_string_allocator *a);
cf00_string *cf00_allocate_string(cf00_string_allocator *a);
cf00_str_vec *cf00_allocate_str_vec(cf00_string_allocator *a);
void cf00_free_string(cf00_string *s);
void cf00_free_str_vec(cf00_str_vec *sv);
void cf00_str_alloc_debug_dump(cf00_string_allocator *a);
uint64 cf00_str_alloc_verify_data(const cf00_string_allocator *a, char *err_msg,
    const size_t max_err_msg_len);

typedef void *cf00_void_ptr_array3[3];


typedef struct cf00_ptr_list_node
{
    void *m_ptr;
    struct cf00_ptr_list_node *m_prev;
    struct cf00_ptr_list_node *m_next;
} cf00_ptr_list_node;

typedef struct cf00_ptr_list
{
    void *m_cf00_ptr_list_node_allocator;
    cf00_ptr_list_node *m_head;
    uint32 m_size;
} cf00_ptr_list;

void cf00_ptr_list_init(cf00_ptr_list *p);
void cf00_ptr_list_clear(cf00_ptr_list *p);
void cf00_ptr_list_push_back(cf00_ptr_list *obj_data, void *p);
uint64 cf00_ptr_list_verify_data(const cf00_ptr_list *pl, 
    cf00_str_vec *messages);


typedef struct cf00_managed_object_data
{
    cf00_managed_object_type m_object_type;
    cf00_reverse_pointer_container_type m_rev_ptr_ctr_type;
    void *m_allocator;
    union
    {
        cf00_void_ptr_array3 m_rev_ptr_array_3;
        cf00_ptr_list m_rev_ptr_list;
        /* tree */
    };
} cf00_managed_object_data;

void cf00_mng_obj_add_rev_ptr(cf00_managed_object_data *obj_data, void *p);
void cf00_mng_obj_remove_rev_ptr(cf00_managed_object_data *obj_data, void *p);
boolean cf00_mng_obj_has_rev_ptr(const cf00_managed_object_data *obj_data,
    const void *p);


struct cf00_data_type;

struct cf00_data_struct;

typedef struct cf00_data_struct_element
{
    struct cf00_data_type *m_data_type;
    cf00_string *m_name;
} cf00_data_struct_element;

typedef struct cf00_data_struct
{
    cf00_managed_object_data m_object_data; /* must be first */
    cf00_data_struct_element *m_data_type_array;
    uint32 m_data_type_array_length;
    uint32 m_data_type_array_capacity;
} cf00_data_struct;




typedef struct cf00_procedure
{
    cf00_managed_object_data m_object_data; /* must be first */

    struct cf00_data_struct *m_data_struct;

    /*
    data
      data type    name
      data type    name
      data type    name
      pointer type
        owning pointer
        non-owning pointer
        shared ownership pointer
          automatic garbage collected pointer
      textual representation
      image representation
      debug output procedure[s]
      text comment[s]



    main procedure = essential to achieve post conditions
    sub-optional procedures    = sub-procedures or instructions interlaced with the main instruction sequence -- purpose is to support assertions or debug output


    preconditions
    postconditions
    intermediate assertions
    textual representation[s]
    image representation[s]
    compiled representation[s]
    
    instruction sequence
      sequence point
      instruction
      sequence point
      instruction
      sequence point
      instruction
      sequence point
      instruction
      sequence point

    

    sequence point
      prev instruction[s]
      next instruction[s]
      checks

    
 
    instruction
      prev sequence point[s]
      next sequence point[s]
      parent procedure[s]
        main parent procedure
        optional parent procedure
      implementation procedure
      map from parent procedure data to implementation procedure data
      instruction type: assertion, nop, ordinary instruction, branch instruction
      if assertion
        required optional procedure
      link to text representation
      link to alternate equivalent instruction
      link to other representation




required procedure = main procedure
optional procedure = 
  required to perform assertion
assertion
  warning
    % confidence
  error



    */
    
} cf00_procedure;

void cf00_proc_init(cf00_procedure *p);
void cf00_proc_clear(cf00_procedure *p);
uint64 cf00_proc_verify_data(const cf00_procedure *p, cf00_str_vec *messages);

typedef struct cf00_time
{
    /*  base time */
    /* enum {base_time_epoch, unix_1970_epoch} m_epoch */
    /* enum { seconds, milliseconds, microseconds, weeks, years } m_time_unit */
    int64 m_ticks;
} cf00_time;


typedef struct cf00_file_data
{
    cf00_managed_object_data m_object_data; /* must be first */
    cf00_string m_file_name;
    cf00_str_vec m_relative_file_path;
    cf00_str_vec m_absolute_file_path;
    uint64 m_file_length;
    uint32 m_adler_32_sum;
    uint8 m_md5_sum[16];
    cf00_time m_modified_time;
    uint8 m_is_this_file;
    
  
} cf00_file_data;


typedef struct cf00_file_sub_section
{
    cf00_managed_object_data m_object_data; /* must be first */
    cf00_file_data *m_file_data;
    uint64 m_section_begin_pos;
    uint64 m_section_end_pos;   
    cf00_string m_first_bytes;
    cf00_string m_last_bytes;
    uint32 m_begin_line_index;
    uint32 m_end_line_index;
    uint16 m_begin_line_offset;
    uint16 m_end_line_offset;
    uint32 m_adler_32_sum;
} cf00_file_sub_section;


typedef enum
{
    CF00_RT_NONE = 0,

    /* attribute (one-way relation, so to speak) */

    /* binary relation */
    CF00_RT_PREV_NEXT,
    CF00_RT_BEFORE_AFTER,

    /* ternary relation */

    /* n-way relation */

    CF00_RT_COUNT
} cf00_relation_type;


typedef struct cf00_unary_relation
{
    cf00_managed_object_data m_object_data; /* must be first */
    cf00_relation_type m_relation_type; /* must be second */
    void *m_object_0;
} cf00_unary_relation;

typedef struct cf00_binary_relation
{
    cf00_managed_object_data m_object_data; /* must be first */
    cf00_relation_type m_relation_type; /* must be second */
    void *m_object_0;
    void *m_object_1;
} cf00_binary_relation;


typedef struct cf00_ternary_relation
{
    cf00_managed_object_data m_object_data; /* must be first */
    cf00_relation_type m_relation_type; /* must be second */

    void *m_object_0;
    void *m_object_1;
    void *m_object_2;

} cf00_ternary_relation;


/* cf00_xyz & cf00_xyzabc & cf00_xyzabc_allocator: template code */

/* unmanaged type */
typedef struct cf00_xyz
{
    int m_x;
    size_t m_y;
    double m_z;
} cf00_xyz;

struct cf00_xyzabc_allocator;

/* managed type */
typedef struct cf00_xyzabc
{
    cf00_managed_object_data m_object_data; /* must be first */
    cf00_xyz *m_xyz_array;
    uint32 m_xyz_array_length;
    uint32 m_xyz_array_capacity;
} cf00_xyzabc;

void cf00_xyzabc_init(cf00_xyzabc *x);
void cf00_xyzabc_clear(cf00_xyzabc *x);
int cf00_xyzabc_compare(const cf00_xyzabc *x, const cf00_xyzabc *y);
void cf00_xyzabc_resize_xyz_array(cf00_xyzabc *x, const uint32 new_sz);
void cf00_xyzabc_reserve_xyz_array(cf00_xyzabc *x, const uint32 new_cap);
void cf00_xyzabc_assign(cf00_xyzabc *dest, const cf00_xyzabc *src);
uint64 cf00_xyzabc_verify_data(const cf00_xyzabc *x, cf00_string *err_msg);


typedef struct cf00_xyzabc_allocator
{
    struct cf00_alloc_block_a *m_alloc_block_chain;
    cf00_xyz *m_free_chain_xyz_buf_16;
    cf00_xyz *m_free_chain_xyz_buf_32;
    cf00_xyz *m_free_chain_xyz_buf_64;
    cf00_xyzabc *m_free_chain_xyzabc;
} cf00_xyzabc_allocator;

#if 0
static void cf00_xyzabc_allocate_block(cf00_xyzabc_allocator *a);
static xyz *cf00_allocate_xyz_buf(cf00_xyzabc_allocator *a, 
    const uint32 capacity);
static void cf00_free_xyz_buf(cf00_xyzabc_allocator *a, xyz *buf, 
    const uint32 capacity);
#endif

void cf00_xyzabc_alloc_init(cf00_xyzabc_allocator *a);
void cf00_xyzabc_alloc_clear(cf00_xyzabc_allocator *a);
cf00_xyzabc *cf00_allocate_xyzabc(cf00_xyzabc_allocator *a);
void cf00_free_xyzabc(struct cf00_xyzabc *s);

void cf00_xyzabc_alloc_debug_dump(cf00_xyzabc_allocator *a);
uint64 cf00_xyzabc_alloc_verify_data(const cf00_xyzabc_allocator *a,
    cf00_string *err_msg);



#endif






