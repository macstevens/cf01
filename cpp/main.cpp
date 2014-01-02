#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
    #include <list>
    #include <map>
    #include <stack>
    #include <vector>
#endif


#include "../cf.h"
#include "../cf_basic_test_suite.h"
#include "../cf_unit_test.h"

typedef cf00_string cf01_string;
typedef cf00_str_vec cf01_str_vec;


//#define CF_DEBUG_ASSERT( _condition_ ) assert(_condition_)
#define CF_DEBUG_ASSERT( _condition_ ) {if(!(_condition_)) throw -1; }
#define CF_ASSERT( _condition_ ) assert(_condition_)


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


typedef enum
{
    CF01_OT_OBJECT = 0,
    CF01_OT_PROCEDURE,

    CF01_OT_XYZABC,     /* template code */

    CF01_OT_COUNT
} cf01_managed_object_type;

typedef enum
{
    CF01_RPCT_NONE = 0,
    CF01_RPCT_ARRAY_3,
    CF01_RPCT_LIST,
    CF01_RPCT_TREE,

    CF01_RPCT_COUNT
} cf01_reverse_pointer_container_type;




typedef void *cf01_void_ptr_array3[3];


typedef struct cf01_ptr_list_node
{
    void *m_ptr;
    struct cf01_ptr_list_node *m_prev;
    struct cf01_ptr_list_node *m_next;
} cf01_ptr_list_node;

typedef struct cf01_ptr_list
{
    void *m_cf01_ptr_list_node_allocator;
    cf01_ptr_list_node *m_head;
    uint32 m_size;
} cf01_ptr_list;

void cf01_ptr_list_init(cf01_ptr_list *p);
void cf01_ptr_list_clear(cf01_ptr_list *p);
void cf01_ptr_list_push_back(cf01_ptr_list *obj_data, void *p);
uint64 cf01_ptr_list_verify_data(const cf01_ptr_list *pl, 
    cf01_str_vec *messages);


typedef struct cf01_managed_object_data
{
    cf01_managed_object_type m_object_type;
    cf01_reverse_pointer_container_type m_rev_ptr_ctr_type;
    void *m_allocator;
    union
    {
        cf01_void_ptr_array3 m_rev_ptr_array_3; /* can contain duplicates */
        cf01_ptr_list m_rev_ptr_list;           /* can contain duplicates */
        /* multimap + list 
        std::list<void *> m_ptr_list;
        std::multimap<void *, std::list<void *>::iterator> m_ptr_list_lookup_mmap;
        void insert_ptr(p) {
            search in lookup map
                found: insert after last same item in list
                not found: insert at end of list
        */

/*
basic operations
  save
  load
  compare
  copy
  hash
  hash_depth_1
  hash_depth_2
  hash_depth_3

*/
    };
} cf01_managed_object_data;

void cf01_mng_obj_add_rev_ptr(cf01_managed_object_data *obj_data, void *p);
void cf01_mng_obj_remove_rev_ptr(cf01_managed_object_data *obj_data, void *p);
boolean cf01_mng_obj_has_rev_ptr(const cf01_managed_object_data *obj_data,
    const void *p);


typedef struct cf01_save_obj_ptr_list
{
#ifdef __cplusplus
    typedef std::list<const void *> list_t;
    typedef list_t::iterator list_itr_t;
    typedef list_t::const_iterator list_citr_t;
    list_t m_ptr_list;
#else
    /* not yet implemented in C */
#endif

} cf01_save_obj_ptr_list;


