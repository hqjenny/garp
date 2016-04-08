
#include <stdint.h>
#include "platform.h"
#include "allocate.h"
#include "configCacheTags.h"
#include "configCacheTagsStruct.h"

configCacheTagsT *
 configCacheTags_new(
     int numContexts, uint_fast32_t numRows, int minAllocationAlignment )
{
    configCacheTagsT *z;
    uint32_t *rowAddresses, *rowLastLoads;
    int contextNum;
    uint_fast32_t rowNum;

    z = allocType( configCacheTagsT );
    z->numContexts = numContexts;
    z->numRows = numRows;
    z->minAllocationAlignment = minAllocationAlignment;
    z->allocationStartRowNum = 0;
    z->allocationEndRowNum = 0;
    rowAddresses = allocArrayType( uint32_t, numContexts * numRows );
    z->rowAddresses = rowAddresses;
    z->loadConfigCount = 0;
    rowLastLoads = allocArrayType( uint32_t, numContexts * numRows );
    z->rowLastLoads = rowLastLoads;
    for ( contextNum = 0; contextNum < numContexts; ++contextNum ) {
        for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
            *rowAddresses++ = -1;
            *rowLastLoads++ = 0;
        }
    }
    return z;

}

