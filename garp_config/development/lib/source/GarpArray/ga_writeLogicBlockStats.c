
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

enum { numModes = MODE_ADD3 + 1 };

static bool inputAActive( logicBlockConfigT *logicBlockConfigPtr )
{
    int mode;
    uint16_t table;

    mode = logicBlockConfigPtr->mode;
    if ( ! mode || (mode == MODE_VARSHIFT) ) return false;
    if (
        (mode == MODE_SELECT_1) || (mode == MODE_SELECT)
            || (mode == MODE_PARTSELECT) || (mode == MODE_ADD3)
    ) {
        return true;
    }
    table = logicBlockConfigPtr->table;
    if ( (table & 0x5555)<<1 == (table & 0xAAAA) ) return false;
    return true;

}

static bool crossbarAActive( logicBlockConfigT *logicBlockConfigPtr )
{
    int mode;

    if ( ! inputAActive( logicBlockConfigPtr ) ) return false;
    mode = logicBlockConfigPtr->mode;
    if (
        (mode == MODE_DIRECT) || (mode == MODE_INPUT5) || (mode == MODE_SPLIT)
            || (mode == MODE_CARRY)
    ) {
        return true;
    }
    return false;

}

static bool shiftInvertAActive( logicBlockConfigT *logicBlockConfigPtr )
{
    int mode;

    mode = logicBlockConfigPtr->mode;
    if (
        (mode == MODE_SELECT_1) || (mode == MODE_SELECT)
            || (mode == MODE_PARTSELECT) || (mode == MODE_ADD3)
    ) {
        return true;
    }
    return false;

}

static bool inputBActive( logicBlockConfigT *logicBlockConfigPtr )
{
    int mode;
    uint16_t table;

    mode = logicBlockConfigPtr->mode;
    if ( ! mode ) return false;
    if (
           (mode == MODE_SELECT_1)
        || (mode == MODE_SELECT)
        || (mode == MODE_PARTSELECT)
        || (mode == MODE_VARSHIFT)
        || (mode == MODE_ADD3)
    ) {
        return true;
    }
    table = logicBlockConfigPtr->table;
    if ( (table & 0x3333)<<2 == (table & 0xCCCC) ) return false;
    return true;

}

static bool crossbarBActive( logicBlockConfigT *logicBlockConfigPtr )
{
    int mode;

    if ( ! inputBActive( logicBlockConfigPtr ) ) return false;
    mode = logicBlockConfigPtr->mode;
    if (
        (mode == MODE_DIRECT) || (mode == MODE_INPUT5) || (mode == MODE_SPLIT)
            || (mode == MODE_CARRY)
    ) {
        return true;
    }
    return false;

}

static bool shiftInvertBActive( logicBlockConfigT *logicBlockConfigPtr )
{
    int mode;

    mode = logicBlockConfigPtr->mode;
    if (
           (mode == MODE_SELECT_1)
        || (mode == MODE_SELECT)
        || (mode == MODE_PARTSELECT)
        || (mode == MODE_VARSHIFT)
        || (mode == MODE_ADD3)
    ) {
        return true;
    }
    return false;

}

static bool inputCActive( logicBlockConfigT *logicBlockConfigPtr )
{
    int mode;
    uint16_t table;

    mode = logicBlockConfigPtr->mode;
    if ( ! mode ) return false;
    if (
           (mode == MODE_SELECT_1)
        || (mode == MODE_SELECT)
        || (mode == MODE_PARTSELECT)
        || (mode == MODE_VARSHIFT)
        || (mode == MODE_ADD3)
    ) {
        return true;
    }
    table = logicBlockConfigPtr->table;
    if ( (table & 0x0F0F)<<4 == (table & 0xF0F0) ) return false;
    return true;

}

static bool crossbarCActive( logicBlockConfigT *logicBlockConfigPtr )
{
    int mode;

    if ( ! inputCActive( logicBlockConfigPtr ) ) return false;
    mode = logicBlockConfigPtr->mode;
    if (
        (mode == MODE_DIRECT) || (mode == MODE_INPUT5) || (mode == MODE_SPLIT)
            || (mode == MODE_CARRY)
    ) {
        return true;
    }
    return false;

}

