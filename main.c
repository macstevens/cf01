#include <stdlib.h>
#include <stdio.h>
#include "cf.h"


int main(int argc, char *argv[])
{
    int i;
    printf("argc = %i\n", argc);
    for (i=0; i<argc; ++i){
        printf("  argv[%i]=%s\n", i, argv[i]);
    }
    return EXIT_SUCCESS;
}
