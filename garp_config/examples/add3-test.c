
#include <stdlib.h>
#include <stdio.h>
#include "add3.h"

typedef unsigned int bits32;

#define NUMITERS 10000

bits32 add3Config[] =
#include "add3.config"
;

main()
{
    int a, b, c, zTest, zTrue, iOuter, i;

    loadAdd3Config();
    for ( iOuter = 0; iOuter < NUMITERS; iOuter += 100 ) {
        printf( "Iteration %5d\n", iOuter );
        for ( i = 0; i < 100; ++i ) {
            a = rand();
            b = rand();
            c = rand();
            zTest = add3( a, b, c );
            zTrue = a + b + c;
            if ( zTest != zTrue ) {
                printf( "%08X %08X %08X %08X %08X\n", a, b, c, zTrue, zTest );
            }
        }
    }

}

