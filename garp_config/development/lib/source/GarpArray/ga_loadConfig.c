
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "platform.h"
#include "GarpArrayConfig.h"
#include "GarpArray.h"
#include "allocate.h"
#include "array.h"

int ga_archVersion = GARP_VERSION;

enum {
    numVWires = 32
};

static const int VWireNomLengths[numVWires] = {
      4,    4,    4,    4,    8,    8,   16,   16,
     32,   32,   64,   64,  128,  128,  256,  256,
    512,  512, 1024, 1024, 2048, 2048, 4096, 4096,
   8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192
};

static const int VWireInitNomStartRows[numVWires] = {
      0,   -2,   -3,   -1,   -6,   -2,  -12,   -4,
    -24,   -8,  -48,  -16,  -96,  -32, -192,  -64,
   -384, -128, -768, -256,-1536, -512,-3072,-1024,
      0,    0,    0,    0,    0,    0,    0,    0
};

static int VWireNomStartRows[numVWires];
static int16_t VWireToVRow[numLogicCols][numVWires];

static void stepVWireState( int rowNum )
{
    int VWireStarted, VWireSwapped, wireNum, colNum;
    int_fast32_t i, j;

    if ( rowNum == 0 ) {
        for ( wireNum = 0; wireNum < numVWires; ++wireNum ) {
            VWireNomStartRows[wireNum] = VWireInitNomStartRows[wireNum];
            for ( colNum = 0; colNum < numLogicCols; ++colNum ) {
                VWireToVRow[colNum][wireNum] = -1;
            }
        }
        return;
    }
    if ( rowNum & 1 ) {
        VWireStarted = 2;
        VWireSwapped = 0;
    } else {
        VWireStarted = 0;
        VWireSwapped = 2;
    }
    VWireNomStartRows[VWireStarted] = rowNum;
    i = VWireNomStartRows[VWireSwapped];
    VWireNomStartRows[VWireSwapped] = VWireNomStartRows[VWireSwapped + 1];
    VWireNomStartRows[VWireSwapped + 1] = i;
    for ( colNum = 0; colNum < numLogicCols; ++colNum ) {
        VWireToVRow[colNum][VWireStarted] = -1;
        i = VWireToVRow[colNum][VWireSwapped];
        VWireToVRow[colNum][VWireSwapped] =
            VWireToVRow[colNum][VWireSwapped + 1];
        VWireToVRow[colNum][VWireSwapped + 1] = i;
    }
    if ( ! (rowNum & 1) ) {
        j = rowNum>>1;
        wireNum = 4;
        while ( ! (j & 1) ) {
            i = VWireNomStartRows[wireNum];
            VWireNomStartRows[wireNum] = VWireNomStartRows[wireNum + 1];
            VWireNomStartRows[wireNum + 1] = i;
            for ( colNum = 0; colNum < numLogicCols; ++colNum ) {
                i = VWireToVRow[colNum][wireNum];
                VWireToVRow[colNum][wireNum] =
                    VWireToVRow[colNum][wireNum + 1];
                VWireToVRow[colNum][wireNum + 1] = i;
            }
            j >>= 1;
            wireNum += 2;
        }
        VWireNomStartRows[wireNum] = rowNum;
        for ( colNum = 0; colNum < numLogicCols; ++colNum ) {
            VWireToVRow[colNum][wireNum] = -1;
        }
    }

}

