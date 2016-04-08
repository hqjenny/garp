
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "cacheTags.h"
#include "cacheTagsX.h"

cacheTags_outT
 cacheTags_prefetch(
     cacheTagsT *cacheTagsPtr, uint_fast32_t clockCycle, uint_fast32_t address
 )
{
    int_fast32_t i;
    cacheTags_outT z;
    int_fast32_t tagNum;
    cacheTagT *tagPtr;
    int prefetchBufferNum;
    uint_fast32_t minClockCycle;
    prefetchBufferT *prefetchBufferPtr;
    int minPrefetchBufferNum;

    i = cacheTags_z_serviceIncomingPrefetches( cacheTagsPtr, clockCycle );
    clockCycle += i;
    z.stall = i;
    z.latency = 0;
    z.needWrite = false;
    z.needRead = false;
    address >>= cacheTagsPtr->logLineSize;
    tagNum = address & (cacheTagsPtr->numTags - 1);
    tagPtr = &cacheTagsPtr->tags[tagNum];
    if ( tagPtr->address != address ) {
        prefetchBufferNum = tagPtr->prefetchBufferNum;
        if ( 0 <= prefetchBufferNum ) {
            cacheTagsPtr->prefetchBuffers[prefetchBufferNum].clockCycle = 0;
            tagPtr->prefetchBufferNum = -1;
        }
        i = cacheTagsPtr->lastBusyClockCycle - clockCycle;
        if ( 0 < i ) {
            clockCycle += i;
            z.stall += i;
        }
        minClockCycle = -1;
        prefetchBufferPtr = &cacheTagsPtr->prefetchBuffers[0];
        i = cacheTagsPtr->numPrefetchBuffers;
        for (
            prefetchBufferNum = 0; prefetchBufferNum != i; ++prefetchBufferNum
        ) {
            if ( prefetchBufferPtr->clockCycle < minClockCycle ) {
                minPrefetchBufferNum = prefetchBufferNum;
                minClockCycle = prefetchBufferPtr->clockCycle;
                if ( minClockCycle < clockCycle ) goto freeBuffer;
            }
            ++prefetchBufferPtr;
        }
        z.stall += minClockCycle - clockCycle;
        clockCycle = minClockCycle;
     freeBuffer:
        prefetchBufferPtr =
            &cacheTagsPtr->prefetchBuffers[minPrefetchBufferNum];
        cacheTagsPtr->tags[prefetchBufferPtr->tagNum].prefetchBufferNum = -1;
        prefetchBufferPtr->tagNum = tagNum;
        prefetchBufferPtr->clockCycle =
            clockCycle + cacheTagsPtr->readMissLatency;
        tagPtr->prefetchBufferNum = minPrefetchBufferNum;
        clockCycle += cacheTagsPtr->readMissBusy;
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

