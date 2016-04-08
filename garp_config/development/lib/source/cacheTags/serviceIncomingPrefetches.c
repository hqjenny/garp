
#include <stdint.h>
#include "platform.h"
#include "cacheTags.h"
#include "cacheTagsX.h"

uint_fast32_t
 cacheTags_z_serviceIncomingPrefetches(
     cacheTagsT *cacheTagsPtr, uint_fast32_t clockCycle )
{
    int stall;
    prefetchBufferT *prefetchBufferPtr;
    int i;

    stall = 0;
 restart:
    prefetchBufferPtr = &cacheTagsPtr->prefetchBuffers[0];
    for ( i = cacheTagsPtr->numPrefetchBuffers; i; --i ) {
        if ( prefetchBufferPtr->clockCycle == clockCycle ) {
            ++stall;
            ++clockCycle;
            goto restart;
        }
        ++prefetchBufferPtr;
    }
    return stall;

}

