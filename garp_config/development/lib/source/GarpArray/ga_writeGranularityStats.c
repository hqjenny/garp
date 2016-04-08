
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

static bool prevOff;
static int AInBaseColNum, BInBaseColNum, CInBaseColNum, DInBaseColNum;
static logicBlockConfigT commonConfig;

static
 bool extendsCommonConfig( int colNum, logicBlockConfigT *logicBlockConfigPtr )
{
    int in, inNum;

    if ( logicBlockConfigPtr->off ) {
        prevOff = true;
        return false;
    }
    if ( ! colNum || prevOff ) goto newBase;
    in = logicBlockConfigPtr->AIn;
    if ( commonConfig.AIn == IN_CONSTANT ) {
        AInBaseColNum = colNum;
        commonConfig.AIn = in;
        commonConfig.perturbA = logicBlockConfigPtr->perturbA;
    } else if ( in != commonConfig.AIn ) {
        if ( in != IN_CONSTANT ) goto newBase;
    } else {
        inNum = logicBlockConfigPtr->AInNum;
        switch ( in ) {
         case IN_VROW:
            if ( inNum != commonConfig.AInNum ) goto newBase;
            break;
         case IN_ABOVEHCOL:
         case IN_ABOVEGWIRE:
         case IN_HCOL:
         case IN_GWIRE:
            inNum -= commonConfig.AInNum;
            if ( inNum && (inNum != colNum - AInBaseColNum) ) {
                goto newBase;
            }
            break;
        }
        if ( logicBlockConfigPtr->perturbA != commonConfig.perturbA ) {
            goto newBase;
        }
    }
    in = logicBlockConfigPtr->BIn;
    if ( commonConfig.BIn == IN_CONSTANT ) {
        BInBaseColNum = colNum;
        commonConfig.BIn = in;
        commonConfig.perturbB = logicBlockConfigPtr->perturbA;
    } else if ( in != commonConfig.BIn ) {
        if ( in != IN_CONSTANT ) goto newBase;
    } else {
        inNum = logicBlockConfigPtr->BInNum;
        switch ( in ) {
         case IN_VROW:
            if ( inNum != commonConfig.BInNum ) goto newBase;
            break;
         case IN_ABOVEHCOL:
         case IN_ABOVEGWIRE:
         case IN_HCOL:
         case IN_GWIRE:
            inNum -= commonConfig.BInNum;
            if ( inNum && (inNum != colNum - BInBaseColNum) ) {
                goto newBase;
            }
            break;
        }
        if ( logicBlockConfigPtr->perturbB != commonConfig.perturbB ) {
            goto newBase;
        }
    }
    in = logicBlockConfigPtr->CIn;
    if ( commonConfig.CIn == IN_CONSTANT ) {
        CInBaseColNum = colNum;
        commonConfig.CIn = in;
        commonConfig.perturbC = logicBlockConfigPtr->perturbA;
    } else if ( in != commonConfig.CIn ) {
        if ( in != IN_CONSTANT ) goto newBase;
    } else {
        inNum = logicBlockConfigPtr->CInNum;
        switch ( in ) {
         case IN_VROW:
            if ( inNum != commonConfig.CInNum ) goto newBase;
            break;
         case IN_ABOVEHCOL:
         case IN_ABOVEGWIRE:
         case IN_HCOL:
         case IN_GWIRE:
            inNum -= commonConfig.CInNum;
            if ( inNum && (inNum != colNum - CInBaseColNum) ) {
                goto newBase;
            }
            break;
        }
        if ( logicBlockConfigPtr->perturbC != commonConfig.perturbC ) {
            goto newBase;
        }
    }
    in = logicBlockConfigPtr->DIn;
    if ( commonConfig.DIn == IN_CONSTANT ) {
        DInBaseColNum = colNum;
        commonConfig.DIn = in;
        commonConfig.perturbD = logicBlockConfigPtr->perturbA;
    } else if ( in != commonConfig.DIn ) {
        if ( in != IN_CONSTANT ) goto newBase;
    } else {
        inNum = logicBlockConfigPtr->DInNum;
        switch ( in ) {
         case IN_VROW:
            if ( inNum != commonConfig.DInNum ) goto newBase;
            break;
         case IN_ABOVEHCOL:
         case IN_ABOVEGWIRE:
         case IN_HCOL:
         case IN_GWIRE:
            inNum -= commonConfig.DInNum;
            if ( inNum && (inNum != colNum - DInBaseColNum) ) {
                goto newBase;
            }
            break;
        }
        if ( logicBlockConfigPtr->perturbD != commonConfig.perturbD ) {
            goto newBase;
        }
    }
    if (
        (logicBlockConfigPtr->mode != commonConfig.mode)
            || logicBlockConfigPtr->zeroIn
    ) {
        goto newBase;
    }
    switch ( commonConfig.mode ) {
     case MODE_CARRY:
     case MODE_ADD3:
        if ( logicBlockConfigPtr->carryResult != commonConfig.carryResult ) {
            goto newBase;
        }
     case MODE_DIRECT:
     case MODE_INPUT5:
     case MODE_SPLIT:
        if ( logicBlockConfigPtr->table != commonConfig.table ) goto newBase;
        break;
    }
    return true;
 newBase:
    prevOff = false;
    AInBaseColNum = colNum;
    BInBaseColNum = colNum;
    CInBaseColNum = colNum;
    DInBaseColNum = colNum;
    commonConfig = *logicBlockConfigPtr;
    return false;

}

