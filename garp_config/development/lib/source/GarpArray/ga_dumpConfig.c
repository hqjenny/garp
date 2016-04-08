
#include <stdio.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

#define ITEMNAMECASE( caseName ) case caseName: itemName = #caseName; break;

static
 void
  writeInputConfig( FILE *streamPtr, int in, unsigned int inNum, int perturb )
{
    char *inName, *itemName;

    itemName = "??";
    switch ( in ) {
     ITEMNAMECASE( IN_CONSTANT )
     ITEMNAMECASE( IN_ZREG )
     ITEMNAMECASE( IN_DREG )
     ITEMNAMECASE( IN_VROW )
     ITEMNAMECASE( IN_ABOVEHCOL )
     ITEMNAMECASE( IN_ABOVEGWIRE )
     ITEMNAMECASE( IN_HCOL )
     ITEMNAMECASE( IN_GWIRE )
    }
    inName = itemName;
    itemName = "??";
    switch ( perturb ) {
     ITEMNAMECASE( PERTURB_00 )
     ITEMNAMECASE( PERTURB_01 )
     ITEMNAMECASE( PERTURB_10 )
     ITEMNAMECASE( PERTURB_11 )
     ITEMNAMECASE( PERTURB_NOT )
     ITEMNAMECASE( PERTURB_SHIFT1 )
     ITEMNAMECASE( PERTURB_SHIFT1_NOT )
    }
    fprintf( streamPtr, "%s %u %s\n", inName, inNum, itemName );

}

static
 void
  writeControlBlockConfig(
      FILE *streamPtr, controlBlockConfigT *controlBlockConfigPtr )
{
    char *itemName;

    if ( controlBlockConfigPtr->off ) goto writeHDir;
    fputs( "    A: ", streamPtr );
    writeInputConfig(
        streamPtr,
        controlBlockConfigPtr->AIn,
        controlBlockConfigPtr->AInNum,
        controlBlockConfigPtr->perturbA
    );
    fputs( "    B: ", streamPtr );
    writeInputConfig(
        streamPtr,
        controlBlockConfigPtr->BIn,
        controlBlockConfigPtr->BInNum,
        controlBlockConfigPtr->perturbB
    );
    fputs( "    C: ", streamPtr );
    writeInputConfig(
        streamPtr,
        controlBlockConfigPtr->CIn,
        controlBlockConfigPtr->CInNum,
        controlBlockConfigPtr->perturbC
    );
    fputs( "    D: ", streamPtr );
    writeInputConfig(
        streamPtr,
        controlBlockConfigPtr->DIn,
        controlBlockConfigPtr->DInNum,
        controlBlockConfigPtr->perturbD
    );
    fputs( "    mode: ", streamPtr );
    switch ( controlBlockConfigPtr->mode ) {
     case MODE_PROCESSOR:
        fprintf(
            streamPtr,
            "MODE_PROCESSOR\n    matchCode: %d\n",
            controlBlockConfigPtr->matchCode
        );
        break;
     case MODE_MEMORY_QUEUE:
        fprintf(
            streamPtr,
            "MODE_MEMORY_QUEUE\n    queueNum: %d\n",
            controlBlockConfigPtr->queueNum
        );
        break;
     case MODE_MEMORY_DEMAND:
        fprintf(
            streamPtr,
        "MODE_MEMORY_DEMAND\n    request: %d*%d attributes: %1X latency: %d\n",
            1<<(controlBlockConfigPtr->requestSize + 3),
            1<<controlBlockConfigPtr->requestCount,
            controlBlockConfigPtr->attributes,
            controlBlockConfigPtr->latency
        );
        break;
     default:
        fputs( "??\n", streamPtr );
        goto noRegbus;
    }
    fprintf(
        streamPtr,
        "    data: %d bus: %s %d\n",
        1<<(controlBlockConfigPtr->dataSize + 3),
        controlBlockConfigPtr->busSelect ? "SELECT_D" : "SELECT_Z",
        controlBlockConfigPtr->busNum
    );
 noRegbus:
    fprintf( streamPtr, "    Gout: %d\n", controlBlockConfigPtr->GOutNum );
 writeHDir:
    itemName = "??";
    switch ( controlBlockConfigPtr->HDir ) {
     ITEMNAMECASE( HWIRES_LEFT )
     ITEMNAMECASE( HWIRES_CENTER )
     ITEMNAMECASE( HWIRES_RIGHT )
    }
    fprintf( streamPtr, "    Hdir: %s\n", itemName );

}

