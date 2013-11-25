#include <assert.h>
#include <stdio.h>
#include <map>
#include <vector>

//#include <stdlib.h>
//#include <stdio.h>


//#define CF_DEBUG_ASSERT( _condition_ ) assert(_condition)
#define CF_DEBUG_ASSERT( _condition_ ) {if(!(_condition)) throw -1; }
#define CF_ASSERT( _condition_ ) assert(_condition_)

typedef unsigned long long int uint64;



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

