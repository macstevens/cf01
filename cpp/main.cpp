#include <assert.h>
#include <stdio.h>
#include <map>
#include <vector>

//#include <stdlib.h>
//#include <stdio.h>


//#define CF_DEBUG_ASSERT( _condition_ ) assert(_condition)
#define CF_DEBUG_ASSERT( _condition_ ) {if(!(_condition)) throw -1; }
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
        cf01_void_ptr_array3 m_rev_ptr_array_3;
        cf01_ptr_list m_rev_ptr_list;
        /* tree */
    };
} cf01_managed_object_data;

void cf01_mng_obj_add_rev_ptr(cf01_managed_object_data *obj_data, void *p);
void cf01_mng_obj_remove_rev_ptr(cf01_managed_object_data *obj_data, void *p);
boolean cf01_mng_obj_has_rev_ptr(const cf01_managed_object_data *obj_data,
    const void *p);

void cf01_write_mng_obj(cf01_writer *w, cf01_managed_object_data *obj_data)
{
    w->write_open_tag("obj_data");
    w->write_line_break();
    w->write_open_tag("obj_type");
    w->write_string(/*m_object_type to string */);
    w->write_close_tag("obj_type");
    w->write_line_break();

    /* ... */
 
    w->write_close_tag("obj_data");


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
       CF_DEBUG_ASSERT(bool_get_double_check() == result);
       return result;
    }
    bool bool_get_double_check(const uint64& i) const {
        enum Result { R_FALSE, R_TRUE, R_UNKNOWN };
        Result result = R_UNKNOWN;
        std::map<uint64,uint64>::const_iterator map_itr =
           m_lower_bound_modulus_map.begin();
        for (; map_itr!=m_lower_bound_modulus_map.end() && R_UNKNOWN == result; 
             ++map_itr) {
            if (map_itr->first <= i) {
                if (m_lower_bound_modulus_map.begin() == map_itr) {
                    result = R_FALSE;                
                }
                else {
                    --map_itr;
                    const uint64& m = map_itr->second;
                    result = ((m < 2) || (((i)%m)==m-1)) ? R_TRUE : R_FALSE; 
                }
            }
        return (R_TRUE == result);
        }
private:
    std::map<uint64, uint64> m_lower_bound_modulus_map;
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

