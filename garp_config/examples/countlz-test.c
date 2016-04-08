
#include <stdlib.h>
#include <stdio.h>
#include "countlz.h"

typedef unsigned int bits32;

#define NUMITERS 10000

bits32 countlzConfig[] =
#include "countlz.config"
;

static int countlz_local( int a )
{
    int z;

    z = 0;
    while ( 0 <= a ) {
        ++z;
        a <<= 1;
    }
    return z;

}

main()
{
    int a, b, c, zTest, zTrue, iOuter, i;

    loadCountlzConfig();
    for ( iOuter = 0; iOuter < NUMITERS; iOuter += 100 ) {
        printf( "Iteration %5d\n", iOuter );
        for ( i = 0; i < 100; ++i ) {
            a = 1<<( rand() & 31 );
            b = ( 1<<( rand() & 31 ) ) - 1;
            c = a + ( rand() & b );
            zTest = countlz( c );
            zTrue = countlz_local( c );
            if ( zTest != zTrue ) {
                printf( "%08X %08X %08X\n", c, zTrue, zTest );
            }
        }
    }

}