static bool shiftInvertCActive( logicBlockConfigT *logicBlockConfigPtr )
{
    int mode;

    mode = logicBlockConfigPtr->mode;
    if (
           (mode == MODE_SELECT_1)
        || (mode == MODE_SELECT)
        || (mode == MODE_PARTSELECT)
        || (mode == MODE_VARSHIFT)
        || (mode == MODE_ADD3)
    ) {
        return true;
    }
    return false;

}

static bool inputDActive( logicBlockConfigT *logicBlockConfigPtr )
{
    int mode;
    uint16_t table;

    if (
        (logicBlockConfigPtr->DIn != IN_CONSTANT)
            || logicBlockConfigPtr->DInNum || logicBlockConfigPtr->bufferD
    ) {
        return true;
    }
    mode = logicBlockConfigPtr->mode;
    if ( mode == MODE_SELECT ) return true;
    if ( mode != MODE_DIRECT ) return false;
    table = logicBlockConfigPtr->table;
    if ( (table & 0x00FF)<<8 == (table & 0xFF00) ) return false;
    return true;

}

static bool inputDFunctionActive( logicBlockConfigT *logicBlockConfigPtr )
{
    int mode;
    uint16_t table;

    mode = logicBlockConfigPtr->mode;
    if ( mode == MODE_SELECT ) return true;
    if ( mode != MODE_DIRECT ) return false;
    table = logicBlockConfigPtr->table;
    if ( (table & 0x00FF)<<8 == (table & 0xFF00) ) return false;
    return true;

}

static bool crossbarDActive( logicBlockConfigT *logicBlockConfigPtr )
{
    uint16_t table;

    if ( logicBlockConfigPtr->mode != MODE_DIRECT ) return false;
    table = logicBlockConfigPtr->table;
    if ( (table & 0x00FF)<<8 == (table & 0xFF00) ) return false;
    return true;

}

static int_fast32_t activeInputCounts[4];
static int_fast32_t inputConstantZeroCounts[4], inputConstantNonzeroCounts[4];
static int_fast32_t inputVariableCounts[4];
static int_fast32_t inputConstantZeroCount, inputConstantNonzeroCount;
static int_fast32_t inputRegisterZCount, inputRegisterDCount;
static int_fast32_t inputWireVCount;
static int_fast32_t inputWireHAboveCount, inputWireGAboveCount;
static int_fast32_t inputWireHCount, inputWireGCount;

static void incrementInputCounts( int i, int inCode, int inNum )
{

    switch ( inCode ) {
     case IN_CONSTANT:
        if ( ! inNum ) {
            ++inputConstantZeroCounts[i];
            ++inputConstantZeroCount;
        } else {
            ++inputConstantNonzeroCounts[i];
            ++inputConstantNonzeroCount;
        }
        return;
     case IN_ZREG:
        ++inputRegisterZCount;
        break;
     case IN_DREG:
        ++inputRegisterDCount;
        break;
     case IN_VWIRE:
     case IN_VROW:
        ++inputWireVCount;
        break;
     case IN_ABOVEHCOL:
        ++inputWireHAboveCount;
        break;
     case IN_ABOVEGWIRE:
        ++inputWireGAboveCount;
        break;
     case IN_HCOL:
        ++inputWireHCount;
        break;
     case IN_GWIRE:
        ++inputWireGCount;
        break;
    }
    ++inputVariableCounts[i];

}

static int_fast32_t crossbar00Count, crossbar01Count;
static int_fast32_t crossbar10Count, crossbar11Count;

static void incrementCrossbarCounts( int perturbCode )
{

    switch ( perturbCode ) {
     case PERTURB_00:
        ++crossbar00Count;
        break;
     case PERTURB_01:
        ++crossbar01Count;
        break;
     case PERTURB_10:
        ++crossbar10Count;
        break;
     case PERTURB_11:
        ++crossbar11Count;
        break;
    }

}

static int_fast32_t noShiftNoInvertCount, noShiftInvertCount;
static int_fast32_t shiftNoInvertCount, shiftInvertCount;

static void incrementShiftInvertCounts( int perturbCode )
{

    switch ( perturbCode ) {
     case PERTURB_10:
        ++noShiftNoInvertCount;
        break;
     case PERTURB_NOT:
        ++noShiftInvertCount;
        break;
     case PERTURB_SHIFT1:
        ++shiftNoInvertCount;
        break;
     case PERTURB_SHIFT1_NOT:
        ++shiftInvertCount;
        break;
    }

}

