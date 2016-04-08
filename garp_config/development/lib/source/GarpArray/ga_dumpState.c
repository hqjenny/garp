
#include <stdint.h>
#include <stdio.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

static
 void ga_dumpRowZState( FILE *streamPtr, GarpArrayT *GarpArrayPtr, int rowNum )
{
    rowStateT *rowStatePtr;
    uint32_t hi, lo;
    int colNum;

    rowStatePtr = &GarpArrayPtr->phase0StatePtr[rowNum];
    hi = 0;
    for ( colNum = numLogicCols - 1; 16 <= colNum; --colNum ) {
        hi = (hi<<2) | rowStatePtr->logicCols[colNum].ZReg;
    }
    lo = 0;
    for ( ; 0 <= colNum; --colNum ) {
        lo = (lo<<2) | rowStatePtr->logicCols[colNum].ZReg;
    }
    fprintf( streamPtr, "Z:%04X%08X", hi, lo );

}

static
 void ga_dumpRowDState( FILE *streamPtr, GarpArrayT *GarpArrayPtr, int rowNum )
{
    rowStateT *rowStatePtr;
    uint32_t hi, lo;
    int colNum;

    rowStatePtr = &GarpArrayPtr->phase0StatePtr[rowNum];
    hi = 0;
    for ( colNum = numLogicCols - 1; 16 <= colNum; --colNum ) {
        hi = (hi<<2) | rowStatePtr->logicCols[colNum].DReg;
    }
    lo = 0;
    for ( ; 0 <= colNum; --colNum ) {
        lo = (lo<<2) | rowStatePtr->logicCols[colNum].DReg;
    }
    fprintf( streamPtr, "D:%04X%08X", hi, lo );

}

static
 void
  ga_dumpRowHOutState( FILE *streamPtr, GarpArrayT *GarpArrayPtr, int rowNum )
{
    rowStateT *rowStatePtr;
    uint32_t hi, lo;
    int colNum;

    rowStatePtr = &GarpArrayPtr->phase0StatePtr[rowNum];
    hi = 0;
    for ( colNum = numLogicCols - 1; 16 <= colNum; --colNum ) {
        hi = (hi<<2) | rowStatePtr->logicCols[colNum].HOut;
    }
    lo = 0;
    for ( ; 0 <= colNum; --colNum ) {
        lo = (lo<<2) | rowStatePtr->logicCols[colNum].HOut;
    }
    fprintf( streamPtr, "H:%04X%08X", hi, lo );

}

static
 void
  ga_dumpRowVOutState( FILE *streamPtr, GarpArrayT *GarpArrayPtr, int rowNum )
{
    rowStateT *rowStatePtr;
    uint32_t hi, lo;
    int colNum;

    rowStatePtr = &GarpArrayPtr->phase0StatePtr[rowNum];
    hi = 0;
    for ( colNum = numLogicCols - 1; 16 <= colNum; --colNum ) {
        hi = (hi<<2) | rowStatePtr->logicCols[colNum].VOut;
    }
    lo = 0;
    for ( ; 0 <= colNum; --colNum ) {
        lo = (lo<<2) | rowStatePtr->logicCols[colNum].VOut;
    }
    fprintf( streamPtr, "V:%04X%08X", hi, lo );

}

static
 void
  ga_dumpRowGWiresState(
      FILE *streamPtr, GarpArrayT *GarpArrayPtr, int rowNum )
{
    rowStateT *rowStatePtr;
    uint32_t lo;
    int wireNum;

    rowStatePtr = &GarpArrayPtr->phase0StatePtr[rowNum];
    lo = 0;
    for ( wireNum = numGWires - 1; 0 <= wireNum; --wireNum ) {
        lo = (lo<<2) | rowStatePtr->GWires[wireNum];
    }
    fprintf( streamPtr, "G:%02X", lo );

}

static
 void
  ga_dumpControlInputState(
      FILE *streamPtr, GarpArrayT *GarpArrayPtr, int rowNum )
{
    rowStateT *rowStatePtr;
    int controlEnable, controlB, controlC, controlD;

    rowStatePtr = &GarpArrayPtr->phase0StatePtr[rowNum];
    controlEnable = rowStatePtr->controlEnable;
    controlB = rowStatePtr->controlB && controlEnable;
    controlC = rowStatePtr->controlC && controlEnable;
    controlD = rowStatePtr->controlD && controlEnable;
    fprintf(
        streamPtr,
        "C:%c%c%c",
        rowStatePtr->controlB ? 'B' : '.',
        rowStatePtr->controlC ? 'C' : '.',
        rowStatePtr->controlD ? 'D' : '.'
    );

}

void ga_dumpState( FILE *streamPtr, GarpArrayT *GarpArrayPtr )
{
    int rowNum;

    for ( rowNum = 0; rowNum < GarpArrayPtr->numRows; ++rowNum ) {
        fprintf( streamPtr, "%3d: ", rowNum );
        ga_dumpRowZState( streamPtr, GarpArrayPtr, rowNum );
        fputc( ' ', streamPtr );
        ga_dumpRowDState( streamPtr, GarpArrayPtr, rowNum );
        fputc( ' ', streamPtr );
        ga_dumpRowHOutState( streamPtr, GarpArrayPtr, rowNum );
        fputc( ' ', streamPtr );
        ga_dumpRowVOutState( streamPtr, GarpArrayPtr, rowNum );
        fputc( ' ', streamPtr );
        ga_dumpRowGWiresState( streamPtr, GarpArrayPtr, rowNum );
        fputc( ' ', streamPtr );
        ga_dumpControlInputState( streamPtr, GarpArrayPtr, rowNum );
        fputc( '\n', streamPtr );
    }

}

