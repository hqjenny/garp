
#include <stdlib.h>
#include <stdio.h>
#include "sub.h"

typedef unsigned int bits32;

#define NUMITERS 10000

bits32 subConfig[] =
#include "sub.config"
;

main()
{
    int a, b, zTest, zTrue, iOuter, i;

    loadSubConfig();
    for ( iOuter = 0; iOuter < NUMITERS; iOuter += 100 ) {
        printf( "Iteration %5d\n", iOuter );
        for ( i = 0; i < 100; ++i ) {
            a = rand();
            b = rand();
            zTest = sub( a, b );
            zTrue = a - b;
            if ( zTest != zTrue ) {
                printf( "%08X %08X %08X %08X\n", a, b, zTrue, zTest );
            }
        }
    }

}

