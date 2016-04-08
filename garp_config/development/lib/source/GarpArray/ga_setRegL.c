
#include <stdint.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

void
 ga_setRegL(
     GarpArrayT *GarpArrayPtr, int regNum, int size, int offset, int_fast32_t a
 )
{
    int rowNum, configRowNum;
    logicBlockStateT *logicBlockStatePtr;
    logicBlockConfigT *logicBlockConfigPtr;
    int GOutNum, value;

    rowNum = regNum>>1;
    if (
        (GarpArrayPtr->numRows <= rowNum) || (offset < 0)
            || (numLogicCols < offset)
    ) {
        return;
    }
    configRowNum = rowNum - GarpArrayPtr->configRowOffset;
    logicBlockStatePtr =
        &GarpArrayPtr->phase0StatePtr[rowNum].logicCols[offset];
    logicBlockConfigPtr =
        &GarpArrayPtr->configPtr[configRowNum].logicCols[offset];
    if ( regNum & 1 ) {
        while ( size-- ) {
            if ( numLogicCols < offset ) break;
            value = a & 3;
            logicBlockStatePtr->DReg = value;
            if ( ! logicBlockConfigPtr->off ) {
                if (
                    logicBlockConfigPtr->bufferD
                        || (logicBlockConfigPtr->DIn == IN_DREG)
                ) {
                    if ( logicBlockConfigPtr->HSelect == SELECT_D ) {
                        logicBlockStatePtr->HOut = value;
                    }
                    if ( logicBlockConfigPtr->GSelect == SELECT_D ) {
                        GOutNum = logicBlockConfigPtr->GOutNum;
                        if ( GOutNum < numGWires ) {
                            GarpArrayPtr
                                ->phase0StatePtr[rowNum].GWires[GOutNum] =
                                value;
                        }
                    }
                    if ( logicBlockConfigPtr->VSelect == SELECT_D ) {
                        logicBlockStatePtr->VOut = value;
                    }
                }
            }
            ++logicBlockStatePtr;
            ++logicBlockConfigPtr;
            a >>= 2;
            ++offset;
        }
    } else {
        while ( size-- ) {
            if ( numLogicCols < offset ) break;
            value = a & 3;
            logicBlockStatePtr->ZReg = value;
            if ( ! logicBlockConfigPtr->off ) {
                if ( logicBlockConfigPtr->bufferZ ) {
                    if ( logicBlockConfigPtr->HSelect == SELECT_Z ) {
                        logicBlockStatePtr->HOut = value;
                    }
                    if ( logicBlockConfigPtr->GSelect == SELECT_Z ) {
                        GOutNum = logicBlockConfigPtr->GOutNum;
                        if ( GOutNum < numGWires ) {
                            GarpArrayPtr
                                ->phase0StatePtr[rowNum].GWires[GOutNum] =
                                value;
                        }
                    }
                    if ( logicBlockConfigPtr->VSelect == SELECT_Z ) {
                        logicBlockStatePtr->VOut = value;
                    }
                }
                if ( logicBlockConfigPtr->DIn == IN_ZREG ) {
                    if ( logicBlockConfigPtr->HSelect == SELECT_D ) {
                        logicBlockStatePtr->HOut = value;
                    }
                    if ( logicBlockConfigPtr->GSelect == SELECT_D ) {
                        GOutNum = logicBlockConfigPtr->GOutNum;
                        if ( GOutNum < numGWires ) {
                            GarpArrayPtr
                                ->phase0StatePtr[rowNum].GWires[GOutNum] =
                                value;
                        }
                    }
                    if ( logicBlockConfigPtr->VSelect == SELECT_D ) {
                        logicBlockStatePtr->VOut = value;
                    }
                }
            }
            ++logicBlockStatePtr;
            ++logicBlockConfigPtr;
            a >>= 2;
            ++offset;
        }
    }

}

