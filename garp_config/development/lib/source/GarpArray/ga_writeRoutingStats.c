
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

enum { maxWirePathLength = 4 };

static int findGWireColNum( GarpArrayT *GarpArrayPtr, int rowNum, int wireNum )
{
    logicBlockConfigT *logicBlockConfigPtr;
    int colNum;

    colNum = 0;
    logicBlockConfigPtr = GarpArrayPtr->configPtr[rowNum].logicCols;
    while (
        (logicBlockConfigPtr->off || (logicBlockConfigPtr->GOutNum != wireNum))
            && (colNum < numLogicCols)
    ) {
        ++colNum;
        ++logicBlockConfigPtr;
    }
    return colNum;

}

static int_fast32_t wirePathCount;
static int_fast32_t wirePathLengthCounts[maxWirePathLength];
static int_fast32_t singleVWireCount, singleHWireCount, singleGWireCount;
static int_fast32_t connectionVectors[13][12];

static int offsetToConnectionVectorIndex( int offset )
{

    return
          (! offset) ? 6
        : (offset < 0) ?
              (  (offset == -1) ? 5
               : ( -4 < offset) ? 4
               : ( -8 < offset) ? 3
               : (-16 < offset) ? 2
               : (-24 < offset) ? 1
               : 0
              )
        : (  (offset == 1) ? 7
           : (offset <  4) ? 8
           : (offset <  8) ? 9
           : (offset < 16) ? 10
           : (offset < 24) ? 11
           : 12
          );

}

static
 void
  updateInputPathLengths(
      GarpArrayT *GarpArrayPtr, int rowNum, int colNum, int in, int inNum )
{
    enum { out_V, out_H, out_G };
    int originalRowNum, originalColNum, wireType, loopLength, pathLength, out;
    logicBlockConfigT *logicBlockConfigPtr;
    int select;

    originalRowNum = rowNum;
    originalColNum = colNum;
    wireType = 0;
    loopLength = 0;
    pathLength = 0;
    do {
        switch ( in ) {
         case IN_CONSTANT:
         case IN_VWIRE:
            goto done;
         case IN_ZREG:
            ++loopLength;
            logicBlockConfigPtr =
                &GarpArrayPtr->configPtr[rowNum].logicCols[colNum];
            goto upZPath;
         case IN_DREG:
            ++loopLength;
            logicBlockConfigPtr =
                &GarpArrayPtr->configPtr[rowNum].logicCols[colNum];
            goto upDPath;
         case IN_VROW:
            wireType = IN_VROW;
            rowNum = inNum;
            out = out_V;
            break;
         case IN_ABOVEHCOL:
            --rowNum;
         case IN_HCOL:
            wireType = IN_HCOL;
            colNum = inNum;
            out = out_H;
            break;
         case IN_ABOVEGWIRE:
            --rowNum;
         case IN_GWIRE:
            wireType = IN_GWIRE;
            colNum = findGWireColNum( GarpArrayPtr, rowNum, inNum );
            out = out_G;
            break;
        }
        loopLength = 0;
        ++pathLength;
        logicBlockConfigPtr =
            &GarpArrayPtr->configPtr[rowNum].logicCols[colNum];
        switch ( out ) {
         case out_V:
            select = logicBlockConfigPtr->VSelect;
            break;
         case out_H:
            select = logicBlockConfigPtr->HSelect;
            break;
         case out_G:
            select = logicBlockConfigPtr->GSelect;
            break;
        }
        switch ( select ) {
         case SELECT_Z:
     upZPath:
            ++logicBlockConfigPtr->temp0;
            if ( logicBlockConfigPtr->mode != MODE_DIRECT ) goto done;
            switch ( logicBlockConfigPtr->table ) {
             case 0xAAAA:
                in = logicBlockConfigPtr->AIn;
                inNum = logicBlockConfigPtr->AInNum;
                break;
             case 0xCCCC:
                in = logicBlockConfigPtr->BIn;
                inNum = logicBlockConfigPtr->BInNum;
                break;
             case 0xF0F0:
                in = logicBlockConfigPtr->CIn;
                inNum = logicBlockConfigPtr->CInNum;
                break;
             case 0xFF00:
                in = logicBlockConfigPtr->DIn;
                inNum = logicBlockConfigPtr->DInNum;
                break;
             default:
                goto done;
            }
            break;
         case SELECT_D:
     upDPath:
            ++logicBlockConfigPtr->temp1;
            in = logicBlockConfigPtr->DIn;
            inNum = logicBlockConfigPtr->DInNum;
            break;
        }
    } while ( (pathLength < maxWirePathLength) && (loopLength < 2) );
    if ( maxWirePathLength <= pathLength ) {
        originalRowNum = rowNum;
        originalColNum = colNum;
    }
 done:
    if ( pathLength ) {
        --pathLength;
        ++wirePathCount;
        ++wirePathLengthCounts[pathLength];
        ++connectionVectors
              [offsetToConnectionVectorIndex(rowNum - originalRowNum)]
              [offsetToConnectionVectorIndex(colNum - originalColNum)];
        if ( ! pathLength ) {
            switch ( wireType ) {
             case IN_VROW:
                ++singleVWireCount;
                break;
             case IN_HCOL:
                ++singleHWireCount;
                break;
             case IN_GWIRE:
                ++singleGWireCount;
                break;
            }
        }
    }

}

