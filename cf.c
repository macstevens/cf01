/** cf.c

This file is C source code for parsing and manipulating a C-flat program
*/

#include <stdio.h>

typedef void *void_ptr;
typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;
typedef long long int64;
typedef unsigned long long uint64;



typedef enum
{
    CF00_OT_OBJECT = 0,
    CF00_OT_PROCEDURE,

    CF00_OT_COUNT
} cf00_object_type;

typedef struct
{
    uint32 m_length;
    char *m_buffer;
} cf00_string;

typedef struct
{
    uint32 m_length;
    cf00_string *m_buffer;
} cf00_str_vec;



typedef struct
{
    void *m_cf00_object;
    struct cf00_object_list *m_prev;
    struct cf00_object_list *m_next;
} cf00_object_list;

typedef struct 
{
    cf00_object_type m_object_type;
    cf00_object_list m_reverse_pointer_list;
    /*  pointer to free-chain owner */
    /*  pointer to free-chain next */
} cf00_object;



typedef struct
{
    cf00_object m_object_data; /* must be first */

    
    
} cf00_procedure;



typedef struct
{
    /*  base time */
    /* enum {base_time_epoch, unix_1970_epoch} m_epoch */
    /* enum { seconds, milliseconds, microseconds, weeks, years } m_time_unit */
    int64 m_ticks;
} cf00_time;


typedef struct
{
    cf00_object m_object_data; /* must be first */
    cf00_string m_file_name;
    cf00_str_vec m_relative_file_path;
    cf00_str_vec m_absolute_file_path;
    uint64 m_file_length;
    uint32 m_adler_32_sum;
    uint8 m_md5_sum[16];
    cf00_time m_modified_time;
    uint8 m_is_this_file;
    
  
} cf00_file_data;

typedef struct
{
    cf00_object m_object_data; /* must be first */
    cf00_file_data *m_file_data;
    uint64 m_section_begin_pos;
    uint64 m_section_end_pos;   
  
} cf00_file_sub_section_a;


typedef struct
{
    cf00_object m_object_data; /* must be first */
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
} cf00_file_sub_section_b;


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


typedef struct
{
    cf00_object m_object_data; /* must be first */
    cf00_relation_type m_relation_type; /* must be second */
    void *m_object_0;
} cf00_unary_relation;

typedef struct
{
    cf00_object m_object_data; /* must be first */
    cf00_relation_type m_relation_type; /* must be second */
    void *m_object_0;
    void *m_object_1;
} cf00_binary_relation;


typedef struct
{
    cf00_object m_object_data; /* must be first */
    cf00_relation_type m_relation_type; /* must be second */

    void *m_object_0;
    void *m_object_1;
    void *m_object_2;

} cf00_ternary_relation;









