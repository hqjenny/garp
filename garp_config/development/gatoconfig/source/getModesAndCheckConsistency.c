
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "errorMessages.h"
#include "tokens.h"
#include "tokenize.h"
#include "logicBlocks.h"
#include "wires.h"
#include "config.h"
#include "getModesAndCheckConsistency.h"

enum {
    ITEM_A_INPUT = 1,
    ITEM_A_XBAR,
    ITEM_A_SHIFT1,
    ITEM_A_INVERT,
    ITEM_B_INPUT,
    ITEM_B_XBAR,
    ITEM_B_SHIFT1,
    ITEM_B_INVERT,
    ITEM_C_INPUT,
    ITEM_C_XBAR,
    ITEM_C_SHIFT1,
    ITEM_C_INVERT,
    ITEM_D_INPUT,
    ITEM_D_XBAR,
    ITEM_D_SHIFT1,
    ITEM_D_INVERT,
    ITEM_FUNCTION_D,
    ITEM_FUNCTION_ABOVEH,
    ITEM_FUNCTION,
    ITEM_FUNCTIONHI,
    ITEM_FUNCTIONLO,
    ITEM_TABLE,
    ITEM_SELECT,
    ITEM_PARTIALSELECT,
    ITEM_VARIABLESHIFT,
    ITEM_ADD3,
    ITEM_U,
    ITEM_V,
    ITEM_RESULT,
    ITEM_SHIFTZEROIN,
    ITEM_BUFFERZ,
    ITEM_BUFFERD,
    ITEM_HOUT_ZD,
    ITEM_GOUT_ZD,
    ITEM_GOUT_WIRE,
    ITEM_VOUT_ZD,
    ITEM_VOUT_LONG,
    ITEM_PROCESSOR,
    ITEM_MATCH,
    ITEM_MEMORY,
    ITEM_QUEUE,
    ITEM_DEMAND,
    ITEM_DEMAND_ARG,
    ITEM_ALLOCATE,
    ITEM_ALIGNED,
    ITEM_LATENCY,
    ITEM_REGBUS,
    ITEM_HDIR,
    NUMITEMS
};

static int items[NUMITEMS];

static inline void addItemToCatalog( int itemCode )
{
    int itemCount;

    itemCount = items[itemCode];
    if ( itemCount < 2 ) items[itemCode] = itemCount + 1;

}

static
 void
  catalogInputItems(
      tokenT *tree, uint_fast16_t rowNum, int colNum, int itemBaseCode )
{
    int itemCode, m;
    rowConfigT *rowConfigPtr;

    if ( ! tree ) return;
    itemCode = 0;
    m = tree->v.m;
    switch ( tree->code ) {
     case OP_LO:
     case OP_HI:
     case OP_SWAP:
     case OP_EITHER:
        itemCode = itemBaseCode + (ITEM_A_XBAR - ITEM_A_INPUT);
        break;
     case OP_SHIFT1:
        itemCode = itemBaseCode + (ITEM_A_SHIFT1 - ITEM_A_INPUT);
        break;
     case OP_NOT:
        itemCode = itemBaseCode + (ITEM_A_INVERT - ITEM_A_INPUT);
        break;
     case OP_ABOVEHORIZ_ABS:
        m -= colNum;
        goto aboveHorizRel;
     case OP_ABOVEHORIZ_REL:
     aboveHorizRel:
        if ( rowNum ) {
            rowConfigPtr = rowConfigPtrs[rowNum - 1];
            if ( m < rowConfigPtr->horizMin ) {
                rowConfigPtr->horizMin = m;
            } else if ( rowConfigPtr->horizMax < m ) {
                rowConfigPtr->horizMax = m;
            }
        }
        break;
     case OP_HORIZ_ABS:
        m -= colNum;
        goto horizRel;
     case OP_HORIZ_REL:
     horizRel:
        rowConfigPtr = rowConfigPtrs[rowNum];
        if ( m < rowConfigPtr->horizMin ) {
            rowConfigPtr->horizMin = m;
        } else if ( rowConfigPtr->horizMax < m ) {
            rowConfigPtr->horizMax = m;
        }
        break;
     default:
        itemCode = itemBaseCode;
        break;
    }
    if ( itemCode ) addItemToCatalog( itemCode );
    catalogInputItems( tree->t, rowNum, colNum, itemBaseCode );

}

