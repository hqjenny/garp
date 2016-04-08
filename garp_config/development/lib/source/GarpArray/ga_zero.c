
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

void ga_zero( GarpArrayT *GarpArrayPtr )
{
    int rowNum, colNum, wireNum;
    logicBlockStateT *logicBlockStatePtr;

    for ( rowNum = 0; rowNum < GarpArrayPtr->numRows; ++rowNum ) {
        logicBlockStatePtr =
            &GarpArrayPtr->phase0StatePtr[rowNum].logicCols[numLogicCols - 1];
        for ( colNum = numLogicCols - 1; 0 <= colNum; --colNum ) {
            logicBlockStatePtr->ZReg = 0;
            logicBlockStatePtr->DReg = 0;
            logicBlockStatePtr->HOut = 0;
            logicBlockStatePtr->VOut = 0;
            --logicBlockStatePtr;
        }
        for ( wireNum = 0; wireNum < numGWires; ++wireNum ) {
            GarpArrayPtr->phase0StatePtr[rowNum].GWires[wireNum] = 0;
        }
        GarpArrayPtr->phase0StatePtr[rowNum].controlEnable = 0;
        GarpArrayPtr->phase0StatePtr[rowNum].controlB = 0;
        GarpArrayPtr->phase0StatePtr[rowNum].controlC = 0;
        GarpArrayPtr->phase0StatePtr[rowNum].controlD = 0;
    }
    GarpArrayPtr->clock = 0;

}