void ga_writeGranularityStats( FILE *streamPtr, GarpArrayT *GarpArrayPtr )
{
    int numRows;
    int_fast32_t numLogicBlocks, activeLogicBlockCount;
    int colNum;
    int_fast32_t granularityCounts[numLogicCols + 1];
    rowConfigT *rowConfigPtr;
    unsigned int rowNum;
    int baseColNum;
    logicBlockConfigT *logicBlockConfigPtr;
    bool savedPrevOff;

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
    for ( colNum = 0; colNum <= numLogicCols; ++colNum ) {
        granularityCounts[colNum] = 0;
    }
    rowConfigPtr = GarpArrayPtr->configPtr;
    for ( rowNum = numRows; rowNum; --rowNum ) {
        logicBlockConfigPtr = rowConfigPtr->logicCols;
        prevOff = true;
        for ( colNum = 0; colNum < numLogicCols; ++colNum ) {
            savedPrevOff = prevOff;
            if ( ! extendsCommonConfig( colNum, logicBlockConfigPtr ) ) {
                if ( ! savedPrevOff ) ++granularityCounts[colNum - baseColNum];
                baseColNum = colNum;
            }
            ++logicBlockConfigPtr;
        }
        if ( ! prevOff ) ++granularityCounts[colNum - baseColNum];
        ++rowConfigPtr;
    }
    fputs(
  "Logic block runs:\n"
  "     1     2     3     4     5     6     7     8     9    10    11    12\n",
        streamPtr
    );
    for ( colNum = 1; colNum <= 12; ++colNum ) {
        fprintf( streamPtr, "%6ld", (long) granularityCounts[colNum] );
    }
    fputc( '\n', streamPtr );
    for ( colNum = 1; colNum <= 12; ++colNum ) {
        fprintf(
            streamPtr, "%6ld", (long) granularityCounts[colNum] * colNum );
    }
    fputc( '\n', streamPtr );
    for ( colNum = 1; colNum <= 12; ++colNum ) {
        fprintf(
            streamPtr,
            "%5.0f%%",
            100.0 * granularityCounts[colNum] * colNum / activeLogicBlockCount
        );
    }
    fputs( "\n\n", streamPtr );
    fputs(
        "    13    14    15    16    17    18    19    20    21    22    23\n",
        streamPtr
    );
    for ( colNum = 13; colNum <= numLogicCols; ++colNum ) {
        fprintf( streamPtr, "%6ld", (long) granularityCounts[colNum] );
    }
    fputc( '\n', streamPtr );
    for ( colNum = 13; colNum <= numLogicCols; ++colNum ) {
        fprintf(
            streamPtr, "%6ld", (long) granularityCounts[colNum] * colNum );
    }
    fputc( '\n', streamPtr );
    for ( colNum = 13; colNum <= numLogicCols; ++colNum ) {
        fprintf(
            streamPtr,
            "%5.0f%%",
            100.0 * granularityCounts[colNum] * colNum / activeLogicBlockCount
        );
    }
    fputs( "\n\n", streamPtr );

}