static void catalogFunctionItems( tokenT *tree )
{

    if ( ! tree ) return;
    switch ( tree->code ) {
     case OP_D:
        items[ITEM_FUNCTION_D] = 1;
        break;
     case OP_ABOVEH:
        items[ITEM_FUNCTION_ABOVEH] = 1;
        break;
     default:
        catalogFunctionItems( tree->a );
        catalogFunctionItems( tree->b );
        break;
    }

}

static void catalogDemandItems( tokenT *tree )
{
    int itemCode;

    if ( ! tree ) return;
    itemCode = 0;
    switch ( tree->code ) {
     case OP_TIMES:
        itemCode = ITEM_DEMAND_ARG;
        break;
     case OP_ALLOCATE:
        itemCode = ITEM_ALLOCATE;
        break;
     case OP_ALIGNED:
        itemCode = ITEM_ALIGNED;
        break;
    }
    if ( itemCode ) addItemToCatalog( itemCode );
    catalogDemandItems( tree->t );

}

static void catalogGOutItems( tokenT *tree )
{
    int itemCode;

    if ( ! tree ) return;
    itemCode = 0;
    switch ( tree->code ) {
     case OP_D:
     case OP_Z:
        itemCode = ITEM_GOUT_ZD;
        break;
     case OP_GLOBAL:
        itemCode = ITEM_GOUT_WIRE;
        break;
    }
    if ( itemCode ) addItemToCatalog( itemCode );
    catalogGOutItems( tree->t );

}

static void catalogVOutItems( tokenT *tree )
{
    int itemCode;

    if ( ! tree ) return;
    itemCode = 0;
    switch ( tree->code ) {
     case OP_D:
     case OP_Z:
        itemCode = ITEM_VOUT_ZD;
        break;
     case OP_LONG:
        itemCode = ITEM_VOUT_LONG;
        break;
    }
    if ( itemCode ) addItemToCatalog( itemCode );
    catalogVOutItems( tree->t );

}

static void catalogControlItems( tokenT *tree, uint_fast16_t rowNum )
{
    int itemCode;

    if ( ! tree ) return;
    itemCode = 0;
    switch ( tree->code ) {
     case OP_CONTROLBLOCK:
        catalogControlItems( tree->b, rowNum );
        break;
     case OP_A:
        catalogInputItems( tree->a, rowNum, NUMLOGICCOLS, ITEM_A_INPUT );
        break;
     case OP_B:
        catalogInputItems( tree->a, rowNum, NUMLOGICCOLS, ITEM_B_INPUT );
        break;
     case OP_C:
        catalogInputItems( tree->a, rowNum, NUMLOGICCOLS, ITEM_C_INPUT );
        break;
     case OP_D:
        catalogInputItems( tree->a, rowNum, NUMLOGICCOLS, ITEM_D_INPUT );
        break;
     case OP_PROCESSOR:
        itemCode = ITEM_PROCESSOR;
        break;
     case OP_MATCH:
        itemCode = ITEM_MATCH;
        break;
     case OP_MEMORY:
        itemCode = ITEM_MEMORY;
        break;
     case OP_QUEUE:
        itemCode = ITEM_QUEUE;
        break;
     case OP_DEMAND:
     case OP_DEMANDREAD:
        catalogDemandItems( tree->a );
        break;
     case OP_LATENCY:
        itemCode = ITEM_LATENCY;
        break;
     case OP_REGBUS:
        itemCode = ITEM_REGBUS;
        break;
     case OP_GOUT:
        catalogGOutItems( tree->a );
        break;
     case OP_HDIR:
        if ( tree->v.m < 0 ) {
            rowConfigPtrs[rowNum]->HWiresDirection = HWIRES_RIGHT;
        } else if ( tree->v.m > 0 ) {
            rowConfigPtrs[rowNum]->HWiresDirection = HWIRES_LEFT;
        } else {
            rowConfigPtrs[rowNum]->HWiresDirection = HWIRES_CENTER;
        }
        itemCode = ITEM_HDIR;
        break;
    }
    if ( itemCode ) addItemToCatalog( itemCode );
    catalogControlItems( tree->t, rowNum );

}

