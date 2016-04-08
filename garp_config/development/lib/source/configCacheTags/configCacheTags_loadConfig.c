
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "configCacheTags.h"
#include "configCacheTagsStruct.h"

int
 configCacheTags_loadConfig(
     configCacheTagsT *configCacheTagsPtr,
     uint_fast32_t address,
     uint_fast32_t rowOffset,
     uint_fast32_t numRowsToLoad
 )
{
    uint_fast32_t allocationEndRowNum, loadStartRowNum;
    int numContexts;
    uint_fast32_t numRows, bestMatch, match;
    int contextNum, loadContextNum;
    uint32_t *rowAddressPtr;
    uint_fast32_t loadConfigCount;
    uint32_t *rowLastLoadPtr;
    uint_fast32_t r;

    allocationEndRowNum = configCacheTagsPtr->allocationEndRowNum;
    loadStartRowNum = configCacheTagsPtr->allocationStartRowNum + rowOffset;
    if ( allocationEndRowNum < loadStartRowNum ) return false;
    if ( allocationEndRowNum < loadStartRowNum + numRowsToLoad ) {
        numRowsToLoad = allocationEndRowNum - loadStartRowNum;
    }
    numContexts = configCacheTagsPtr->numContexts;
    numRows = configCacheTagsPtr->numRows;
    rowAddressPtr = configCacheTagsPtr->rowAddresses + loadStartRowNum;
    loadConfigCount = configCacheTagsPtr->loadConfigCount + 1;
    configCacheTagsPtr->loadConfigCount = loadConfigCount;
    rowLastLoadPtr = configCacheTagsPtr->rowLastLoads + loadStartRowNum;
    bestMatch = -1;
    for ( contextNum = 0; contextNum < numContexts; ++contextNum ) {
        if ( *rowAddressPtr == address ) {
            for ( r = 1; r < numRowsToLoad; ++r ) {
                if ( ! (rowAddressPtr[r] & 1) ) goto notAlreadyLoaded;
            }
            for ( r = 0; r < numRowsToLoad; ++r ) {
                *rowLastLoadPtr++ = loadConfigCount;
            }
            return false;
        }
     notAlreadyLoaded:
        match = 0;
        for ( r = 0; r < numRowsToLoad; ++r ) match += rowLastLoadPtr[r];
        if ( match < bestMatch ) {
            bestMatch = match;
            loadContextNum = contextNum;
        }
        rowAddressPtr += numRows;
        rowLastLoadPtr += numRows;
    }
    rowAddressPtr =
        configCacheTagsPtr->rowAddresses + loadContextNum * numRows
            + loadStartRowNum;
    rowLastLoadPtr =
        configCacheTagsPtr->rowLastLoads + loadContextNum * numRows
            + loadStartRowNum;
    *rowAddressPtr++ = address;
    *rowLastLoadPtr++ = loadConfigCount;
    for ( r = 1; r < numRowsToLoad; ++r ) {
        *rowAddressPtr++ = -1;
        *rowLastLoadPtr++ = loadConfigCount;
    }
    if (
        (loadStartRowNum + numRowsToLoad < numRows) && (*rowAddressPtr & 1)
    ) {
        *rowAddressPtr = -2;
    }
    return true;

}