static
 void
  writeLogicBlockConfig(
      FILE *streamPtr, logicBlockConfigT *logicBlockConfigPtr )
{
    char *itemName;

    fputs( "    A: ", streamPtr );
    writeInputConfig(
        streamPtr,
        logicBlockConfigPtr->AIn,
        logicBlockConfigPtr->AInNum,
        logicBlockConfigPtr->perturbA
    );
    fputs( "    B: ", streamPtr );
    writeInputConfig(
        streamPtr,
        logicBlockConfigPtr->BIn,
        logicBlockConfigPtr->BInNum,
        logicBlockConfigPtr->perturbB
    );
    fputs( "    C: ", streamPtr );
    writeInputConfig(
        streamPtr,
        logicBlockConfigPtr->CIn,
        logicBlockConfigPtr->CInNum,
        logicBlockConfigPtr->perturbC
    );
    fputs( "    D: ", streamPtr );
    writeInputConfig(
        streamPtr,
        logicBlockConfigPtr->DIn,
        logicBlockConfigPtr->DInNum,
        logicBlockConfigPtr->perturbD
    );
    itemName = "??";
    switch ( logicBlockConfigPtr->mode ) {
     ITEMNAMECASE( MODE_DIRECT )
     ITEMNAMECASE( MODE_INPUT5 )
     ITEMNAMECASE( MODE_SPLIT )
     ITEMNAMECASE( MODE_SELECT_1 )
     ITEMNAMECASE( MODE_SELECT )
     ITEMNAMECASE( MODE_PARTSELECT )
     ITEMNAMECASE( MODE_VARSHIFT )
     ITEMNAMECASE( MODE_CARRY )
     ITEMNAMECASE( MODE_ADD3 )
    }
    fprintf(
        streamPtr,
        "    function: %s%s %04X",
        itemName,
        logicBlockConfigPtr->zeroIn ? ":0" : "",
        logicBlockConfigPtr->table
    );
    switch ( logicBlockConfigPtr->mode ) {
     case MODE_CARRY:
     case MODE_ADD3:
        itemName = "??";
        switch ( logicBlockConfigPtr->carryResult ) {
         ITEMNAMECASE( RESULT_V )
         ITEMNAMECASE( RESULT_V_OR_K )
         ITEMNAMECASE( RESULT_K_OUT )
         ITEMNAMECASE( RESULT_U_XOR_K )
         ITEMNAMECASE( RESULT_U_EQU_K )
        }
        fprintf( streamPtr, " %s", itemName );
        break;
    }
    fprintf(
        streamPtr,
        "\n"
        "    bufferZ: %d bufferD: %d\n"
        "    HOut: %s\n"
        "    GOut: %s %d\n"
        "    VOut: %s\n",
        logicBlockConfigPtr->bufferZ,
        logicBlockConfigPtr->bufferD,
        logicBlockConfigPtr->HSelect ? "SELECT_D" : "SELECT_Z",
        logicBlockConfigPtr->GSelect ? "SELECT_D" : "SELECT_Z",
        logicBlockConfigPtr->GOutNum,
        logicBlockConfigPtr->VSelect ? "SELECT_D" : "SELECT_Z"
    );

}

void ga_dumpConfig( FILE *streamPtr, GarpArrayT *GarpArrayPtr )
{
    unsigned int rowNum;
    int colNum;
    rowConfigT *rowConfigPtr;
    logicBlockConfigT *logicBlockConfigPtr;

    rowConfigPtr = GarpArrayPtr->configPtr;
    for ( rowNum = 0; rowNum < GarpArrayPtr->numConfigRows; ++rowNum ) {
        fprintf( streamPtr, "row %u:\n", rowNum );
        fputs( "  control:\n", streamPtr );
        writeControlBlockConfig( streamPtr, &rowConfigPtr->controlCol );
        fputc( '\n', streamPtr );
        logicBlockConfigPtr = rowConfigPtr->logicCols;
        for ( colNum = 0; colNum < numLogicCols; ++colNum ) {
            if ( ! logicBlockConfigPtr->off ) {
                fprintf( streamPtr, "  col %d:\n", colNum );
                writeLogicBlockConfig( streamPtr, logicBlockConfigPtr );
                fputc( '\n', streamPtr );
            }
            ++logicBlockConfigPtr;
        }
        ++rowConfigPtr;
    }

}

