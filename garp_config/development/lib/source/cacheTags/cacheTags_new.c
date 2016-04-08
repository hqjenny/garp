
#include <stdint.h>
#include "platform.h"
#include "cacheTags.h"
#include "allocate.h"
#include "cacheTagsX.h"

cacheTagsT * cacheTags_new( const cacheTags_paramsT *paramsPtr )
{
    cacheTagsT *cacheTagsPtr;
    int lineSize, logLineSize;
    int_fast32_t numTags;
    cacheTagT *tags;
    int_fast32_t i;
    int numPrefetchBuffers;
    prefetchBufferT *prefetchBuffers;

    cacheTagsPtr = allocType( cacheTagsT );
    lineSize = paramsPtr->lineSize;
    cacheTagsPtr->lineSize = lineSize;
    logLineSize = 0;
    while ( 1 < lineSize ) {
        lineSize >>= 1;
        ++logLineSize;
    }
    numTags = paramsPtr->size>>logLineSize;
    cacheTagsPtr->numTags = numTags;
    cacheTagsPtr->logLineSize = logLineSize;
    cacheTagsPtr->readMissPenalty = paramsPtr->readMissPenalty;
    cacheTagsPtr->readMissLatency = paramsPtr->readMissLatency;
    cacheTagsPtr->readMissBusy = paramsPtr->readMissBusy;
    cacheTagsPtr->writeMissPenalty = paramsPtr->writeMissPenalty;
    cacheTagsPtr->writeMissBusy = paramsPtr->writeMissBusy;
    cacheTagsPtr->writeBackBusy = paramsPtr->writeBackBusy;
    cacheTagsPtr->lastBusyClockCycle = 0;
    tags = allocArrayType( cacheTagT, numTags );
    cacheTagsPtr->tags = tags;
    for ( i = numTags; 0 < i; --i ) {
        tags->address = -1;
        tags->dirty = false;
        tags->prefetchBufferNum = -1;
        ++tags;
    }
    numPrefetchBuffers = paramsPtr->numPrefetchBuffers;
    cacheTagsPtr->numPrefetchBuffers = numPrefetchBuffers;
    prefetchBuffers = allocArrayType( prefetchBufferT, numPrefetchBuffers );
    cacheTagsPtr->prefetchBuffers = prefetchBuffers;
    for ( i = numPrefetchBuffers; 0 < i; --i ) {
        prefetchBuffers->clockCycle = 0;
        ++prefetchBuffers;
    }
    return cacheTagsPtr;

}

