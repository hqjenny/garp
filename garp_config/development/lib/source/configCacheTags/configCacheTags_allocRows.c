
#include <stdint.h>
#include "platform.h"
#include "configCacheTags.h"
#include "configCacheTagsStruct.h"

void
 configCacheTags_allocRows(
     configCacheTagsT *configCacheTagsPtr,
     uint_fast32_t numRowsToAllocate,
     uint_fast32_t address,
     uint_fast32_t rowOffset
 )
{
    int numContexts;
    uint_fast32_t numRows, rowAlignment;
    uint32_t *rowAddresses, *rowLastLoads;
    uint_fast32_t bestMatch, match, allocationStartRowNum, rowJumpSize, rowNum;
    int contextNum;
    uint32_t *rowAddressPtr, *rowLastLoadPtr;
    uint_fast32_t r;

    if ( numRowsToAllocate < rowOffset ) rowOffset = 0;
    numContexts = configCacheTagsPtr->numContexts;
    numRows = configCacheTagsPtr->numRows;
    rowAlignment = configCacheTagsPtr->minAllocationAlignment;
    rowAddresses = configCacheTagsPtr->rowAddresses;
    rowLastLoads = configCacheTagsPtr->rowLastLoads;
    while ( rowAlignment < numRowsToAllocate ) rowAlignment <<= 1;
    bestMatch = -1;
    for (
        rowJumpSize = rowAlignment;
        rowJumpSize <= numRows;
        rowJumpSize += rowJumpSize
    ) {
        for (
            rowNum = rowJumpSize - rowAlignment;
            rowNum < numRows;
            rowNum += rowJumpSize + rowJumpSize
        ) {
            rowAddressPtr = rowAddresses + rowNum;
            rowLastLoadPtr = rowLastLoads + rowNum;
            for ( contextNum = 0; contextNum < numContexts; ++contextNum ) {
                if ( address && (rowAddressPtr[rowOffset] == address) ) {
                    allocationStartRowNum = rowNum;
                    goto allocationChosen;
                }
                match = 0;
                for ( r = 0; r < numRowsToAllocate; ++r ) {
                    match += rowLastLoadPtr[r];
                }
                if ( match < bestMatch ) {
                    bestMatch = match;
                    allocationStartRowNum = rowNum;
                }
                rowAddressPtr += numRows;
                rowLastLoadPtr += numRows;
            }
        }
    }
 allocationChosen:
    configCacheTagsPtr->allocationStartRowNum = allocationStartRowNum;
    configCacheTagsPtr->allocationEndRowNum =
        allocationStartRowNum + numRowsToAllocate;

}