static
 void catalogLogicBlockItems( tokenT *tree, uint_fast16_t rowNum, int colNum )
{
    int itemCode;

    if ( ! tree ) return;
    itemCode = 0;
    switch ( tree->code ) {
     case OP_A:
        catalogInputItems( tree->a, rowNum, colNum, ITEM_A_INPUT );
        break;
     case OP_B:
        catalogInputItems( tree->a, rowNum, colNum, ITEM_B_INPUT );
        break;
     case OP_C:
        catalogInputItems( tree->a, rowNum, colNum, ITEM_C_INPUT );
        break;
     case OP_D:
        catalogInputItems( tree->a, rowNum, colNum, ITEM_D_INPUT );
        break;
     case OP_FUNCTION:
        catalogFunctionItems( tree->a );
        itemCode = ITEM_FUNCTION;
        break;
     case OP_FUNCTIONLO:
        itemCode = ITEM_FUNCTIONLO;
        break;
     case OP_FUNCTIONHI:
        itemCode = ITEM_FUNCTIONHI;
        break;
     case OP_TABLE:
        itemCode = ITEM_TABLE;
        break;
     case OP_SELECT:
        itemCode = ITEM_SELECT;
        break;
     case OP_PARTIALSELECT:
        itemCode = ITEM_PARTIALSELECT;
        break;
     case OP_VARIABLESHIFT:
        itemCode = ITEM_VARIABLESHIFT;
        break;
     case OP_ADD3:
        itemCode = ITEM_ADD3;
        break;
     case OP_U:
        itemCode = ITEM_U;
        break;
     case OP_V:
        itemCode = ITEM_V;
        break;
     case OP_RESULT:
        itemCode = ITEM_RESULT;
        break;
     case OP_SHIFTZEROIN:
        itemCode = ITEM_SHIFTZEROIN;
        break;
     case OP_BUFFERZ:
        itemCode = ITEM_BUFFERZ;
        break;
     case OP_BUFFERD:
        itemCode = ITEM_BUFFERD;
        break;
     case OP_HOUT:
        if ( tree->a && (tree->a->code != OP_VOID) ) itemCode = ITEM_HOUT_ZD;
        break;
     case OP_GOUT:
        catalogGOutItems( tree->a );
        break;
     case OP_VOUT:
        catalogVOutItems( tree->a );
        break;
    }
    if ( itemCode ) addItemToCatalog( itemCode );
    catalogLogicBlockItems( tree->t, rowNum, colNum );

}

static void catalogColItems( tokenT *tree, uint_fast16_t rowNum, int colNum )
{

    if ( ! tree ) return;
    switch ( tree->code ) {
     case OP_LOGICBLOCK:
        if ( logicBlockCoversCol( tree->a, colNum ) ) {
            catalogLogicBlockItems( tree->b, rowNum, colNum );
        }
        break;
    }
    catalogColItems( tree->t, rowNum, colNum );

}

static char redundantItemErrorMessage[] = "Redundant item.";

static
 void
  markRedundantControlInputItems(
      tokenT *tree, int itemBaseCode, tokenT *errorToken )
{
    int itemCode;

    if ( ! tree ) return;
    itemCode = 0;
    switch ( tree->code ) {
     case OP_LO:
     case OP_HI:
     case OP_EITHER:
        itemCode = itemBaseCode + (ITEM_A_XBAR - ITEM_A_INPUT);
        break;
     default:
        itemCode = itemBaseCode;
        break;
    }
    if ( itemCode && (1 < items[itemCode]) ) {
        markErrorAtToken( errorToken, redundantItemErrorMessage );
    }
    markRedundantControlInputItems( tree->t, itemBaseCode, errorToken );

}

static void markRedundantDemandItems( tokenT *tree, tokenT *errorToken )
{
    int itemCode;

    if ( ! tree ) return;
    itemCode = 0;
    switch ( tree->code ) {
     case OP_TIMES:
        itemCode = ITEM_DEMAND_ARG;
        break;
     case OP_ALLOCATE:
        itemCode = ITEM_ALLOCATE;
        break;
     case OP_ALIGNED:
        itemCode = ITEM_ALIGNED;
        break;
    }
    if ( itemCode && (1 < items[itemCode]) ) {
        markErrorAtToken( errorToken, redundantItemErrorMessage );
    }
    markRedundantDemandItems( tree->t, errorToken );

}

