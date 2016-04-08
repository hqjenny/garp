
#include <stdint.h>
#include "platform.h"
#include "cacheTags.h"
#include "cacheTagsX.h"

void
 cacheTags_extendPrefetchLatency(
     cacheTagsT *cacheTagsPtr, uint_fast32_t address, int latency )
{
    int_fast32_t tagNum;
    cacheTagT *tagPtr;
    int prefetchBufferNum;
    uint_fast32_t clockCycle;
    prefetchBufferT *prefetchBufferPtr;
    int i;

    address >>= cacheTagsPtr->logLineSize;
    tagNum = address & (cacheTagsPtr->numTags - 1);
    tagPtr = &cacheTagsPtr->tags[tagNum];
    prefetchBufferNum = tagPtr->prefetchBufferNum;
    if ( prefetchBufferNum < 0 ) return;
    clockCycle =
        cacheTagsPtr->prefetchBuffers[prefetchBufferNum].clockCycle + latency;
 restart:
    prefetchBufferPtr = &cacheTagsPtr->prefetchBuffers[0];
    for ( i = cacheTagsPtr->numPrefetchBuffers; i; --i ) {
        if ( prefetchBufferPtr->clockCycle == clockCycle ) {
            ++clockCycle;
            goto restart;
        }
        ++prefetchBufferPtr;
    }
    cacheTagsPtr->prefetchBuffers[prefetchBufferNum].clockCycle = clockCycle;

}

