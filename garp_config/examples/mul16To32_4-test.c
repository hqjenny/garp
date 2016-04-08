
#include <stdlib.h>
#include <stdio.h>

void loadConfig_mul16To32_4( void );
unsigned int mul16To32_4( int, int );

typedef unsigned int bits32;

#define NUMITERS 10000

bits32 config_mul16To32_4[] =
#include "mul16To32_4.config"
;

main()
{
    unsigned int a, b, zTest, zTrue;
    int iOuter, i;

    loadConfig_mul16To32_4();
    for ( iOuter = 0; iOuter < NUMITERS; iOuter += 100 ) {
        printf( "Iteration %5d\n", iOuter );
        for ( i = 0; i < 100; ++i ) {
            a = ( ( rand()<<15 ) | ( rand() & 0x7FFF ) ) & 0xFFFF;
            b = ( ( rand()<<15 ) | ( rand() & 0x7FFF ) ) & 0xFFFF;
            zTest = mul16To32_4( a, b );
            zTrue = a * b;
            if ( zTest != zTrue ) {
                printf( "%08X %08X %08X %08X\n", a, b, zTrue, zTest );
            }
        }
    }

}

