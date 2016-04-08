
#include <stdlib.h>
#include <stdio.h>
#include "writestats.h"
#include "add.h"

typedef unsigned int bits32;

#define NUMITERS 10000

bits32 addConfig[] =
#include "add.config"
;

main()
{
    int a, b, zTest, zTrue, iOuter, i;

    loadAddConfig();
    for ( iOuter = 0; iOuter < NUMITERS; iOuter += 100 ) {
        printf( "Iteration %5d\n", iOuter );
        writestats();
        for ( i = 0; i < 100; ++i ) {
            a = ( rand()<<16 ) ^ rand();
            b = ( rand()<<16 ) ^ rand();
            zTest = add( a, b );
            zTrue = a + b;
            if ( zTest != zTrue ) {
                printf( "%08X %08X %08X %08X\n", a, b, zTrue, zTest );
            }
        }
    }
    writestats();
    return EXIT_SUCCESS;

}

