
#include <stdint.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

int_fast32_t ga_regL( GarpArrayT *GarpArrayPtr, int regNum, int offset )
{
    int rowNum;
    logicBlockStateT *logicBlockStatePtr;
    uint_fast32_t z;
    int shift, i;

    rowNum = regNum>>1;
    if (
        (GarpArrayPtr->numRows <= rowNum) || (offset < 0)
            || (numLogicCols < offset)
    ) {
        return 0;
    }
    logicBlockStatePtr =
        &GarpArrayPtr->phase0StatePtr[rowNum].logicCols[offset];
    z = 0;
    shift = 0;
    i = 15;
    if ( regNum & 1 ) {
        for ( ; 0 <= i; --i ) {
            if ( numLogicCols < offset ) break;
            z |= logicBlockStatePtr->DReg<<shift;
            ++logicBlockStatePtr;
            shift += 2;
            ++offset;
        }
    } else {
        for ( ; 0 <= i; --i ) {
            if ( numLogicCols < offset ) break;
            z |= logicBlockStatePtr->ZReg<<shift;
            ++logicBlockStatePtr;
            shift += 2;
            ++offset;
        }
    }
    return z;

}