static
 void
  fixVWireIns(
      logicBlockConfigT *logicBlockConfigPtr, int wireNum, int rowNum )
{

    if ( logicBlockConfigPtr->off ) return;
    if (
        (logicBlockConfigPtr->AIn == IN_VWIRE)
            && (logicBlockConfigPtr->AInNum == wireNum)
    ) {
        logicBlockConfigPtr->AIn = IN_VROW;
        logicBlockConfigPtr->AInNum = rowNum;
    }
    if (
        (logicBlockConfigPtr->BIn == IN_VWIRE)
            && (logicBlockConfigPtr->BInNum == wireNum)
    ) {
        logicBlockConfigPtr->BIn = IN_VROW;
        logicBlockConfigPtr->BInNum = rowNum;
    }
    if (
        (logicBlockConfigPtr->CIn == IN_VWIRE)
            && (logicBlockConfigPtr->CInNum == wireNum)
    ) {
        logicBlockConfigPtr->CIn = IN_VROW;
        logicBlockConfigPtr->CInNum = rowNum;
    }
    if (
        (logicBlockConfigPtr->DIn == IN_VWIRE)
            && (logicBlockConfigPtr->DInNum == wireNum)
    ) {
        logicBlockConfigPtr->DIn = IN_VROW;
        logicBlockConfigPtr->DInNum = rowNum;
    }

}

static
 void setVOut( GarpArrayT *GarpArrayPtr, int rowNum, int colNum, int wireNum )
{
    int quarterNomLength, threeQuartersNomLength, nomOffset, inRowNum;

    VWireToVRow[colNum][wireNum] = rowNum;
    quarterNomLength = VWireNomLengths[wireNum]>>2;
    threeQuartersNomLength = quarterNomLength * 3;
    inRowNum = rowNum;
    nomOffset = inRowNum - VWireNomStartRows[wireNum];
    if ( threeQuartersNomLength <= nomOffset ) {
        while ( threeQuartersNomLength <= nomOffset ) {
            fixVWireIns(
                &GarpArrayPtr->configPtr[inRowNum].logicCols[colNum],
                wireNum,
                rowNum
            );
            --inRowNum;
            if ( inRowNum < 0 ) return;
            --nomOffset;
        }
        wireNum ^= 1;
    }
    if ( quarterNomLength <= nomOffset ) {
        while ( quarterNomLength <= nomOffset ) {
            fixVWireIns(
                &GarpArrayPtr->configPtr[inRowNum].logicCols[colNum],
                wireNum,
                rowNum
            );
            --inRowNum;
            if ( inRowNum < 0 ) return;
            --nomOffset;
        }
        wireNum ^= 1;
    }
    while ( 0 <= nomOffset ) {
        fixVWireIns(
            &GarpArrayPtr->configPtr[inRowNum].logicCols[colNum],
            wireNum,
            rowNum
        );
        --inRowNum;
        if ( inRowNum < 0 ) return;
        --nomOffset;
    }

}

