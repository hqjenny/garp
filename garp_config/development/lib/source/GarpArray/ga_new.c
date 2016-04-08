
#include "platform.h"
#include "GarpArray.h"
#include "allocate.h"
#include "array.h"

GarpArrayT * ga_new( void )
{
    GarpArrayT *GarpArrayPtr;

    GarpArrayPtr = allocType( GarpArrayT );
    GarpArrayPtr->numRows = 0;
    GarpArrayPtr->configPtr = 0;
    GarpArrayPtr->phase0StatePtr = 0;
    GarpArrayPtr->phase1StatePtr = 0;
    GarpArrayPtr->clock = 0;
    return GarpArrayPtr;

}