static void markRedundantControlItems( tokenT *tree )
{
    int itemCode;

    if ( ! tree ) return;
    if ( items[ITEM_PROCESSOR] && ! items[ITEM_MEMORY] ) {
        switch( tree->code ) {
         case OP_QUEUE:
         case OP_DEMAND:
         case OP_DEMANDREAD:
         case OP_LATENCY:
            markErrorAtToken( tree, "Item inconsistent with control mode." );
            break;
        }
    } else if ( ! items[ITEM_PROCESSOR] && items[ITEM_MEMORY] ) {
        switch( tree->code ) {
         case OP_MATCH:
            markErrorAtToken( tree, "Item inconsistent with control mode." );
            break;
        }
    }
    itemCode = 0;
    switch ( tree->code ) {
     case OP_CONTROLBLOCK:
        markRedundantControlItems( tree->b );
        break;
     case OP_A:
        markRedundantControlInputItems( tree->a, ITEM_A_INPUT, tree );
        break;
     case OP_B:
        markRedundantControlInputItems( tree->a, ITEM_B_INPUT, tree );
        break;
     case OP_C:
        markRedundantControlInputItems( tree->a, ITEM_C_INPUT, tree );
        break;
     case OP_D:
        markRedundantControlInputItems( tree->a, ITEM_D_INPUT, tree );
        break;
     case OP_PROCESSOR:
        if ( items[ITEM_MEMORY] ) {
            markErrorAtToken( tree, "Contradictory control mode." );
        }
        itemCode = ITEM_PROCESSOR;
        break;
     case OP_MATCH:
        itemCode = ITEM_MATCH;
        break;
     case OP_MEMORY:
        if ( items[ITEM_PROCESSOR] ) {
            markErrorAtToken( tree, "Contradictory control mode." );
        }
        itemCode = ITEM_MEMORY;
        break;
     case OP_QUEUE:
        itemCode = ITEM_QUEUE;
        break;
     case OP_DEMAND:
     case OP_DEMANDREAD:
        markRedundantDemandItems( tree->a, tree );
        break;
     case OP_LATENCY:
        itemCode = ITEM_LATENCY;
        break;
     case OP_REGBUS:
        itemCode = ITEM_REGBUS;
        break;
     case OP_GOUT:
        itemCode = ITEM_GOUT_WIRE;
        break;
     case OP_HDIR:
        itemCode = ITEM_HDIR;
        break;
    }
    if ( itemCode && (1 < items[itemCode]) ) {
        markErrorAtToken( tree, redundantItemErrorMessage );
    }
    markRedundantControlItems( tree->t );

}

static void markRedundantItemError( tokenT *errorToken, int colNum )
{

    markErrorInColumnAtToken( errorToken, "Redundant item", colNum );

}

static
 void
  markRedundantInputItems(
      tokenT *tree, int colNum, int itemBaseCode, int mode, tokenT *errorToken
  )
{
    static const char modeNotKnownMessage[] = "Function mode not known";
    static const char improperModeMessage[] =
        "Argument inconsistent with function mode";
    const char *errorMessage;
    int itemCode;

    if ( ! tree ) return;
    errorMessage = 0;
    switch ( tree->code ) {
     case L_BINARY:
        if (
            tree->v.m && items[itemBaseCode + (ITEM_A_SHIFT1 - ITEM_A_INPUT)]
        ) {
            errorMessage = "Nonzero constant not allowed with shift1";
        }
        break;
     case OP_LO:
     case OP_HI:
     case OP_SWAP:
        switch ( mode ) {
         case 0:
            errorMessage = modeNotKnownMessage;
            break;
         case -1:
         case MODE_DIRECT:
            break;
         case MODE_INPUT5:
         case MODE_SPLIT:
         case MODE_CARRY:
            if ( itemBaseCode == ITEM_D_INPUT ) {
                errorMessage = improperModeMessage;
            }
            break;
         default:
            errorMessage = improperModeMessage;
            break;
        }
        break;
     case OP_SHIFT1:
     case OP_NOT:
        switch ( mode ) {
         case 0:
            errorMessage = modeNotKnownMessage;
            break;
         case -1:
         case MODE_SELECT:
         case MODE_PPSELECT:
         case MODE_VARSHIFT:
         case MODE_ADD3:
            break;
         default:
            errorMessage = improperModeMessage;
            break;
        }
        break;
    }
    if ( errorMessage ) markErrorInColumnAtToken( tree, errorMessage, colNum );
    itemCode = 0;
    switch ( tree->code ) {
     case OP_LO:
     case OP_HI:
     case OP_SWAP:
        itemCode = itemBaseCode + (ITEM_A_XBAR - ITEM_A_INPUT);
        break;
     case OP_SHIFT1:
        itemCode = itemBaseCode + (ITEM_A_SHIFT1 - ITEM_A_INPUT);
        break;
     case OP_NOT:
        itemCode = itemBaseCode + (ITEM_A_INVERT - ITEM_A_INPUT);
        break;
     default:
        itemCode = itemBaseCode;
        break;
    }
    if ( itemCode && (1 < items[itemCode]) ) {
        markRedundantItemError( errorToken, colNum );
    }
    markRedundantInputItems( tree->t, colNum, itemBaseCode, mode, errorToken );

}

