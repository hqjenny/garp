
#include <stdlib.h>
#include <stdio.h>
#include "add3sub.h"

typedef unsigned int bits32;

#define NUMITERS 10000

bits32 config_add3sub[] =
#include "add3sub.config"
;

main()
{
    int a, b, c, zTest, zTrue, iOuter, i;

    loadConfig_add3sub();
    for ( iOuter = 0; iOuter < NUMITERS; iOuter += 100 ) {
        printf( "Iteration %5d\n", iOuter );
        for ( i = 0; i < 100; ++i ) {
            a = rand();
            b = rand();
            c = rand();
            zTest = add3sub( a, b, c );
            zTrue = a + b - c;
            if ( zTest != zTrue ) {
                printf( "%08X %08X %08X %08X %08X\n", a, b, c, zTrue, zTest );
            }
        }
    }

}

