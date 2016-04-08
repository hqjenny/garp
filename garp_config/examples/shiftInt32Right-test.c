
#include <stdio.h>

void loadConfig_shiftInt32Right( void );
int shiftInt32Right( int, int );

typedef unsigned int bits32;

#define NUMITERS 10000

bits32 config_shiftInt32Right[] =
#include "shiftInt32Right.config"
;

main()
{
    int a, b, zTest, zTrue, iOuter, i;

    loadConfig_shiftInt32Right();
    for ( iOuter = 0; iOuter < NUMITERS; iOuter += 100 ) {
        printf( "Iteration %5d\n", iOuter );
        for ( i = 0; i < 100; ++i ) {
            a = ( rand()<<16 ) ^ rand();
            b = ( rand()<<16 ) ^ rand();
            zTest = shiftInt32Right( a, b );
            zTrue = a>>b;
            if ( zTest != zTrue ) {
                printf( "%08X %08X %08X %08X\n", a, b, zTrue, zTest );
            }
        }
    }
    return 0;

}