static bool functionIsActive( logicBlockConfigT *logicBlockConfigPtr )
{
    uint16_t table;

    if ( logicBlockConfigPtr->mode != MODE_DIRECT ) return true;
    table = logicBlockConfigPtr->table;
    if ( table == 0x0000 ) return false;
    if ( table == 0xAAAA ) return false;
    if ( table == 0xCCCC ) return false;
    if ( table == 0xF0F0 ) return false;
    if ( table == 0xFF00 ) return false;
    return true;

}

void ga_writeRoutingStats( FILE *streamPtr, GarpArrayT *GarpArrayPtr )
{
    int numRows;
    int_fast32_t numLogicBlocks;
    rowConfigT *rowConfigPtr;
    int rowNum;
    logicBlockConfigT *logicBlockConfigPtr;
    int colNum, in, inNum;
    uint16_t table;
    int i, j;
    int_fast32_t singleWireCount, activeDPathCount;

/* temp0 is Z visits (ultimately Z fanout); temp1 is D visits (ultimately D
   fanout). */

    numRows = GarpArrayPtr->numConfigRows;
    numLogicBlocks = numRows * numLogicCols;
    rowConfigPtr = GarpArrayPtr->configPtr;
    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        logicBlockConfigPtr = rowConfigPtr->logicCols;
        for ( colNum = 0; colNum < numLogicCols; ++colNum ) {
            logicBlockConfigPtr->temp0 = 0;
            logicBlockConfigPtr->temp1 = 0;
            ++logicBlockConfigPtr;
        }
        ++rowConfigPtr;
    }
    wirePathCount = 0;
    for ( i = 0; i < maxWirePathLength; ++i ) wirePathLengthCounts[i] = 0;
    singleVWireCount = 0;
    singleHWireCount = 0;
    singleGWireCount = 0;
    for ( i = 0; i < 13; ++i ) {
        for ( j = 0; j < 12; ++j ) connectionVectors[i][j] = 0;
    }
    rowConfigPtr = GarpArrayPtr->configPtr;
    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        logicBlockConfigPtr = rowConfigPtr->logicCols;
        for ( colNum = 0; colNum < numLogicCols; ++colNum ) {
            if (
                ! logicBlockConfigPtr->off
                    && functionIsActive( logicBlockConfigPtr )
            ) {
                updateInputPathLengths(
                    GarpArrayPtr,
                    rowNum,
                    colNum,
                    logicBlockConfigPtr->AIn,
                    logicBlockConfigPtr->AInNum
                );
                in = logicBlockConfigPtr->BIn;
                inNum = logicBlockConfigPtr->BInNum;
                if (
                    (in != logicBlockConfigPtr->AIn)
                        || (inNum != logicBlockConfigPtr->AInNum)
                ) {
                    updateInputPathLengths(
                        GarpArrayPtr, rowNum, colNum, in, inNum );
                }
                in = logicBlockConfigPtr->CIn;
                inNum = logicBlockConfigPtr->CInNum;
                if (
                       ((in != logicBlockConfigPtr->AIn)
                            || (inNum != logicBlockConfigPtr->AInNum))
                    && ((in != logicBlockConfigPtr->BIn)
                            || (inNum != logicBlockConfigPtr->BInNum))
                ) {
                    updateInputPathLengths(
                        GarpArrayPtr, rowNum, colNum, in, inNum );
                }
                if ( logicBlockConfigPtr->mode == MODE_DIRECT ) {
                    table = logicBlockConfigPtr->table;
                    if ( table>>8 != (table & 0xFF) ) {
                        in = logicBlockConfigPtr->DIn;
                        inNum = logicBlockConfigPtr->DInNum;
                        if (
                               ((in != logicBlockConfigPtr->AIn)
                                    || (inNum != logicBlockConfigPtr->AInNum))
                            && ((in != logicBlockConfigPtr->BIn)
                                    || (inNum != logicBlockConfigPtr->BInNum))
                            && ((in != logicBlockConfigPtr->CIn)
                                    || (inNum != logicBlockConfigPtr->CInNum))
                        ) {
                            updateInputPathLengths(
                                GarpArrayPtr, rowNum, colNum, in, inNum );
                        }
                    }
                }
            }
            ++logicBlockConfigPtr;
        }
        ++rowConfigPtr;
    }