static void markRedundantFunctionItems( tokenT *tree, int colNum )
{
    int itemCode;

    if ( ! tree ) return;
    itemCode = 0;
    switch ( tree->code ) {
     case OP_D:
        itemCode = ITEM_FUNCTION_ABOVEH;
        break;
     case OP_ABOVEH:
        itemCode = ITEM_FUNCTION_D;
        break;
     default:
        markRedundantFunctionItems( tree->a, colNum );
        markRedundantFunctionItems( tree->b, colNum );
        break;
    }
    if ( itemCode && items[itemCode] ) {
        markErrorInColumnAtToken( tree, "Contradictory argument", colNum );
    }

}

static void markRedundantUVItems( tokenT *tree, int colNum, int mode )
{
    bool error;

    if ( ! tree ) return;
    error = false;
    switch ( tree->code ) {
     case OP_A:
     case OP_B:
     case OP_C:
        error = mode != MODE_CARRY;
        break;
     case OP_SUM:
     case OP_CARRY:
        error = mode != MODE_ADD3;
        break;
     default:
        markRedundantUVItems( tree->a, colNum, mode );
        markRedundantUVItems( tree->b, colNum, mode );
        break;
    }
    if ( error ) {
        markErrorInColumnAtToken(
            tree,
            "Expression component inconsistent with function mode",
            colNum
        );
    }

}

static
 void
  markRedundantOutputItems(
      tokenT *tree, int colNum, int mode, int ZDItemCode, tokenT *errorToken )
{
    int itemCode;

    if ( ! tree ) return;
    itemCode = 0;
    switch ( tree->code ) {
     case OP_Z:
        itemCode = ZDItemCode;
        break;
     case OP_D:
        itemCode = ZDItemCode;
        break;
     case OP_GLOBAL:
        itemCode = ITEM_GOUT_WIRE;
        break;
     case OP_LONG:
        itemCode = ITEM_VOUT_LONG;
        break;
    }
    if ( itemCode && (1 < items[itemCode]) ) {
        markRedundantItemError( errorToken, colNum );
    }
    markRedundantOutputItems( tree->t, colNum, mode, ZDItemCode, errorToken );

}

static void markRedundantLogicBlockItems( tokenT *tree, int colNum, int mode )
{
    int itemCode;

    if ( ! tree ) return;
    switch ( tree->code ) {
     case OP_FUNCTION:
     case OP_FUNCTIONLO:
     case OP_FUNCTIONHI:
     case OP_TABLE:
     case OP_SELECT:
     case OP_PARTIALSELECT:
     case OP_VARIABLESHIFT:
     case OP_ADD3:
     case OP_U:
     case OP_V:
     case OP_RESULT:
        if ( mode < 0 ) {
            markErrorInColumnAtToken(
                tree, "Contradictory function mode", colNum );
        }
    }
    itemCode = 0;
    switch ( tree->code ) {
     case OP_A:
        markRedundantInputItems( tree->a, colNum, ITEM_A_INPUT, mode, tree );
        break;
     case OP_B:
        markRedundantInputItems( tree->a, colNum, ITEM_B_INPUT, mode, tree );
        break;
     case OP_C:
        markRedundantInputItems( tree->a, colNum, ITEM_C_INPUT, mode, tree );
        break;
     case OP_D:
        markRedundantInputItems( tree->a, colNum, ITEM_D_INPUT, mode, tree );
        break;
     case OP_FUNCTION:
        markRedundantFunctionItems( tree->a, colNum );
        itemCode = ITEM_FUNCTION;
        break;
     case OP_FUNCTIONLO:
        itemCode = ITEM_FUNCTIONLO;
        break;
     case OP_FUNCTIONHI:
        itemCode = ITEM_FUNCTIONHI;
        break;
     case OP_TABLE:
        itemCode = ITEM_TABLE;
        break;
     case OP_SELECT:
        itemCode = ITEM_SELECT;
        break;
     case OP_PARTIALSELECT:
        itemCode = ITEM_PARTIALSELECT;
        break;
     case OP_VARIABLESHIFT:
        itemCode = ITEM_VARIABLESHIFT;
        break;
     case OP_ADD3:
        itemCode = ITEM_ADD3;
        break;
     case OP_U:
        markRedundantUVItems( tree->a, colNum, mode );
        itemCode = ITEM_U;
        break;
     case OP_V:
        markRedundantUVItems( tree->a, colNum, mode );
        itemCode = ITEM_V;
        break;
     case OP_RESULT:
        itemCode = ITEM_RESULT;
        break;
     case OP_SHIFTZEROIN:
        itemCode = ITEM_SHIFTZEROIN;
        break;
     case OP_BUFFERZ:
        itemCode = ITEM_BUFFERZ;
        if ( mode == 0 ) {
            markErrorInColumnAtToken(
                tree, "Function for Z not known", colNum );
        }
        break;
     case OP_BUFFERD:
        itemCode = ITEM_BUFFERD;
        break;
     case OP_HOUT:
        markRedundantOutputItems( tree->a, colNum, mode, ITEM_HOUT_ZD, tree );
        break;
     case OP_GOUT:
        markRedundantOutputItems( tree->a, colNum, mode, ITEM_GOUT_ZD, tree );
        break;
     case OP_VOUT:
        markRedundantOutputItems( tree->a, colNum, mode, ITEM_VOUT_ZD, tree );
        break;
    }
    if ( itemCode && (1 < items[itemCode]) ) {
        markRedundantItemError( tree, colNum );
    }
    markRedundantLogicBlockItems( tree->t, colNum, mode );

}

