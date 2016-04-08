
#include <stdlib.h>
#include <stdio.h>

void loadConfig_reverse( void );
unsigned int reverse( unsigned int );

typedef unsigned int bits32;

#define NUMITERS 10000

bits32 config_reverse[] =
#include "reverse.config"
;

static unsigned int reverse_local( unsigned int a )
{
    unsigned int z;
    int i;

    z = 0;
    for ( i = 31; 0 <= i; --i ) {
        z = ( z<<1 ) | ( a & 1 );
        a >>= 1;
    }
    return z;

}

main()
{
    unsigned int a, zTest, zTrue;
    int iOuter, i;

    loadConfig_reverse();
    for ( iOuter = 0; iOuter < NUMITERS; iOuter += 100 ) {
        printf( "Iteration %5d\n", iOuter );
        for ( i = 0; i < 100; ++i ) {
            a = ( ( lrand48() & 0xFFFF )<<16 ) ^ lrand48();
            zTest = reverse( a );
            zTrue = reverse_local( a );
            if ( zTest != zTrue ) {
                printf( "%08X %08X %08X\n", a, zTrue, zTest );
            }
        }
    }
    return EXIT_SUCCESS;

}