void ga_writeLogicBlockStats( FILE *streamPtr, GarpArrayT *GarpArrayPtr )
{
    int numRows;
    int_fast32_t numLogicBlocks, activeLogicBlockCount;
    rowConfigT *rowConfigPtr;
    unsigned int rowNum;
    logicBlockConfigT *logicBlockConfigPtr;
    int colNum;
    int_fast32_t activeInputCount, activeInputDFunctionCount;
    int_fast32_t activeCrossbarCount, activeShiftInvertCount;
    int_fast32_t modeCounts[numModes];
    int_fast32_t *modeCountPtr;
    int mode;
    uint16_t table;
    int_fast32_t activeFunctionCount, activeDPathCount, activeRegisterCount;
    int i;

    numRows = GarpArrayPtr->numConfigRows;
    numLogicBlocks = numRows * numLogicCols;
    activeLogicBlockCount = 0;
    rowConfigPtr = GarpArrayPtr->configPtr;
    for ( rowNum = numRows; rowNum; --rowNum ) {
        logicBlockConfigPtr = rowConfigPtr->logicCols;
        for ( colNum = numLogicCols; colNum; --colNum ) {
            activeLogicBlockCount += ! logicBlockConfigPtr->off;
            ++logicBlockConfigPtr;
        }
        ++rowConfigPtr;
    }
    fprintf(
        streamPtr,
        "Active logic blocks:  %ld  %.0f%%\n\n",
        (long) activeLogicBlockCount,
        100.0 * activeLogicBlockCount / numLogicBlocks
    );
    fputs(
  "Active logic blocks per column:\n"
  "    22    21    20    19    18    17    16    15    14    13    12    11\n",
        streamPtr
    );
    for ( colNum = numLogicCols - 1; 11 <= colNum; --colNum ) {
        activeLogicBlockCount = 0;
        rowConfigPtr = GarpArrayPtr->configPtr;
        for ( rowNum = numRows; rowNum; --rowNum ) {
            activeLogicBlockCount += ! rowConfigPtr->logicCols[colNum].off;
            ++rowConfigPtr;
        }
        fprintf( streamPtr, " %5ld", (long) activeLogicBlockCount );
    }
    fputs( "\n ", streamPtr );
    for ( colNum = numLogicCols - 1; 11 <= colNum; --colNum ) {
        activeLogicBlockCount = 0;
        rowConfigPtr = GarpArrayPtr->configPtr;
        for ( rowNum = numRows; rowNum; --rowNum ) {
            activeLogicBlockCount += ! rowConfigPtr->logicCols[colNum].off;
            ++rowConfigPtr;
        }
        fprintf(
            streamPtr, " %4.0f%%", 100.0 * activeLogicBlockCount / numRows );
    }
    fputs(
    "\n\n    10     9     8     7     6     5     4     3     2     1     0\n",
        streamPtr
    );
    for ( colNum = 10; 0 <= colNum; --colNum ) {
        activeLogicBlockCount = 0;
        rowConfigPtr = GarpArrayPtr->configPtr;
        for ( rowNum = numRows; rowNum; --rowNum ) {
            activeLogicBlockCount += ! rowConfigPtr->logicCols[colNum].off;
            ++rowConfigPtr;
        }
        fprintf( streamPtr, " %5ld", (long) activeLogicBlockCount );
    }
    fputs( "\n ", streamPtr );
    for ( colNum = 10; 0 <= colNum; --colNum ) {
        activeLogicBlockCount = 0;
        rowConfigPtr = GarpArrayPtr->configPtr;
        for ( rowNum = numRows; rowNum; --rowNum ) {
            activeLogicBlockCount += ! rowConfigPtr->logicCols[colNum].off;
            ++rowConfigPtr;
        }
        fprintf(
            streamPtr, " %4.0f%%", 100.0 * activeLogicBlockCount / numRows );
    }
    fputs( "\n\n", streamPtr );
    activeInputCount = 0;
    activeInputDFunctionCount = 0;
    for ( i = 0; i < 4; ++i ) {
        activeInputCounts[i] = 0;
        inputConstantZeroCounts[i] = 0;
        inputConstantNonzeroCounts[i] = 0;
        inputVariableCounts[i] = 0;
    }
    inputConstantZeroCount = 0;
    inputConstantNonzeroCount = 0;
    inputRegisterZCount = 0;
    inputRegisterDCount = 0;
    inputWireVCount = 0;
    inputWireHAboveCount = 0;
    inputWireGAboveCount = 0;
    inputWireHCount = 0;
    inputWireGCount = 0;
    rowConfigPtr = GarpArrayPtr->configPtr;
    for ( rowNum = numRows; rowNum; --rowNum ) {
        logicBlockConfigPtr = rowConfigPtr->logicCols;
        for ( colNum = numLogicCols; colNum; --colNum ) {
            if ( ! logicBlockConfigPtr->off ) {
                if ( inputAActive( logicBlockConfigPtr ) ) {
                    ++activeInputCount;
                    ++activeInputCounts[0];
                    incrementInputCounts(
                        0,
                        logicBlockConfigPtr->AIn,
                        logicBlockConfigPtr->AInNum
                    );
                }
                if ( inputBActive( logicBlockConfigPtr ) ) {
                    ++activeInputCount;
                    ++activeInputCounts[1];
                    incrementInputCounts(
                        1,
                        logicBlockConfigPtr->BIn,
                        logicBlockConfigPtr->BInNum
                    );
                }
                if ( inputCActive( logicBlockConfigPtr ) ) {
                    ++activeInputCount;
                    ++activeInputCounts[2];
                    incrementInputCounts(
                        2,
                        logicBlockConfigPtr->CIn,
                        logicBlockConfigPtr->CInNum
                    );
                }
                if ( inputDActive( logicBlockConfigPtr ) ) {
                    ++activeInputCount;
                    ++activeInputCounts[3];
                    incrementInputCounts(
                        3,
                        logicBlockConfigPtr->DIn,
                        logicBlockConfigPtr->DInNum
                    );
                    if ( inputDFunctionActive( logicBlockConfigPtr ) ) {
                        ++activeInputDFunctionCount;
                    }
                }
            }
            ++logicBlockConfigPtr;
        }
        ++rowConfigPtr;
    }
    for ( i = 0; i < 4; ++i ) {
        fprintf(
            streamPtr,
            "Active %c inputs:  %ld  %.0f%%\n",
            'A' + i,
            (long) activeInputCounts[i],
            100.0 * activeInputCounts[i] / numLogicBlocks
        );
        if ( activeInputCounts[i] ) {
            if ( i == 3 ) {
                fprintf(
                    streamPtr,
                    "Active D inputs, to function:  %ld  %.0f%%\n",
                    (long) activeInputDFunctionCount,
                    100.0 * activeInputDFunctionCount / numLogicBlocks
                );
            }
            fputs( "  zero  nonzero variable\n", streamPtr );
            fprintf(
                streamPtr,
                "%5ld %7ld %7ld\n",
                (long) inputConstantZeroCounts[i],
                (long) inputConstantNonzeroCounts[i],
                (long) inputVariableCounts[i]
            );
            fprintf(
                streamPtr,
                "%5.0f%% %6.0f%% %6.0f%%\n",
                100.0 * inputConstantZeroCounts[i] / activeInputCounts[i],
                100.0 * inputConstantNonzeroCounts[i] / activeInputCounts[i],
                100.0 * inputVariableCounts[i] / activeInputCounts[i]
            );
        }
        fputc( '\n', streamPtr );
    }
    fprintf(
        streamPtr,
        "Active inputs:  %ld  %.0f%%\n",
        (long) activeInputCount,
        25.0 * activeInputCount / numLogicBlocks
    );
    if ( activeInputCount ) {
        fputs(
     "  zero   nonzero  Zreg    Dreg      V    Habove  Gabove     H       G\n",
            streamPtr
        );
        fprintf(
            streamPtr,
            "%5ld %7ld %7ld %7ld %7ld %7ld %7ld %7ld %7ld\n",
            (long) inputConstantZeroCount,
            (long) inputConstantNonzeroCount,
            (long) inputRegisterZCount,
            (long) inputRegisterDCount,
            (long) inputWireVCount,
            (long) inputWireHAboveCount,
            (long) inputWireGAboveCount,
            (long) inputWireHCount,
            (long) inputWireGCount
        );
        fprintf(
            streamPtr,
   "%5.0f%% %6.0f%% %6.0f%% %6.0f%% %6.0f%% %6.0f%% %6.0f%% %6.0f%% %6.0f%%\n",
            100.0 * inputConstantZeroCount / activeInputCount,
            100.0 * inputConstantNonzeroCount / activeInputCount,
            100.0 * inputRegisterZCount / activeInputCount,
            100.0 * inputRegisterDCount / activeInputCount,
            100.0 * inputWireVCount / activeInputCount,
            100.0 * inputWireHAboveCount / activeInputCount,
            100.0 * inputWireGAboveCount / activeInputCount,
            100.0 * inputWireHCount / activeInputCount,
            100.0 * inputWireGCount / activeInputCount
        );
    }
    fputc( '\n', streamPtr );
    activeCrossbarCount = 0;
    crossbar00Count = 0;
    crossbar01Count = 0;
    crossbar10Count = 0;
    crossbar11Count = 0;
    rowConfigPtr = GarpArrayPtr->configPtr;
    for ( rowNum = numRows; rowNum; --rowNum ) {
        logicBlockConfigPtr = rowConfigPtr->logicCols;
        for ( colNum = numLogicCols; colNum; --colNum ) {
            if ( ! logicBlockConfigPtr->off ) {
                if ( crossbarAActive( logicBlockConfigPtr ) ) {
                    ++activeCrossbarCount;
                    incrementCrossbarCounts( logicBlockConfigPtr->perturbA );
                }
                if ( crossbarBActive( logicBlockConfigPtr ) ) {
                    ++activeCrossbarCount;
                    incrementCrossbarCounts( logicBlockConfigPtr->perturbB );
                }
                if ( crossbarCActive( logicBlockConfigPtr ) ) {
                    ++activeCrossbarCount;
                    incrementCrossbarCounts( logicBlockConfigPtr->perturbC );
                }
                if ( crossbarDActive( logicBlockConfigPtr ) ) {
                    ++activeCrossbarCount;
                    incrementCrossbarCounts( logicBlockConfigPtr->perturbD );
                }
            }
            ++logicBlockConfigPtr;
        }
        ++rowConfigPtr;
    }
    fprintf(
        streamPtr,
        "Active crossbars:  %ld  %.0f%%\n",
        (long) activeCrossbarCount,
        25.0 * activeCrossbarCount / numLogicBlocks
    );
    if ( activeCrossbarCount ) {
        fputs( " straight  low    high   reverse\n", streamPtr );
        fprintf(
            streamPtr,
            "%5ld %7ld %7ld %7ld\n",
            (long) crossbar10Count,
            (long) crossbar00Count,
            (long) crossbar11Count,
            (long) crossbar01Count
        );
        fprintf(
            streamPtr,
            "%5.0f%% %6.0f%% %6.0f%% %6.0f%%\n",
            100.0 * crossbar10Count / activeCrossbarCount,
            100.0 * crossbar00Count / activeCrossbarCount,
            100.0 * crossbar11Count / activeCrossbarCount,
            100.0 * crossbar01Count / activeCrossbarCount
        );
    }
    fputc( '\n', streamPtr );
    activeShiftInvertCount = 0;
    noShiftNoInvertCount = 0;
    noShiftInvertCount = 0;
    shiftNoInvertCount = 0;
    shiftInvertCount = 0;
    rowConfigPtr = GarpArrayPtr->configPtr;
    for ( rowNum = numRows; rowNum; --rowNum ) {
        logicBlockConfigPtr = rowConfigPtr->logicCols;
        for ( colNum = numLogicCols; colNum; --colNum ) {
            if ( ! logicBlockConfigPtr->off ) {
                if ( shiftInvertAActive( logicBlockConfigPtr ) ) {
                    ++activeShiftInvertCount;
                    incrementShiftInvertCounts(
                        logicBlockConfigPtr->perturbA );
                }
                if ( shiftInvertBActive( logicBlockConfigPtr ) ) {
                    ++activeShiftInvertCount;
                    incrementShiftInvertCounts(
                        logicBlockConfigPtr->perturbB );
                }
                if ( shiftInvertCActive( logicBlockConfigPtr ) ) {
                    ++activeShiftInvertCount;
                    incrementShiftInvertCounts(
                        logicBlockConfigPtr->perturbC );
                }
            }
            ++logicBlockConfigPtr;
        }
        ++rowConfigPtr;
    }
    fprintf(
        streamPtr,
        "Active shiftInverts:  %ld  %.0f%%\n",
        (long) activeShiftInvertCount,
        25.0 * activeShiftInvertCount / numLogicBlocks
    );
    if ( activeShiftInvertCount ) {
        fputs( " straight invert shift shift-invert\n", streamPtr );
        fprintf(
            streamPtr,
            "%5ld %7ld %7ld %7ld\n",
            (long) noShiftNoInvertCount,
            (long) noShiftInvertCount,
            (long) shiftNoInvertCount,
            (long) shiftInvertCount
        );
        fprintf(
            streamPtr,
            "%5.0f%% %6.0f%% %6.0f%% %6.0f%%\n",
            100.0 * noShiftNoInvertCount / activeShiftInvertCount,
            100.0 * noShiftInvertCount / activeShiftInvertCount,
            100.0 * shiftNoInvertCount / activeShiftInvertCount,
            100.0 * shiftInvertCount / activeShiftInvertCount
        );
    }
    fputc( '\n', streamPtr );
    modeCountPtr = modeCounts;
    for ( mode = numModes; mode; --mode ) *modeCountPtr++ = 0;
    rowConfigPtr = GarpArrayPtr->configPtr;
    for ( rowNum = numRows; rowNum; --rowNum ) {
        logicBlockConfigPtr = rowConfigPtr->logicCols;
        for ( colNum = numLogicCols; colNum; --colNum ) {
            if ( logicBlockConfigPtr->off ) {
                mode = 0;
            } else {
                mode = logicBlockConfigPtr->mode;
                if ( mode == MODE_DIRECT ) {
                    table = logicBlockConfigPtr->table;
                    if (
                        ! table || (table == 0xFF00) || (table == 0xF0F0)
                            || (table == 0xCCCC) || (table == 0xAAAA)
                    ) {
                        mode = 0;
                    }
                }
            }
            ++modeCounts[mode];
            ++logicBlockConfigPtr;
        }
        ++rowConfigPtr;
    }
    activeFunctionCount = numLogicBlocks - modeCounts[0];
    fprintf(
        streamPtr,
        "Active functions:  %ld  %.0f%%\n"
        " direct  input5   split  select p.select varshift carry   add3\n",
        (long) activeFunctionCount,
        100.0 * activeFunctionCount / numLogicBlocks
    );
    if ( activeFunctionCount ) {
        modeCountPtr = modeCounts + 1;
        fprintf( streamPtr, "%5ld", (long) *modeCountPtr++ );
        for ( mode = 2; mode < numModes; ++mode ) {
            if ( (mode == MODE_SELECT_1) || (mode == MODE_SELECT) ) {
                fprintf(
                    streamPtr, " %7ld", (long) *modeCountPtr + modeCountPtr[1]
                );
                modeCountPtr += 2;
                ++mode;
            } else {
                fprintf( streamPtr, " %7ld", (long) *modeCountPtr++ );
            }
        }
        fputc( '\n', streamPtr );
        modeCountPtr = modeCounts + 1;
        fprintf(
            streamPtr, "%5.0f%%", 100.0 * *modeCountPtr++ / activeFunctionCount
        );
        for ( mode = 2; mode < numModes; ++mode ) {
            if ( (mode == MODE_SELECT_1) || (mode == MODE_SELECT) ) {
                fprintf(
                    streamPtr,
                    " %6.0f%%",
                    100.0 * (*modeCountPtr + modeCountPtr[1])
                        / activeFunctionCount
                );
                modeCountPtr += 2;
                ++mode;
            } else {
                fprintf(
                    streamPtr,
                    " %6.0f%%",
                    100.0 * *modeCountPtr++ / activeFunctionCount
                );
            }
        }
    }
    fputs( "\n\n", streamPtr );
    activeDPathCount = 0;
    activeRegisterCount = 0;
    rowConfigPtr = GarpArrayPtr->configPtr;
    for ( rowNum = numRows; rowNum; --rowNum ) {
        logicBlockConfigPtr = rowConfigPtr->logicCols;
        for ( colNum = numLogicCols; colNum; --colNum ) {
            if ( ! logicBlockConfigPtr->off ) {
                if ( logicBlockConfigPtr->bufferZ ) ++activeRegisterCount;
                if ( logicBlockConfigPtr->bufferD ) {
                    ++activeDPathCount;
                    ++activeRegisterCount;
                }
            }
            ++logicBlockConfigPtr;
        }
        ++rowConfigPtr;
    }
    fprintf(
        streamPtr,
        "Active registers:  %ld  %.0f%%\n\n",
        (long) activeRegisterCount,
        50.0 * activeRegisterCount / numLogicBlocks
    );

}

