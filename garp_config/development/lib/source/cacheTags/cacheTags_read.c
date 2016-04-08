
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "cacheTags.h"
#include "cacheTagsX.h"

cacheTags_outT
 cacheTags_read(
     cacheTagsT *cacheTagsPtr, uint_fast32_t clockCycle, uint_fast32_t address
 )
{
    int_fast32_t i;
    cacheTags_outT z;
    int_fast32_t tagNum;
    cacheTagT *tagPtr;
    int prefetchBufferNum;
    prefetchBufferT *prefetchBufferPtr;

    i = cacheTags_z_serviceIncomingPrefetches( cacheTagsPtr, clockCycle );
    clockCycle += i;
    z.stall = i;
    z.latency = i;
    z.needWrite = false;
    z.needRead = false;
    address >>= cacheTagsPtr->logLineSize;
    tagNum = address & (cacheTagsPtr->numTags - 1);
    tagPtr = &cacheTagsPtr->tags[tagNum];
    prefetchBufferNum = tagPtr->prefetchBufferNum;
    if ( tagPtr->address == address ) {
        if ( 0 <= prefetchBufferNum ) {
            prefetchBufferPtr =
                &cacheTagsPtr->prefetchBuffers[prefetchBufferNum];
            i = prefetchBufferPtr->clockCycle - clockCycle;
            if ( 0 < i ) {
                clockCycle += i;
                z.stall += i;
                z.latency += i;
            }
            prefetchBufferPtr->clockCycle = 0;
            tagPtr->prefetchBufferNum = -1;
        }
    } else {
        if ( 0 <= prefetchBufferNum ) {
            cacheTagsPtr->prefetchBuffers[prefetchBufferNum].clockCycle = 0;
            tagPtr->prefetchBufferNum = -1;
        }
        i = cacheTagsPtr->lastBusyClockCycle - clockCycle;
        if ( 0 < i ) {
            clockCycle += i;
            z.stall += i;
            z.latency += i;
        }
        clockCycle += cacheTagsPtr->readMissBusy;
        z.stall += cacheTagsPtr->readMissPenalty;
        z.latency += cacheTagsPtr->readMissLatency;
        z.needRead = true;
        if ( tagPtr->dirty ) {
            clockCycle += cacheTagsPtr->writeBackBusy;
            z.needWrite = true;
            z.writeAddress = (tagPtr->address)<<(cacheTagsPtr->logLineSize);
        }
        tagPtr->dirty = false;
        tagPtr->address = address;
    }
    cacheTagsPtr->lastBusyClockCycle = clockCycle;
    return z;

}