/**** RESURRECT THIS? ****/
/****/ #if 0
    /*------------------------------------------------------------------------
    | Count blocks that have no outputs.  (They must be written to memory.)
    *------------------------------------------------------------------------*/
    rowConfigPtr = GarpArrayPtr->configPtr;
    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        logicBlockConfigPtr = rowConfigPtr->logicCols;
        for ( colNum = 0; colNum < numLogicCols; ++colNum ) {
            if ( ! logicBlockConfigPtr->off ) {
                if (
                    ! functionIsActive( logicBlockConfigPtr )
                        && ! logicBlockConfigPtr->temp0
                ) {
                    updateInputPathLengths(
                        GarpArrayPtr,
                        rowNum,
                        colNum,
                        logicBlockConfigPtr->AIn,
                        logicBlockConfigPtr->AInNum
                    );
                    in = logicBlockConfigPtr->BIn;
                    inNum = logicBlockConfigPtr->BInNum;
                    if (
                        (in != logicBlockConfigPtr->AIn)
                            || (inNum != logicBlockConfigPtr->AInNum)
                    ) {
                        updateInputPathLengths(
                            GarpArrayPtr, rowNum, colNum, in, inNum );
                    }
                    in = logicBlockConfigPtr->CIn;
                    inNum = logicBlockConfigPtr->CInNum;
                    if (
                           ((in != logicBlockConfigPtr->AIn)
                                || (inNum != logicBlockConfigPtr->AInNum))
                        && ((in != logicBlockConfigPtr->BIn)
                                || (inNum != logicBlockConfigPtr->BInNum))
                    ) {
                        updateInputPathLengths(
                            GarpArrayPtr, rowNum, colNum, in, inNum );
                    }
                }
                if ( logicBlockConfigPtr->mode == MODE_DIRECT ) {
                    table = logicBlockConfigPtr->table;
                    if ( (table == 0xFF00) && ! logicBlockConfigPtr->temp0 ) {
                        goto DSink;
                    }
                    if ( table>>8 == (table & 0xFF) ) goto maybeDSink;
                } else {
             maybeDSink:
                    if ( ! logicBlockConfigPtr->temp1 ) {
             DSink:
                        updateInputPathLengths(
                            GarpArrayPtr,
                            rowNum,
                            colNum,
                            logicBlockConfigPtr->DIn,
                            logicBlockConfigPtr->DInNum
                        );
                    }
                }
            }
            ++logicBlockConfigPtr;
        }
        ++rowConfigPtr;
    }
