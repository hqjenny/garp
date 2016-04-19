#include <stdlib.h>
#include <stdio.h>
#include "add3.h"

typedef unsigned int bits32;

#define Z 0
#define D 1
#define NUMITERS 10000

//static int funct = 2;
// R = 0 -> Z; R = 1 -> D
void mtga (bits32 rt, bits32 row_num, bits32 R, bits32 clock_count){ 
	bits32 funct = 4 | R;
	
        asm volatile ("fence");
        asm volatile ("custom0 %[rd], %[rs1], %[rs2], %[funct]" : [rd]"=r"(clock_count) :  [rs1]"r"(rt), [rs2]"i"(row_num), [funct]"i"(funct));
        asm volatile ("fence");
}

void gaconf(unsigned int * rt){
        asm volatile ("fence");
        asm volatile ("custom0 0, %[rs1], 0, 0" ::  [rs1]"r"(rt));
        asm volatile ("fence");
}

int add3(int a, int b, int c){
	mtga(a, 0, Z, 0);
	mtga(b, 1, Z, 0);
	mtga(c, 1, D, 1);

        unsigned int rd = 0;

	// mfga
        return rd;
}

bits32 add3Config[] =
#include "add3.config"
;

void loadAdd3Config(){
	gaconf(add3Config);
}


int main()
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
	return 0;
}

