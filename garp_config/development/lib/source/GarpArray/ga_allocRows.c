
#include <stdbool.h>
#include <stdlib.h>
#include "platform.h"
#include "GarpArray.h"
#include "allocate.h"
#include "array.h"

void ga_allocRows( GarpArrayT *GarpArrayPtr, int numRows )
{
    int queueNum, readBufferNum;

    if ( ga_maxNumRows <= numRows ) numRows = ga_maxNumRows;
    if ( GarpArrayPtr->configPtr ) {
        free( GarpArrayPtr->configPtr );
        GarpArrayPtr->configPtr = 0;
    }
    if ( GarpArrayPtr->phase0StatePtr ) free( GarpArrayPtr->phase0StatePtr );
    if ( GarpArrayPtr->phase1StatePtr ) free( GarpArrayPtr->phase1StatePtr );
    if ( numRows ) {
        GarpArrayPtr->phase0StatePtr = allocArrayType( rowStateT, numRows );
        GarpArrayPtr->phase1StatePtr = allocArrayType( rowStateT, numRows );
    } else {
        GarpArrayPtr->phase0StatePtr = 0;
        GarpArrayPtr->phase1StatePtr = 0;
    }
    GarpArrayPtr->numRows = numRows;
    GarpArrayPtr->branchFlag = false;
    GarpArrayPtr->interruptFlag = false;
    GarpArrayPtr->memoryRequest.enable = false;
    for ( queueNum = 0; queueNum < numQueues; ++queueNum ) {
        GarpArrayPtr->queues[queueNum].control.enable = false;
    }
    for (
        readBufferNum = 0; readBufferNum < numReadBuffers; ++readBufferNum
    ) {
        GarpArrayPtr->readBuffers[readBufferNum].enable = false;
    }
    GarpArrayPtr->readStall = 0;

}