/****/ #endif
    fputs(
 "Connection vectors:\n"
 "        23-16  15-8   7-4   3-2    1     0     1    2-3   4-7   8-15  16-23",
        streamPtr
    );
    if ( 24 < numRows ) {
        fputs( "\n 31-24", streamPtr );
        for ( i = 11; i; --i ) {
            fprintf( streamPtr, "%6ld", (long) connectionVectors[0][i] );
        }
    }
    if ( 16 < numRows ) {
        fputs( "\n 23-16", streamPtr );
        for ( i = 11; i; --i ) {
            fprintf( streamPtr, "%6ld", (long) connectionVectors[1][i] );
        }
    }
    if ( 8 < numRows ) {
        fputs( "\n 15-8 ", streamPtr );
        for ( i = 11; i; --i ) {
            fprintf( streamPtr, "%6ld", (long) connectionVectors[2][i] );
        }
    }
    if ( 4 < numRows ) {
        fputs( "\n  7-4 ", streamPtr );
        for ( i = 11; i; --i ) {
            fprintf( streamPtr, "%6ld", (long) connectionVectors[3][i] );
        }
    }
    fputs( "\n  3-2 ", streamPtr );
    for ( i = 11; i; --i ) {
        fprintf( streamPtr, "%6ld", (long) connectionVectors[4][i] );
    }
    fputs( "\n   1  ", streamPtr );
    for ( i = 11; i; --i ) {
        fprintf( streamPtr, "%6ld", (long) connectionVectors[5][i] );
    }
    fputs( "\n   0  ", streamPtr );
    for ( i = 11; i; --i ) {
        if ( i == 6 ) {
            fputs( "      ", streamPtr );
        } else {
            fprintf( streamPtr, "%6ld", (long) connectionVectors[6][i] );
        }
    }
    fputs( "\n   1  ", streamPtr );
    for ( i = 11; i; --i ) {
        fprintf( streamPtr, "%6ld", (long) connectionVectors[7][i] );
    }
    fputs( "\n  2-3 ", streamPtr );
    for ( i = 11; i; --i ) {
        fprintf( streamPtr, "%6ld", (long) connectionVectors[8][i] );
    }
    if ( 4 < numRows ) {
        fputs( "\n  4-7 ", streamPtr );
        for ( i = 11; i; --i ) {
            fprintf( streamPtr, "%6ld", (long) connectionVectors[9][i] );
        }
    }
    if ( 8 < numRows ) {
        fputs( "\n  8-15", streamPtr );
        for ( i = 11; i; --i ) {
            fprintf( streamPtr, "%6ld", (long) connectionVectors[10][i] );
        }
    }
    if ( 16 < numRows ) {
        fputs( "\n 16-23", streamPtr );
        for ( i = 11; i; --i ) {
            fprintf( streamPtr, "%6ld", (long) connectionVectors[11][i] );
        }
    }
    if ( 24 < numRows ) {
        fputs( "\n 24-31", streamPtr );
        for ( i = 11; i; --i ) {
            fprintf( streamPtr, "%6ld", (long) connectionVectors[12][i] );
        }
    }
    fputs( "\n\n", streamPtr );
    fputs(
        "Connection hops:     1     2     3     4 or more\n                ",
        streamPtr
    );
    for ( i = 0; i < maxWirePathLength; ++i ) {
        fprintf( streamPtr, "%6ld", (long) wirePathLengthCounts[i ] );
    }
    fputs( "\n                 ", streamPtr );
    for ( i = 0; i < maxWirePathLength; ++i ) {
        fprintf(
            streamPtr,
            "%5.0f%%",
            100.0 * wirePathLengthCounts[i] / wirePathCount
        );
    }
    fputs( "\n\n", streamPtr );
    singleWireCount = singleVWireCount + singleHWireCount + singleGWireCount;
    fprintf(
        streamPtr,
        "Single-hop wires:    V     H     G\n"
        "                 %5ld %5ld %5ld\n"
        "                  %4.0f%% %4.0f%% %4.0f%%\n\n",
        (long) singleVWireCount,
        (long) singleHWireCount,
        (long) singleGWireCount,
        100.0 * singleVWireCount / singleWireCount,
        100.0 * singleHWireCount / singleWireCount,
        100.0 * singleGWireCount / singleWireCount
    );
    activeDPathCount = 0;
    rowConfigPtr = GarpArrayPtr->configPtr;
    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        logicBlockConfigPtr = rowConfigPtr->logicCols;
        for ( colNum = 0; colNum < numLogicCols; ++colNum ) {
            if ( logicBlockConfigPtr->temp1 ) ++activeDPathCount;
            ++logicBlockConfigPtr;
        }
        ++rowConfigPtr;
    }
    fprintf(
        streamPtr,
        "Active D paths:  %ld  %.0f%%\n\n",
        (long) activeDPathCount,
        100.0 * activeDPathCount / numLogicBlocks
    );

}