static void markRedundantColItems( tokenT *tree, int colNum, int mode )
{

    if ( ! tree ) return;
    switch ( tree->code ) {
     case OP_LOGICBLOCK:
        if ( logicBlockCoversCol( tree->a, colNum ) ) {
            markRedundantLogicBlockItems( tree->b, colNum, mode );
        }
        break;
    }
    markRedundantColItems( tree->t, colNum, mode );

}

static
 void getModesAndCheckConsistencyOnRow( tokenT *tree, uint_fast16_t rowNum )
{
    rowConfigT *rowConfigPtr;
    int colNum, mode, i;

    for ( i = 0; i < NUMITEMS; ++i ) items[i] = 0;
    catalogControlItems( tree, rowNum );
    markRedundantControlItems( tree );
    rowConfigPtr = rowConfigPtrs[rowNum];
    for ( colNum = 0; colNum < NUMLOGICCOLS; ++colNum ) {
        for ( i = 0; i < NUMITEMS; ++i ) items[i] = 0;
        catalogColItems( tree, rowNum, colNum );
        mode = 0;
        if ( items[ITEM_FUNCTION] ) {
            mode = items[ITEM_FUNCTION_ABOVEH] ? MODE_INPUT5 : MODE_DIRECT;
        }
        if (
            items[ITEM_FUNCTIONHI] || items[ITEM_FUNCTIONLO]
                || items[ITEM_TABLE]
        ) {
            if ( mode ) goto contradictoryModes;
            mode = MODE_SPLIT;
        }
        if ( items[ITEM_SELECT] ) {
            if ( mode ) goto contradictoryModes;
            mode = MODE_SELECT;
        }
        if ( items[ITEM_PARTIALSELECT] ) {
            if ( mode ) goto contradictoryModes;
            mode = MODE_PPSELECT;
        }
        if ( items[ITEM_VARIABLESHIFT] ) {
            if ( mode ) goto contradictoryModes;
            mode = MODE_VARSHIFT;
        }
        if ( items[ITEM_U] || items[ITEM_V] || items[ITEM_RESULT] ) {
            if ( mode ) goto contradictoryModes;
            mode = items[ITEM_ADD3] ? MODE_ADD3 : MODE_CARRY;
        }
        goto markRedundancy;
      contradictoryModes:
        mode = -1;
      markRedundancy:
        rowConfigPtr->logicColModes[colNum] = mode;
        markRedundantColItems( tree, colNum, mode );
    }

}

void getModesAndCheckConsistency( tokenT *tree )
{

    if ( ! tree ) return;
    if ( tree->code == OP_ROW ) {
        getModesAndCheckConsistencyOnRow( tree->b, tree->v.m );
    }
    getModesAndCheckConsistency( tree->t );

}

