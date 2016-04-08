
#include <stdlib.h>
#include <stdio.h>
#include "writestats.h"
#include "addInt32Vector.h"

typedef unsigned int bits32;

enum {
    numElements = 4
};
int testVector[ numElements ] = {
    0x00000001,
    0x00000012,
    0x00000123,
    0x00001234
};

bits32 config_addInt32Vector[] =
#include "addInt32Vector.config"
;

main()
{
    int z;

    printf( "address: %p\n", (void *) testVector );
    writestats();
    z = addInt32Vector( testVector, numElements );
    writestats();
    printf( "answer: %08X\n", z );
    return EXIT_SUCCESS;

}

