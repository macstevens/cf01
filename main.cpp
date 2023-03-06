#include <iostream>

#include <stdlib.h>
#include <cstring>
#include <string>
#include <stdio.h>
#include "cf01.h"


int main(int argc, char** argv)
{
std::string msg;
msg = "CF01 Test Program\n";
printf( "[%i]%s", argc, msg.c_str() );
for( int i = 0; i < argc; ++i )
    {
    printf( "argv[%i] = %s\n" , i, argv[i] );
    }

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
return 0;
}