static
 void
  installInputConfig(
      GarpArrayT *GarpArrayPtr,
      uint8_t inputConfig,
      int mode,
      int rowNum,
      int colNum,
      uint8_t *inPtr,
      uint16_t *inNumPtr,
      uint8_t *perturbPtr
  )
{
    int perturb, in, inNum, sourceRowNum;

    switch ( mode ) {
     case MODE_DIRECT:
     case MODE_INPUT5:
     case MODE_SPLIT:
     case MODE_CARRY:
        switch ( inputConfig & 3 ) {
         case GARP_IN_XBAR_00:
            perturb = PERTURB_00;
            break;
         case GARP_IN_XBAR_01:
            perturb = PERTURB_01;
            break;
         case GARP_IN_XBAR_10:
            perturb = PERTURB_10;
            break;
         case GARP_IN_XBAR_11:
            perturb = PERTURB_11;
            break;
        }
        break;
     case MODE_SELECT_1:
     case MODE_SELECT:
     case MODE_PARTSELECT:
     case MODE_VARSHIFT:
     case MODE_ADD3:
        if ( inputConfig & GARP_IN_SHIFT1 ) {
            perturb =
                inputConfig & GARP_IN_INVERT ? PERTURB_SHIFT1_NOT
                    : PERTURB_SHIFT1;
        } else {
            perturb = inputConfig & GARP_IN_INVERT ? PERTURB_NOT : PERTURB_10;
        }
        break;
    }
    *perturbPtr = perturb;
    inputConfig &= ~3;
    inNum = 0;
    switch ( inputConfig ) {
     case GARP_IN_00:
        in = IN_CONSTANT;
        break;
     case GARP_IN_10:
        in = IN_CONSTANT;
        inNum = 2;
        break;
     case GARP_IN_ZREG:
        in = IN_ZREG;
        break;
     case GARP_IN_DREG:
        in = IN_DREG;
        break;
     default:
        if ( inputConfig <= GARP_IN_V_0 ) {
            inNum = (GARP_IN_V_0 - inputConfig)>>2;
            sourceRowNum = VWireToVRow[colNum][inNum];
            if ( 0 <= sourceRowNum ) {
                in = IN_VROW;
                inNum = sourceRowNum;
            } else {
                in = IN_VWIRE;
            }
        } else if ( inputConfig <= GARP_IN_ABOVEH_MIN ) {
            in = IN_ABOVEHCOL;
            inNum = colNum + ((GARP_IN_ABOVEH_MIN - inputConfig)>>2) - 5;
            switch ( GarpArrayPtr->configPtr[rowNum - 1].controlCol.HDir ) {
             case HWIRES_LEFT:
                inNum -= 4;
                break;
             case HWIRES_RIGHT:
                inNum += 4;
                break;
            }
            if ( (inNum < 0) || (numLogicCols <= inNum) ) {
                in = IN_CONSTANT;
                inNum = 0;
            }
        } else if ( inputConfig <= GARP_IN_ABOVEG_0 ) {
            in = IN_ABOVEGWIRE;
            inNum = (GARP_IN_ABOVEG_0 - inputConfig)>>2;
        } else if ( inputConfig <= GARP_IN_H_MIN ) {
            in = IN_HCOL;
            inNum = colNum + ((GARP_IN_H_MIN - inputConfig)>>2) - 5;
            switch ( GarpArrayPtr->configPtr[rowNum].controlCol.HDir ) {
             case HWIRES_LEFT:
                inNum -= 4;
                break;
             case HWIRES_RIGHT:
                inNum += 4;
                break;
            }
            if ( (inNum < 0) || (numLogicCols <= inNum) ) {
                in = IN_CONSTANT;
                inNum = 0;
            }
        } else if ( inputConfig <= GARP_IN_G_0 ) {
            in = IN_GWIRE;
            inNum = (GARP_IN_G_0 - inputConfig)>>2;
        }
        break;
    }
    *inPtr = in;
    *inNumPtr = inNum;

}

