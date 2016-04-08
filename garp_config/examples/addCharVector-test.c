
#include <stdlib.h>
#include <stdio.h>
#include "../GarpArray/GarpArray.h"
#include "addCharVector.h"

typedef unsigned int bits32;

enum {
    numElements = 4
};
char testVector[ numElements + 1 ] = {
    0,
    0x01,
    0x12,
    0x23,
    0x34
};

bits32 config_addCharVector[] =
#include "addCharVector.config"
;
ga_queueControlT queue0_addCharVector;

main()
{
    char z;

    queue0_addCharVector.enable = 1;
    queue0_addCharVector.direction = ga_read;
    queue0_addCharVector.attributes = 0;
    queue0_addCharVector.size = ga_1;
    queue0_addCharVector.count = ga_1;
    queue0_addCharVector.address = (bits32) ( testVector + 1 );
    queue0_addCharVector.limitAddress = 0;
    queue0_addCharVector.busNums[ 0 ] = 2;

    printf( "address: %p\n", (void *) testVector + 1 );
    z = addCharVector( numElements );
    printf( "answer: %08X\n", z );
    z = addCharVector( numElements );
    printf( "answer: %08X\n", z );
    return EXIT_SUCCESS;

}

