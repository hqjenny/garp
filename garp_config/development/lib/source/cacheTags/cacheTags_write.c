
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "cacheTags.h"
#include "cacheTagsX.h"

cacheTags_outT
 cacheTags_write(
     cacheTagsT *cacheTagsPtr,
     uint_fast32_t clockCycle,
     uint_fast32_t address,
     int size
 )
{
    int_fast32_t i;
    cacheTags_outT z;
    int_fast32_t tagNum;
    cacheTagT *tagPtr;
    int prefetchBufferNum, busy;

    i = cacheTags_z_serviceIncomingPrefetches( cacheTagsPtr, clockCycle );
    clockCycle += i;
    z.stall = i;
    z.latency = 0;
    z.needWrite = false;
    z.needRead = false;
    address >>= cacheTagsPtr->logLineSize;
    tagNum = address & (cacheTagsPtr->numTags - 1);
    tagPtr = &cacheTagsPtr->tags[tagNum];
    prefetchBufferNum = tagPtr->prefetchBufferNum;
    if ( 0 <= prefetchBufferNum ) {
        cacheTagsPtr->prefetchBuffers[prefetchBufferNum].clockCycle = 0;
        tagPtr->prefetchBufferNum = -1;
    }
    if ( tagPtr->address != address ) {
        busy = 0;
        if ( tagPtr->dirty ) {
            busy += cacheTagsPtr->writeBackBusy;
            z.needWrite = true;
            z.writeAddress = (tagPtr->address)<<(cacheTagsPtr->logLineSize);
        }
        if ( size < cacheTagsPtr->lineSize ) {
            busy += cacheTagsPtr->writeMissBusy;
            z.stall += cacheTagsPtr->writeMissPenalty;
            z.needRead = true;
        }
        if ( z.needWrite | z.needRead ) {
            i = cacheTagsPtr->lastBusyClockCycle - clockCycle;
            if ( 0 < i ) {
                busy += i;
                z.stall += i;
            }
        }
        clockCycle += busy;
        tagPtr->address = address;
    }
    tagPtr->dirty = true;
    cacheTagsPtr->lastBusyClockCycle = clockCycle;
    return z;

}