static
 void
  installControlBlockConfig(
      GarpArrayT *GarpArrayPtr,
      uint64_t config,
      int rowNum,
      controlBlockConfigT *controlBlockConfigPtr
  )
{
    int mode, cacheAllocate, prefetch;

    controlBlockConfigPtr->HDir = (config & 0x00000018)>>3;
    switch ( config & 7 ) {
     case 2:
        mode = MODE_PROCESSOR;
        break;
     case 6:
        switch ( (config>>30) & 3 ) {
         case 0:
            mode = MODE_MEMORY_QUEUE;
            break;
         case 1:
            mode = MODE_MEMORY_DEMAND;
            cacheAllocate = true;
            prefetch = true;
            break;
         case 2:
            mode = MODE_MEMORY_DEMAND;
            cacheAllocate = true;
            prefetch = false;
            break;
         case 3:
            mode = MODE_MEMORY_DEMAND;
            cacheAllocate = false;
            prefetch = false;
            break;
        }
        break;
     default:
        controlBlockConfigPtr->off = true;
        return;
    }
    controlBlockConfigPtr->off = false;
    installInputConfig(
        GarpArrayPtr,
        (config>>GARP_INPUT_A) & 0xFF,
        MODE_DIRECT,
        rowNum,
        numLogicCols,
        &controlBlockConfigPtr->AIn,
        &controlBlockConfigPtr->AInNum,
        &controlBlockConfigPtr->perturbA
    );
    installInputConfig(
        GarpArrayPtr,
        (config>>GARP_INPUT_B) & 0xFF,
        MODE_DIRECT,
        rowNum,
        numLogicCols,
        &controlBlockConfigPtr->BIn,
        &controlBlockConfigPtr->BInNum,
        &controlBlockConfigPtr->perturbB
    );
    installInputConfig(
        GarpArrayPtr,
        (config>>GARP_INPUT_C) & 0xFF,
        MODE_DIRECT,
        rowNum,
        numLogicCols,
        &controlBlockConfigPtr->CIn,
        &controlBlockConfigPtr->CInNum,
        &controlBlockConfigPtr->perturbC
    );
    installInputConfig(
        GarpArrayPtr,
        (config>>GARP_INPUT_D) & 0xFF,
        MODE_DIRECT,
        rowNum,
        numLogicCols,
        &controlBlockConfigPtr->DIn,
        &controlBlockConfigPtr->DInNum,
        &controlBlockConfigPtr->perturbD
    );
    controlBlockConfigPtr->mode = mode;
    switch ( mode ) {
     case MODE_PROCESSOR:
        controlBlockConfigPtr->matchCode = (config & 0x003F0000)>>16;
        break;
     case MODE_MEMORY_QUEUE:
        controlBlockConfigPtr->queueNum = (config & 0x00030000)>>16;
        break;
     case MODE_MEMORY_DEMAND:
        controlBlockConfigPtr->requestSize = (config & 0x00C00000)>>22;
        controlBlockConfigPtr->requestCount = (config & 0x00030000)>>16;
        controlBlockConfigPtr->attributes =
            (cacheAllocate ? ga_allocate : 0) | (prefetch ? ga_prefetch : 0);
        controlBlockConfigPtr->latency = (config & 0x07000000)>>24;
        break;
    }
    controlBlockConfigPtr->dataSize = (config & 0x0000C000)>>14;
    controlBlockConfigPtr->busSelect =
        config & GARP_REGBUS_D ? SELECT_D : SELECT_Z;
    controlBlockConfigPtr->busNum = (config & 0x00000300)>>8;
    controlBlockConfigPtr->GOutNum = 7 - ((config>>GARP_OUTPUT_G) & 7);

}