int cf01_save_file_fputc(void *output_stream, const uint8 ch)
{
const int fputc_result = fputc((int)ch, (FILE *)output_stream);
return (fputc_result == (int)ch) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* Thomas Wang's 64-bit integer hash 
http://naml.us/blog/tag/thomas-wang
https://gist.github.com/badboy/6267743 hash64shift()
*/
uint64 cf01_void_ptr_wang_hash_64(const void *p)
{
    uint64 k = (uint64)p;
    k = (~k) + (k << 21); // k = (k << 21) - k - 1;
    k = k ^ (k >> 24);
    k = (k + (k << 3)) + (k << 8); // k * 265
    k = k ^ (k >> 14);
    k = (k + (k << 2)) + (k << 4); // k * 21
    k = k ^ (k >> 28);
    k = k + (k << 31);
    return k;
}

typedef int (* cf01_save_putc_func)(void *, const uint8);

typedef uint64 cf01_object_handle;

const cf01_object_handle cf01_object_handle_not_found=(cf01_object_handle)(-1);

typedef struct cf01_void_ptr_obj_handle_map
{
#ifdef __cplusplus
    typedef std::pair<uint64, const void *> key_t;
    typedef std::pair<key_t, cf01_object_handle> kv_t;
    typedef std::map<key_t, cf01_object_handle> map_t;
    typedef map_t::iterator map_itr_t;
    typedef map_t::const_iterator map_citr_t;
    map_t m_ptr_hndl_map;
#else
    /* not yet implemented in C */
#endif
} cf01_void_ptr_obj_handle_map;


void cf01_void_ptr_obj_handle_map_clear(cf01_void_ptr_obj_handle_map *m)
{
    if (NULL != m)
    {
    (m->m_ptr_hndl_map).clear();
    }
}

/*
insert pointer in map, get handle
input:p
output:h
*/
void cf01_void_ptr_obj_handle_map_insert(cf01_void_ptr_obj_handle_map *m,
    const void *p, uint64 *h)
{
    uint64 hndl = 0;
    if (NULL != m)
    {
#ifdef __cplusplus
        cf01_void_ptr_obj_handle_map::key_t key;
        cf01_void_ptr_obj_handle_map::map_itr_t lb;
        key.first = cf01_void_ptr_wang_hash_64(p);
        key.second = p;
        lb = (m->m_ptr_hndl_map).lower_bound(key);
        if (lb->first == key)
        {
            /* found in map */
            hndl = lb->second;
        }
        else
        {
            /* not found, need to insert */
            cf01_void_ptr_obj_handle_map::kv_t kv;
            hndl = (m->m_ptr_hndl_map).size();
            kv.first = key;
            kv.second = hndl;
            (m->m_ptr_hndl_map).insert(lb, kv);            
        }
#else
        /* not yet implemented in C */
#endif
    }
    CF_DEBUG_ASSERT(hndl != cf01_object_handle_not_found);
    if (NULL != h)
    {
        *h = hndl;
    }
}


/* get handle for void * 
return cf01_object_handle_not_found if not found
 */
cf01_object_handle cf01_void_ptr_obj_handle_map_find(const cf01_void_ptr_obj_handle_map *m,
    const void *p)
{
    cf01_object_handle hndl = 0;
    if (NULL != m)
    {
#ifdef __cplusplus
        cf01_void_ptr_obj_handle_map::key_t key;
        cf01_void_ptr_obj_handle_map::map_citr_t search_itr;
        key.first = cf01_void_ptr_wang_hash_64(p);
        key.second = p;
        search_itr = (m->m_ptr_hndl_map).find(key);
        if (search_itr != (m->m_ptr_hndl_map).end())
        {
            hndl = search_itr->second;
        }
        else
        {
            hndl = cf01_object_handle_not_found;
        }
#else
        /* not yet implemented in C */
#endif
    }
    return hndl;
}


typedef struct cf01_save_writer
{
    cf01_save_putc_func m_putc_func;
    void *m_ostream;
    uint16 m_indent_count;
    uint64 m_error_count;
    cf01_void_ptr_obj_handle_map m_ptr_hndl_map;
    cf01_save_obj_ptr_list m_obj_list;
} cf01_save_writer;

void cf01_save_writer_clear(cf01_save_writer *w)
{
    if (NULL != w)
    {
        w->m_putc_func = NULL;
        w->m_ostream = NULL;
        w->m_indent_count = 0;
        w->m_error_count = 0;
        cf01_void_ptr_obj_handle_map_clear(&(w->m_ptr_hndl_map));
#ifdef __cplusplus
        (w->m_obj_list).m_ptr_list.clear();
#else
        /* not yet implemented in C */
#endif
    }
}


void cf01_save_add_obj(cf01_save_writer *w, const void *p)
{
    if (NULL != w && NULL != p)
    {
#ifdef __cplusplus
        (w->m_obj_list).m_ptr_list.push_back(p);
#else
        /* not yet implemented in C */
#endif
    }
}

void cf01_save_log_error(cf01_save_writer *w, const int err_code,
    const char *err_msg)
{
    assert(NULL != w);
    ++(w->m_error_count); 
}

void cf01_save_write_char_buf(cf01_save_writer *w, const char *buf)
{
    const char *ch;
    int putc_result = EXIT_SUCCESS;
    if (NULL != buf & NULL != w && NULL != w->m_putc_func && NULL != w->m_ostream)
    {
        ch = buf;
        while (*ch != 0x0 && EXIT_SUCCESS == putc_result)
        {
            /* TODO: convert control or non-readble characters to escape sequence */
            putc_result = (*(w->m_putc_func))(w->m_ostream, *ch);
            if (EXIT_SUCCESS != putc_result)
                {
                cf01_save_log_error(w, 0, "putc failure");
                }
            ++ch;
        }
    }
}

void cf01_save_indent(cf01_save_writer *w)
{
    if (NULL != w)
    {
        uint16 i;
        for (i=0; i<(w->m_indent_count); ++i)
        {
        cf01_save_write_char_buf(w, "  ");
        }
    }
}

void cf01_save_write_open_tag(cf01_save_writer *w, const char *buf)
{
    if (NULL != buf & NULL != w)
    {
        ++(w->m_indent_count);
        cf01_save_indent(w);
        cf01_save_write_char_buf(w, "<");
        cf01_save_write_char_buf(w, buf);
        cf01_save_write_char_buf(w, ">");
    }
}

void cf01_save_write_close_tag(cf01_save_writer *w, const char *buf)
{
    if (NULL != buf & NULL != w)
    {
        cf01_save_write_char_buf(w, "</");
        cf01_save_write_char_buf(w, buf);
        cf01_save_write_char_buf(w, ">");
        --(w->m_indent_count);
    }
}

void cf01_save_write_newline(cf01_save_writer *w)
{
    if (NULL != w)
    {
        cf01_save_write_char_buf(w, "\n");
    }
}
	
void cf01_save_write_mng_obj(cf01_save_writer *w, const cf01_managed_object_data *obj_data)
{
    if (NULL != w && NULL != obj_data)
    {
        cf01_save_write_open_tag(w, "obj_data");
        cf01_save_write_newline(w);
        cf01_save_write_open_tag(w, "obj_type");
        cf01_save_write_char_buf(w, ""/*m_object_type to string */);
        cf01_save_write_close_tag(w, "obj_type");
        cf01_save_write_newline(w);

        /* ... */
        switch (obj_data->m_rev_ptr_ctr_type)
        {
        case CF01_RPCT_NONE:
            break;
        case CF01_RPCT_ARRAY_3:
            break;
        case CF01_RPCT_LIST:
            break;
        case CF01_RPCT_TREE:
            break;

        case CF01_RPCT_COUNT : // fall through
        default:
            break;          
        }


        cf01_save_write_close_tag(w, "obj_data");
    }


}


typedef struct cf01_const_void_ptr_stack
{
#ifdef __cplusplus
    std::stack<const void*> m_stack;
#else
    /* not yet implemented in C */
#endif
} cf01_const_void_ptr_stack;


const void *cf01_const_void_ptr_stack_pop(cf01_const_void_ptr_stack *s)
{
    const void *t = NULL;
#ifdef __cplusplus
    if (NULL != s && !((s->m_stack).empty()))
        {
        t = (s->m_stack).top();
        (s->m_stack).pop();
        }
#else
    /* not yet implemented in C */
#endif
    return t;
}

void cf01_const_void_ptr_stack_pop(cf01_const_void_ptr_stack *s, const void *p)
{
#ifdef __cplusplus
    if (NULL != s)
        {
        (s->m_stack).push(p);
        }
#else
    /* not yet implemented in C */
#endif
}



void cf01_save_push_procedure_sub_objs(cf01_const_void_ptr_stack *s,
const void *obj)
{
    if (NULL != s && NULL != obj)
    {
        struct cf01_procedure;
        const cf01_procedure *procedure = (const cf01_procedure *)obj;


    }
}


typedef void (*cf01_push_sub_objects_func)(
    cf01_const_void_ptr_stack *s, const void *obj);

static const cf01_push_sub_objects_func cf01_save_push_sub_obj_funcs[] =
{
    NULL,  /* CF01_OT_OBJECT */
    &cf01_save_push_procedure_sub_objs,  /* CF01_OT_PROCEDURE */
    NULL,  /* CF01_OT_XYZABC */
    NULL   /* CF01_OT_COUNT */
};







void cf01_save_procedure_obj(cf01_save_writer *w, const void *obj)
{



}


typedef void (*cf01_save_obj_func)(
    cf01_save_writer *w, const void *obj);

static const cf01_save_obj_func cf01_save_obj_funcs[] =
{
    NULL,  /* CF01_OT_OBJECT */
    &cf01_save_procedure_obj,  /* CF01_OT_PROCEDURE */
    NULL,  /* CF01_OT_XYZABC */
    NULL   /* CF01_OT_COUNT */
};






void cf01_save_file_obj_recursive(const void *obj, FILE *f)
{
    if (NULL != obj && NULL != f)
    {
    struct cf01_save_writer w;
    cf01_save_writer_clear(&w);
    w.m_putc_func = &cf01_save_file_fputc;
    w.m_ostream = f;
    /*
    add object recursive
    sort object list by 1) object type and 2) order of existing object list
    init object handle map
    iterate object list
      write each object
        convert pointers to handles
    */
    }
}



struct cf01_data_type;

struct cf01_data_struct;

typedef struct cf01_data_struct_element
{
    struct cf01_data_type *m_data_type;
    cf01_string *m_name;
} cf01_data_struct_element;

typedef struct cf01_data_struct
{
    cf01_managed_object_data m_object_data; /* must be first */
    cf01_data_struct_element *m_data_type_array;
    uint32 m_data_type_array_length;
    uint32 m_data_type_array_capacity;
} cf01_data_struct;




typedef struct cf01_procedure
{
    cf01_managed_object_data m_object_data; /* must be first */

    struct cf01_data_struct *m_data_struct;

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
    
} cf01_procedure;

void cf01_proc_init(cf01_procedure *p);
void cf01_proc_clear(cf01_procedure *p);
uint64 cf01_proc_verify_data(const cf01_procedure *p, cf01_str_vec *messages);

typedef struct cf01_time
{
    /*  base time */
    /* enum {base_time_epoch, unix_1970_epoch} m_epoch */
    /* enum { seconds, milliseconds, microseconds, weeks, years } m_time_unit */
    int64 m_ticks;
} cf01_time;


typedef struct cf01_file_data
{
    cf01_managed_object_data m_object_data; /* must be first */
    cf01_string m_file_name;
    cf01_str_vec m_relative_file_path;
    cf01_str_vec m_absolute_file_path;
    uint64 m_file_length;
    uint32 m_adler_32_sum;
    uint8 m_md5_sum[16];
    cf01_time m_modified_time;
    uint8 m_is_this_file;
    
  
} cf01_file_data;


typedef struct cf01_file_sub_section
{
    cf01_managed_object_data m_object_data; /* must be first */
    cf01_file_data *m_file_data;
    uint64 m_section_begin_pos;
    uint64 m_section_end_pos;   
    cf01_string m_first_bytes;
    cf01_string m_last_bytes;
    uint32 m_begin_line_index;
    uint32 m_end_line_index;
    uint16 m_begin_line_offset;
    uint16 m_end_line_offset;
    uint32 m_adler_32_sum;
} cf01_file_sub_section;


typedef enum
{
    CF01_RT_NONE = 0,

    /* attribute (one-way relation, so to speak) */

    /* binary relation */
    CF01_RT_PREV_NEXT,
    CF01_RT_BEFORE_AFTER,

    /* ternary relation */

    /* n-way relation */

    CF01_RT_COUNT
} cf01_relation_type;


typedef struct cf01_unary_relation
{
    cf01_managed_object_data m_object_data; /* must be first */
    cf01_relation_type m_relation_type; /* must be second */
    void *m_object_0;
} cf01_unary_relation;

typedef struct cf01_binary_relation
{
    cf01_managed_object_data m_object_data; /* must be first */
    cf01_relation_type m_relation_type; /* must be second */
    void *m_object_0;
    void *m_object_1;
} cf01_binary_relation;


typedef struct cf01_ternary_relation
{
    cf01_managed_object_data m_object_data; /* must be first */
    cf01_relation_type m_relation_type; /* must be second */

    void *m_object_0;
    void *m_object_1;
    void *m_object_2;

} cf01_ternary_relation;





/* 

CF control file
  cf log file path
  



CF log file
 CF control file name, md5 sum
 <assertion number, assertion_result = pass/fail/not run >


*/

/*
INPUT:
0, 5
20, 2
30, 3

MEANING:
 0<=i<20 => result=((i%5)==4)
20<=i<30 => result=((i%2)==1)
30<=i    => result=((i%3)==2)
[ 0] F  [ 1] F  [ 2] F  [ 3] F  [ 4] T  [ 5] F  [ 6] F  [ 7] F  [ 8] F  [ 9] T
[10] F  [11] F  [12] F  [13] F  [14] T  [15] F  [16] F  [17] F  [18] F  [19] T
[20] F  [21] T  [22] F  [23] T  [24] F  [25] T  [26] F  [27] T  [28] F  [29] T
[30] F  [31] F  [32] T  [33] F  [34] F  [35] T  [36] F  [37] F  [38] T  [39] F
[40] F  [41] T  [42] F  [43] F  [44] T  [45] F  [46] F  [47] T  [48] F  [49] F
...

*/
class cf01_modulus_bool_set
{
private:
    std::map<uint64, uint64> m_lower_bound_modulus_map;
public:
    void clear() { m_lower_bound_modulus_map.clear(); }
    void add_lower_bound_modulus(const uint64& lb, const uint64& m)
       { m_lower_bound_modulus_map.insert(std::pair<uint64, uint64>(lb, m)); }
    bool bool_get(const uint64& i) const {
       bool result = false;
       if (!m_lower_bound_modulus_map.empty()) {
           if (m_lower_bound_modulus_map.empty()) {
               result = false;
               }
           else {
               std::map<uint64,uint64>::const_iterator map_lb =
                   m_lower_bound_modulus_map.upper_bound(i);
               if (m_lower_bound_modulus_map.begin() == map_lb) {
                   result = false;
                   }
               else {
                   --map_lb;
                   const uint64& m = map_lb->second;
                   result = (m < 2) || (((i)%m)==m-1);
                   }
               }
           }
       CF_DEBUG_ASSERT(bool_get_double_check(i) == result);
       return result;
    }
    bool bool_get_double_check(const uint64& i) const {
        enum Result { R_FALSE, R_TRUE, R_UNKNOWN };
        Result result = R_UNKNOWN;
        std::map<uint64,uint64>::const_iterator map_itr =
           m_lower_bound_modulus_map.begin();
        for (; R_UNKNOWN == result; ++map_itr) {
            if (map_itr==m_lower_bound_modulus_map.end() || map_itr->first > i){
                if (m_lower_bound_modulus_map.begin() == map_itr) {
                    result = R_FALSE;                
                }
                else {
                    --map_itr;
                    const uint64& m = map_itr->second;
                    result = ((m < 2) || (((i)%m)==m-1)) ? R_TRUE : R_FALSE; 
                }
            }
        }
        return (R_TRUE == result);
    }
};


class cf01_cf_ctrl_data
{


};

int main(int argc, char *argv[])
{
    cf01_modulus_bool_set s;
    s.add_lower_bound_modulus(0, 5);
    s.add_lower_bound_modulus(20, 2);
    s.add_lower_bound_modulus(30, 3);


    printf("MAIN CPP\n");
    for (uint64 i = 0; i < 50; ++i)
    {
        printf("[%2i] %s ", (int)i, s.bool_get(i)?"T":"F");
        if ((i%10) == 9) { printf("\n"); }
    }

    return 0;
}



