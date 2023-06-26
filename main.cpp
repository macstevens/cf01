#include <iostream>

#include <stdlib.h>
#include <cstring>
#include <string>
#include <stdio.h>
#include "cf01.h"


void auto_assert_test_01(){
CF01_AA_INCR_CALL_DEPTH();
CF01_AUTO_ASSERT( true );
for (int a = 0; a < 100; ++a) {
    CF01_AA_INCR_CALL_DEPTH();
    CF01_AUTO_ASSERT( true );
    for (int b = 0; b < 100; ++b) {
        CF01_AA_INCR_CALL_DEPTH();
        CF01_AUTO_ASSERT( true );
        for (int c = 0; c < 100; ++c) {
            CF01_AA_INCR_CALL_DEPTH();
            sprintf(CF01_AA_ERR_BUF(),"a=%i  b=%i  c=%i",a,b,c);
            *(CF01_AA_ERR_BUF_POS_PTR()) = strlen(CF01_AA_ERR_BUF());
            CF01_AUTO_ASSERT( ((a+b+c) % 17 < 3 ) || (a + b + c) < 200);
            CF01_AA_DECR_CALL_DEPTH();
            }
        CF01_AA_DECR_CALL_DEPTH();
        }
    CF01_AA_DECR_CALL_DEPTH();
    }
CF01_AA_DECR_CALL_DEPTH();
}


void hash_consistency_check_test_01(){
int aa[3];
aa[0] = 0;
aa[1] = 0;
aa[2] = 0;
static const size_t ii_fail_idx = 1234;
static const int ii_limit = 10000;
CF01_AA_INCR_CALL_DEPTH();
for( size_t ii = 0; ii < ii_limit; ++ii ) {
    CF01_AA_INCR_CALL_DEPTH();
    ++(aa[0]);
    const bool fail = ( ii == ii_fail_idx ) ? true : false;
    if( fail ){
        /* non-reproducible change of state */
        (aa[2]) += static_cast<int>( time(NULL) % 73459 );
        }
    CF01_HASH_CONSISTENCY_CHECK( cf01_auto_assert_wksp::default_hash( 0,
        reinterpret_cast<const cf01_uint8 *>(aa), sizeof(aa) ) );
    CF01_AA_DECR_CALL_DEPTH();
    }
CF01_AA_DECR_CALL_DEPTH();
}


void hash_consistency_check_test_02(){
int aa[3];
aa[0] = 0;
aa[1] = 0;
aa[2] = 0;
static const size_t ii_fail_idx = 12;
static const size_t jj_fail_idx = 23;
static const size_t kk_fail_idx = 34;
static const int ijk_limit = 100;
CF01_AA_INCR_CALL_DEPTH();
for( size_t ii = 0; ii < ijk_limit; ++ii ) {
    ++(aa[0]);
    CF01_AA_INCR_CALL_DEPTH();
    for( size_t jj = 0; jj < ijk_limit; ++jj ) {
        ++(aa[1]);
        CF01_AA_INCR_CALL_DEPTH();
        for( size_t kk = 0; kk < ijk_limit; ++kk ) {
            CF01_AA_INCR_CALL_DEPTH();
            ++(aa[2]);
            const bool fail = ( ( ii == ii_fail_idx ) &&
                                ( jj == jj_fail_idx ) &&
                                ( kk == kk_fail_idx ) ) ? true : false;
            if( fail ){
                /* non-reproducible change of state */
                (aa[2]) += static_cast<int>( time(NULL) % 73459 );
                }
            CF01_HASH_CONSISTENCY_CHECK( cf01_auto_assert_wksp::default_hash( 0,
                reinterpret_cast<const cf01_uint8 *>(aa), sizeof(aa) ) );
            CF01_AA_DECR_CALL_DEPTH();
            }
        CF01_HASH_CONSISTENCY_CHECK( cf01_auto_assert_wksp::default_hash( 0,
            reinterpret_cast<const cf01_uint8 *>(aa), sizeof(aa) ) );
        CF01_AA_DECR_CALL_DEPTH();
        }
    CF01_HASH_CONSISTENCY_CHECK( cf01_auto_assert_wksp::default_hash( 0,
        reinterpret_cast<const cf01_uint8 *>(aa), sizeof(aa) ) );
    CF01_AA_DECR_CALL_DEPTH();
    }
CF01_AA_DECR_CALL_DEPTH();
}


void command_test(int argc, char** argv){
if( argc > 1 )
    {
    std::string cmd;
    for( int j = 0; j < (argc-1); ++j )
        {
        cmd += argv[j];
        cmd += " ";
        }
    system ( cmd.c_str() );
    }
printf( "[%i] done\n", argc );
}

int main(int argc, char** argv)
{
std::string msg;
msg = "CF01 Test Program\n";
printf( "[%i]%s", argc, msg.c_str() );
for( int i = 0; i < argc; ++i )
    {
    printf( "argv[%i] = %s\n" , i, argv[i] );
    }

static const int test_choice = 0;
switch( test_choice ) {
    case 0:
        CF01_SET_JRNL_WRITE_MODE_ON_ERROR();
        auto_assert_test_01();
        break;
    case 1:
        CF01_SET_JRNL_WRITE_MODE_ON();       
        hash_consistency_check_test_01();    
        break;
    case 2:
    default:
        CF01_SET_JRNL_WRITE_MODE_ON();     
        hash_consistency_check_test_02();  
        break;
    case 3:
    default:
        command_test(argc, argv);
        break;
    }
return 0;
}


