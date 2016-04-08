
#include <stdio.h>

void loadConfig_shiftUint32Right( void );
int shiftUint32Right( int, int );

typedef unsigned int bits32;

#define NUMITERS 10000

bits32 config_shiftUint32Right[] =
#include "shiftUint32Right.config"
;

main()
{
    unsigned int a, zTest, zTrue;
    int b, iOuter, i;

    loadConfig_shiftUint32Right();
    for ( iOuter = 0; iOuter < NUMITERS; iOuter += 100 ) {
        printf( "Iteration %5d\n", iOuter );
        for ( i = 0; i < 100; ++i ) {
            a = ( rand()<<16 ) ^ rand();
            b = ( rand()<<16 ) ^ rand();
            zTest = shiftUint32Right( a, b );
            zTrue = a>>b;
            if ( zTest != zTrue ) {
                printf( "%08X %08X %08X %08X\n", a, b, zTrue, zTest );
            }
        }
    }
    return 0;

}

