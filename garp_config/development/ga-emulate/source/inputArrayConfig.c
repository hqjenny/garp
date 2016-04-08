
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "platform.h"
#include "GarpArrayConfig.h"
#include "GarpArray.h"
#include "inputArrayConfig.h"

void inputArrayConfig( FILE *streamPtr, GarpArrayT *GarpArrayPtr )
{
    uint32_t *arrayConfigPtr;
    int numRows;

    arrayConfigPtr = GarpArrayConfig_read( streamPtr );
    numRows = *arrayConfigPtr;
    ga_allocRows( GarpArrayPtr, numRows );
    ga_zero( GarpArrayPtr );
    ga_loadConfig( GarpArrayPtr, 0, numRows, arrayConfigPtr + 1 );
    free( arrayConfigPtr );
    printf( "Configuration of %d rows loaded.\n", numRows );

}

