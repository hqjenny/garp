
/****/ #include <stdio.h>

#include <stdint.h>
#include "platform.h"
#include "tokens.h"
#include "logicBlocks.h"
#include "GarpArrayConfig.h"
#include "wires.h"
#include "config.h"
#include "addWiresToConfig.h"

static
 void
  addWiresToConfigFromInput(
      tokenT *tree, uint_fast16_t rowNum, int colNum, int bitsShift )
{
    uint_fast16_t sourceRowNum;
    wireT *wirePtr;
    int newBits;

    if ( tree == 0 ) return;
    if ( tree->code == OP_NAMED_ROW ) {
        sourceRowNum = tree->ref->v.m;
        wirePtr = rowConfigPtrs[sourceRowNum]->logicColVOutWires[colNum];
/****/
if ( ! wirePtr ) {
 fprintf(
  stderr, "sourceRowNum: %u colNum: %d\n", (unsigned int) sourceRowNum, colNum
 );
}
        if ( wirePtr == useHWire ) {
            switch ( rowConfigPtrs[sourceRowNum]->HWiresDirection ) {
             case HWIRES_LEFT:
                newBits = GARP_IN_ABOVEH_MIN - (9<<2);
                break;
             case HWIRES_CENTER:
                newBits = GARP_IN_ABOVEH_MIN - (5<<2);
                break;
             case HWIRES_RIGHT:
                newBits = GARP_IN_ABOVEH_MIN - (1<<2);
                break;
            }
        } else {
            newBits = wireToVInOutCode( rowNum, wirePtr )<<2;
        }
        rowConfigPtrs[rowNum]->logicColConfigs[colNum] |=
            ((uint64_t) newBits)<<bitsShift;
    }
    addWiresToConfigFromInput( tree->t, rowNum, colNum, bitsShift );

}

static
 void
  addWiresToConfigFromLogicBlock(
      tokenT *tree, uint_fast16_t rowNum, int colNum )
{

    if ( tree == 0 ) return;
    switch ( tree->code ) {
     case OP_A:
        addWiresToConfigFromInput( tree->a, rowNum, colNum, GARP_INPUT_A );
        break;
     case OP_B:
        addWiresToConfigFromInput( tree->a, rowNum, colNum, GARP_INPUT_B );
        break;
     case OP_C:
        addWiresToConfigFromInput( tree->a, rowNum, colNum, GARP_INPUT_C );
        break;
     case OP_D:
        addWiresToConfigFromInput( tree->a, rowNum, colNum, GARP_INPUT_D );
        break;
    }
    addWiresToConfigFromLogicBlock( tree->t, rowNum, colNum );

}

static
 void
  addWiresToLogicColConfig( tokenT *tree, uint_fast16_t rowNum, int colNum )
{

    if ( tree == 0 ) return;
    if (
        (tree->code == OP_LOGICBLOCK) && logicBlockCoversCol( tree->a, colNum )
    ) {
        addWiresToConfigFromLogicBlock( tree->b, rowNum, colNum );
    }
    addWiresToLogicColConfig( tree->t, rowNum, colNum );

}

static void addWiresToRowConfig( tokenT *tree, uint_fast16_t rowNum )
{
    rowConfigT *rowConfigPtr;
    int colNum;
    wireT *wirePtr;
    uint64_t newBits;

    rowConfigPtr = rowConfigPtrs[rowNum];
    for ( colNum = 0; colNum < NUMLOGICCOLS; ++colNum ) {
        addWiresToLogicColConfig( tree, rowNum, colNum );
        wirePtr = rowConfigPtr->logicColVOutWires[colNum];
        if ( wirePtr ) {
            newBits = 0;
            if ( wirePtr == useHWire ) {
                if ( rowConfigPtr->logicColConfigs[colNum] & GARP_VOUT_D ) {
                    newBits = GARP_HOUT_D;
                }
            } else {
                newBits = (wireToVInOutCode( rowNum, wirePtr ))<<GARP_OUTPUT_V;
            }
            rowConfigPtr->logicColConfigs[colNum] |= newBits;
        }
    }

}

void addWiresToConfig( tokenT *tree )
{

    if ( tree == 0 ) return;
    if ( tree->code == OP_ROW ) addWiresToRowConfig( tree->b, tree->v.m );
    addWiresToConfig( tree->t );

}