static
 void
  installLogicBlockConfig(
      GarpArrayT *GarpArrayPtr,
      uint64_t config,
      int rowNum,
      int colNum,
      logicBlockConfigT *logicBlockConfigPtr
  )
{
    int mx, mode, carryResult;
    bool zeroIn;

    if ( config == 0 ) {
        logicBlockConfigPtr->off = true;
        GarpArrayPtr->phase0StatePtr[rowNum].logicCols[colNum].HOut = 0;
        GarpArrayPtr->phase0StatePtr[rowNum].logicCols[colNum].VOut = 0;
        return;
    }
    logicBlockConfigPtr->off = false;
    mx = (config>>32) & 3;
    mode = 0;
    carryResult = 0;
    zeroIn = false;
    switch ( (config>>13) & 7 ) {
     case 0:
        mode = MODE_DIRECT;
        break;
     case 1:
        mode = (mx == 0) ? MODE_INPUT5 : MODE_SPLIT;
        break;
     case 2:
        zeroIn = true;
        goto selectAndShiftModes;
     case 3:
     selectAndShiftModes:
        switch ( mx ) {
         case 0:
            mode = (ga_archVersion < 2) ? MODE_SELECT_1 : MODE_SELECT;
            break;
         case 1:
            mode = MODE_PARTSELECT;
            break;
         case 2:
            if ( ga_archVersion < 2 ) mode = MODE_VARSHIFT;
            break;
        }
        break;
     case 4:
        zeroIn = true;
        mode = MODE_CARRY;
        goto setCarryResult;
     case 5:
        mode = MODE_CARRY;
        goto setCarryResult;
     case 6:
        zeroIn = true;
        mode = MODE_ADD3;
        goto setCarryResult;
     case 7:
        mode = MODE_ADD3;
        goto setCarryResult;
     setCarryResult:
        switch ( mx ) {
         case 0:
            carryResult = RESULT_V;
            break;
         case 1:
            carryResult = (ga_archVersion < 2) ? RESULT_V_OR_K : RESULT_K_OUT;
            break;
         case 2:
            carryResult = RESULT_U_XOR_K;
            break;
         case 3:
            carryResult = RESULT_U_EQU_K;
            break;
        }
        break;
    }
    installInputConfig(
        GarpArrayPtr,
        (config>>GARP_INPUT_A) & 0xFF,
        mode,
        rowNum,
        colNum,
        &logicBlockConfigPtr->AIn,
        &logicBlockConfigPtr->AInNum,
        &logicBlockConfigPtr->perturbA
    );
    installInputConfig(
        GarpArrayPtr,
        (config>>GARP_INPUT_B) & 0xFF,
        mode,
        rowNum,
        colNum,
        &logicBlockConfigPtr->BIn,
        &logicBlockConfigPtr->BInNum,
        &logicBlockConfigPtr->perturbB
    );
    installInputConfig(
        GarpArrayPtr,
        (config>>GARP_INPUT_C) & 0xFF,
        mode,
        rowNum,
        colNum,
        &logicBlockConfigPtr->CIn,
        &logicBlockConfigPtr->CInNum,
        &logicBlockConfigPtr->perturbC
    );
    installInputConfig(
        GarpArrayPtr,
        (config>>GARP_INPUT_D) & 0xFF,
        mode,
        rowNum,
        colNum,
        &logicBlockConfigPtr->DIn,
        &logicBlockConfigPtr->DInNum,
        &logicBlockConfigPtr->perturbD
    );
    logicBlockConfigPtr->mode = mode;
    logicBlockConfigPtr->zeroIn = zeroIn;
    logicBlockConfigPtr->table = (config & 0xFFFF0000)>>16;
    logicBlockConfigPtr->carryResult = carryResult;
    logicBlockConfigPtr->bufferZ = (config & GARP_BUFFERZ) != 0;
    logicBlockConfigPtr->bufferD = (config & GARP_BUFFERD) != 0;
    logicBlockConfigPtr->HSelect = config & GARP_HOUT_D ? SELECT_D : SELECT_Z;
    logicBlockConfigPtr->GSelect = config & GARP_GOUT_D ? SELECT_D : SELECT_Z;
    logicBlockConfigPtr->VSelect = config & GARP_VOUT_D ? SELECT_D : SELECT_Z;
    logicBlockConfigPtr->GOutNum = 7 - ((config>>GARP_OUTPUT_G) & 7);
    setVOut(
        GarpArrayPtr, rowNum, colNum, 31 - ((config>>GARP_OUTPUT_V) & 31) );

}

void
 ga_loadConfig(
     GarpArrayT *GarpArrayPtr,
     int configRowOffset,
     int numConfigRows,
     const uint32_t *inputConfigPtr
 )
{
    int rowNum;
    logicBlockConfigT *logicBlockConfigPtr;
    int colNum;

    if ( GarpArrayPtr->configPtr ) free( GarpArrayPtr->configPtr );
    GarpArrayPtr->configRowOffset = configRowOffset;
    GarpArrayPtr->numConfigRows = numConfigRows;
    GarpArrayPtr->configPtr = allocArrayType( rowConfigT, numConfigRows );
    for ( rowNum = 0; rowNum < numConfigRows; ++rowNum ) {
        stepVWireState( rowNum );
        installControlBlockConfig(
            GarpArrayPtr,
            (((uint64_t) *inputConfigPtr)<<32) | inputConfigPtr[1],
            rowNum,
            &GarpArrayPtr->configPtr[rowNum].controlCol
        );
        inputConfigPtr += 2;
        logicBlockConfigPtr =
            &GarpArrayPtr->configPtr[rowNum].logicCols[numLogicCols - 1];
        for ( colNum = numLogicCols - 1; 0 <= colNum; --colNum ) {
            installLogicBlockConfig(
                GarpArrayPtr,
                (((uint64_t) *inputConfigPtr)<<32) | inputConfigPtr[1],
                rowNum,
                colNum,
                logicBlockConfigPtr
            );
            inputConfigPtr += 2;
            --logicBlockConfigPtr;
        }
    }

}

