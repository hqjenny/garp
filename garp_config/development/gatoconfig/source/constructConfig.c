
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "dialect.h"
#include "errorMessages.h"
#include "names.h"
#include "tokens.h"
#include "tokenize.h"
#include "logicBlocks.h"
#include "GarpArrayConfig.h"
#include "wires.h"
#include "config.h"
#include "constructConfig.h"

enum {
    PSEUDOTABLE_U = GARP_UVTABLE_A,
    PSEUDOTABLE_V = GARP_UVTABLE_B,
    PSEUDOTABLE_K = GARP_UVTABLE_C
};

static void setLogicColNamedConstants( tokenT *tree, int colNum )
{
    int colOffset, colConstantValue;
    tokenT *constantName;

    while ( tree ) {
        if (
            (tree->code == OP_LOGICBLOCK) && tree->b
                && (tree->b->code == L_NUMBER)
        ) {
            colOffset = colOffsetInIndexList( tree->a, colNum );
            colConstantValue =
                (colOffset < 0) ? 0 : (tree->b->v.n>>(colOffset<<1)) & 3;
            constantName = tree->b->a;
            while ( constantName ) {
                constantName->v.name.namePtr->m = colConstantValue;
                constantName = constantName->t;
            }
        }
        tree = tree->t;
    }

}

static void
 fuseRowWithAbove( uint_fast16_t rowNum, int colNum, tokenT *errorToken )
{
    uint_fast16_t fusedRowCount;

    if ( ! rowNum ) {
        if ( errorToken ) {
            markErrorAtToken( errorToken, "Not valid on first row." );
        }
        return;
    }
    if ( 0 <= colNum ) {
        rowConfigPtrs[rowNum - 1]->logicColHOutUsed[colNum] = true;
    }
    fusedRowCount = fusedRowCounts[rowNum];
    if ( fusedRowCount ) {
        fusedRowCounts[rowNum] = 0;
        do {
            --rowNum;
        } while ( ! fusedRowCounts[rowNum] );
        fusedRowCounts[rowNum] += fusedRowCount;
    }

}

static bool
 horizInputWithinReach(
     rowConfigT *rowConfigPtr,
     int horizRel,
     tokenT *errorToken,
     int colNum,
     int *horizRelPtr
 )
{

    if ( rowConfigPtr->HWiresDirection == HWIRES_LEFT ) {
        horizRel += 4;
    } else if ( rowConfigPtr->HWiresDirection == HWIRES_RIGHT ) {
        horizRel -= 4;
    }
    *horizRelPtr = horizRel;
    if ( (horizRel < -5) || (5 < horizRel) ) {
        markErrorInColumnAtToken( errorToken, "Input too far away", colNum );
        return false;
    } else {
        return true;
    }

}

static void
 addToControlConfigFromInput( tokenT *tree, uint_fast16_t rowNum, int bitsShift )
{
    rowConfigT *rowConfigPtr;
    int newBits, m;

    if ( ! tree ) return;
    rowConfigPtr = rowConfigPtrs[rowNum];
    newBits = GARP_IN_XBAR_10;
    m = tree->v.m;
    switch ( tree->code ) {
     case OP_NAMED_CONSTANT:
        m = tree->v.name.namePtr->m;
        goto constantInput;
     case L_BINARY:
     constantInput:
        switch ( m ) {
         case 0:
            newBits |= GARP_IN_00;
            break;
         case 1:
            newBits += GARP_IN_10 + GARP_IN_XBAR_01 - GARP_IN_XBAR_10;
            break;
         case 2:
            newBits |= GARP_IN_10;
            break;
         case 3:
            newBits += GARP_IN_10 + GARP_IN_XBAR_11 - GARP_IN_XBAR_10;
            break;
        }
        break;
     case OP_VOID:
        break;
     case OP_NAMED_COL:
fputs( "Input is NAMED_COL.\n", stderr );
        break;
     case OP_ABOVEHORIZ_ABS:
        m -= NUMLOGICCOLS;
        goto aboveHorizRel;
     case OP_ABOVEHORIZ_REL:
     aboveHorizRel:
        fuseRowWithAbove( rowNum, NUMLOGICCOLS + m, tree );
        if (
            rowNum
                && horizInputWithinReach(
                       rowConfigPtrs[rowNum - 1], m, tree, NUMLOGICCOLS, &m )
        ) {
            newBits |= GARP_IN_ABOVEH_MIN - ((m + 5)<<2);
        }
        break;
     case OP_ABOVEGLOBAL:
        fuseRowWithAbove( rowNum, -1, tree );
        newBits |= GARP_IN_ABOVEG_0 - (m<<2);
        break;
     case OP_HORIZ_ABS:
        m -= NUMLOGICCOLS;
        goto horizRel;
     case OP_HORIZ_REL:
     horizRel:
        rowConfigPtr->logicColHOutUsed[NUMLOGICCOLS + m] = true;
        if (
            horizInputWithinReach( rowConfigPtr, m, tree, NUMLOGICCOLS, &m )
        ) {
            newBits |= GARP_IN_H_MIN - ((m + 5)<<2);
        }
        break;
     case OP_GLOBAL:
        newBits |= GARP_IN_G_0 - (m<<2);
        break;
    }
    rowConfigPtr->controlConfig |= ((uint64_t) newBits)<<bitsShift;
    addToControlConfigFromInput( tree->t, rowNum, bitsShift );

}

static void addToControlConfig( tokenT *tree, uint_fast16_t rowNum )
{
    rowConfigT *rowConfigPtr;
    uint64_t *configPtr, newBits;
    int colNumLo, colNumHi, m;

    if ( ! tree ) return;
    rowConfigPtr = rowConfigPtrs[rowNum];
    configPtr = &(rowConfigPtr->controlConfig);
    newBits = 0;
    m = tree->v.m;
    switch ( tree->code ) {
     case OP_CONTROLBLOCK:
        addToControlConfig( tree->b, rowNum );
        break;
     case OP_A:
        addToControlConfigFromInput( tree->a, rowNum, GARP_INPUT_A );
        break;
     case OP_B:
        addToControlConfigFromInput( tree->a, rowNum, GARP_INPUT_B );
        break;
     case OP_C:
        addToControlConfigFromInput( tree->a, rowNum, GARP_INPUT_C );
        break;
     case OP_D:
        addToControlConfigFromInput( tree->a, rowNum, GARP_INPUT_D );
        break;
     case OP_TIMES:
        newBits =
            ((uint64_t) tree->a->v.m)<<GARP_SIZE_A | ((uint64_t) m)<<GARP_K;
        break;
     case OP_PROCESSOR:
        newBits = GARP_X_PROCESSOR;
        break;
     case OP_MATCH:
        newBits = ((uint64_t) m)<<GARP_MATCH;
        break;
     case OP_MEMORY:
        newBits = GARP_X_MEMORY;
        break;
     case OP_QUEUE:
        newBits = GARP_QUEUE | ((uint64_t) m)<<GARP_K;
        break;
     case OP_DEMAND:
        addToControlConfig( tree->a, rowNum );
        newBits = GARP_DEMAND;
        break;
     case OP_DEMANDREAD:
        addToControlConfig( tree->a, rowNum );
        newBits = GARP_DEMANDREAD;
        break;
     case OP_LATENCY:
        newBits = ((uint64_t) m)<<GARP_LATENCY;
        break;
     case OP_ALLOCATE:
        newBits = m ? GARP_ALLOCATE : GARP_NOALLOCATE;
        break;
     case OP_ALIGNED:
        newBits = m ? GARP_ALIGNED : GARP_NONALIGNED;
        break;
     case OP_REGBUS:
        switch ( tree->a->code ) {
         case OP_ZREG:
            newBits = GARP_REGBUS_Z;
            break;
         case OP_DREG:
            newBits = GARP_REGBUS_D;
            break;
        }
        newBits |=
            ((uint64_t) tree->a->t->v.m)<<GARP_SIZE_B 
                | ((uint64_t) tree->a->t->t->v.m)<<GARP_BUS;
        break;
     case OP_GOUT:
        newBits = (GARP_OUT_G_0 - tree->a->v.m)<<GARP_OUTPUT_G;
        break;
    }
    *configPtr |= newBits;
    addToControlConfig( tree->t, rowNum );

}

static void
 addToLogicColConfigFromInput(
     tokenT *tree, uint_fast16_t rowNum, int colNum, int bitsShift )
{
    rowConfigT *rowConfigPtr;
    bool XBarMode;
    int newBits, m;

    if ( ! tree ) return;
    rowConfigPtr = rowConfigPtrs[rowNum];
    XBarMode = false;
    newBits = 0;
    switch ( rowConfigPtr->logicColModes[colNum] ) {
     case MODE_DIRECT:
        XBarMode = true;
        newBits = GARP_IN_XBAR_10;
        break;
     case MODE_INPUT5:
     case MODE_SPLIT:
     case MODE_CARRY:
        if ( bitsShift != GARP_INPUT_D ) {
            XBarMode = true;
            newBits = GARP_IN_XBAR_10;
        }
        break;
    }
    m = tree->v.m;
    switch ( tree->code ) {
     case OP_NAMED_CONSTANT:
        m = tree->v.name.namePtr->m;
        goto constantInput;
     case L_BINARY:
     constantInput:
        switch ( m ) {
         case 0:
            newBits |= GARP_IN_00;
            break;
         case 1:
            if ( XBarMode ) {
                newBits += GARP_IN_10 + GARP_IN_XBAR_01 - GARP_IN_XBAR_10;
            } else {
                newBits = GARP_IN_10 | GARP_IN_INVERT;
            }
            break;
         case 2:
            newBits |= GARP_IN_10;
            break;
         case 3:
            if ( XBarMode ) {
                newBits += GARP_IN_10 + GARP_IN_XBAR_11 - GARP_IN_XBAR_10;
            } else {
                newBits = GARP_IN_00 | GARP_IN_INVERT;
            }
            break;
        }
        break;
     case OP_VOID:
        break;
     case OP_NAMED_COL:
fputs( "Input is NAMED_COL.\n", stderr );
        break;
     case OP_NAMED_ROW:
        rowConfigPtr->rowInputs[tree->ref->v.m] |= 1<<colNum;
        break;
     case OP_ZREG:
        newBits |= GARP_IN_ZREG;
        break;
     case OP_DREG:
        newBits |= GARP_IN_DREG;
        break;
     case OP_ABOVEHORIZ_ABS:
        m -= colNum;
        goto aboveHorizRel;
     case OP_ABOVEHORIZ_REL:
     aboveHorizRel:
        fuseRowWithAbove( rowNum, colNum + m, tree );
        if (
            rowNum
                && horizInputWithinReach(
                       rowConfigPtrs[rowNum - 1], m, tree, colNum, &m )
        ) {
            newBits |= GARP_IN_ABOVEH_MIN - ((m + 5)<<2);
        }
        break;
     case OP_ABOVEGLOBAL:
        fuseRowWithAbove( rowNum, -1, tree );
        newBits |= GARP_IN_ABOVEG_0 - (m<<2);
        break;
     case OP_HORIZ_ABS:
        m -= colNum;
        goto horizRel;
     case OP_HORIZ_REL:
     horizRel:
        rowConfigPtr->logicColHOutUsed[colNum + m] = true;
        if ( horizInputWithinReach( rowConfigPtr, m, tree, colNum, &m ) ) {
            newBits |= GARP_IN_H_MIN - ((m + 5)<<2);
        }
        break;
     case OP_GLOBAL:
        newBits |= GARP_IN_G_0 - (m<<2);
        break;
    }
    rowConfigPtr->logicColConfigs[colNum] |= ((uint64_t) newBits)<<bitsShift;
    addToLogicColConfigFromInput( tree->t, rowNum, colNum, bitsShift );

}

static uint_fast16_t tableLookupTable( tokenT *tree )
{
    uint_fast16_t table, newTableBits;
    int i;

    table = 0;
    for ( i = 7; 0 <= i; --i ) {
        newTableBits = 0;
        switch ( tree->code ) {
         case L_BINARY:
            newTableBits = tree->v.m;
            break;
         case OP_NAMED_CONSTANT:
            newTableBits = tree->v.name.namePtr->m;
            break;
        }
        table = table>>1 | (newTableBits & 2)<<14 | (newTableBits & 1)<<7;
        tree = tree->t;
    }
    return table;

}

static uint_fast16_t
 functionLookupTable( tokenT *tree, uint_fast16_t rowNum, int colNum )
{
    uint_fast16_t table;

    if ( ! tree ) return 0;
    switch ( tree->code ) {
     case L_BINARY:
        switch ( tree->v.m ) {
         case 0:
            table = 0x0000;
            break;
         case 3:
            table = 0xFFFF;
            break;
        }
        break;
     case OP_VOID:
        table = 0;
        break;
     case OP_NAMED_CONSTANT:
fputs( "Input is NAMED_CONSTANT.\n", stderr );
        break;
     case OP_A:
        table = GARP_TABLE_A;
        break;
     case OP_B:
        table = GARP_TABLE_B;
        break;
     case OP_C:
        table = GARP_TABLE_C;
        break;
     case OP_D:
        table = GARP_TABLE_D;
        break;
     case OP_ABOVEH:
        fuseRowWithAbove( rowNum, colNum, tree );
        table = GARP_TABLE_ABOVEH;
        break;
     case OP_NOT:
        table = ~functionLookupTable( tree->a, rowNum, colNum );
        break;
     case OP_AND:
        table =
              functionLookupTable( tree->a, rowNum, colNum )
            & functionLookupTable( tree->b, rowNum, colNum );
        break;
     case OP_XOR:
        table =
              functionLookupTable( tree->a, rowNum, colNum )
            ^ functionLookupTable( tree->b, rowNum, colNum );
        break;
     case OP_OR:
        table =
              functionLookupTable( tree->a, rowNum, colNum )
            | functionLookupTable( tree->b, rowNum, colNum );
        break;
    }
    return table & 0xFFFF;

}

static int UVFunctionLookupTable( tokenT *tree )
{
    int table;

    if ( ! tree ) return 0;
    switch ( tree->code ) {
     case L_BINARY:
        switch ( tree->v.m ) {
         case 0:
            table = 0x00;
            break;
         case 3:
            table = 0xFF;
            break;
        }
        break;
     case OP_VOID:
        table = 0;
        break;
     case OP_NAMED_CONSTANT:
fputs( "Input is NAMED_CONSTANT.\n", stderr );
        break;
     case OP_A:
        table = GARP_UVTABLE_A;
        break;
     case OP_B:
        table = GARP_UVTABLE_B;
        break;
     case OP_C:
        table = GARP_UVTABLE_C;
        break;
     case OP_SUM:
        table = GARP_UVTABLE_SUM;
        break;
     case OP_CARRY:
        table = GARP_UVTABLE_CARRY;
        break;
     case OP_U:
        table = PSEUDOTABLE_U;
        break;
     case OP_V:
        table = PSEUDOTABLE_V;
        break;
     case OP_K:
        table = PSEUDOTABLE_K;
        break;
     case OP_KOUT:
        table =
            (PSEUDOTABLE_U & PSEUDOTABLE_K) | (~PSEUDOTABLE_U & PSEUDOTABLE_V);
        break;
     case OP_NOT:
        table = ~UVFunctionLookupTable( tree->a );
        break;
     case OP_AND:
        table =
              UVFunctionLookupTable( tree->a )
            & UVFunctionLookupTable( tree->b );
        break;
     case OP_XOR:
        table =
              UVFunctionLookupTable( tree->a )
            ^ UVFunctionLookupTable( tree->b );
        break;
     case OP_OR:
        table =
              UVFunctionLookupTable( tree->a )
            | UVFunctionLookupTable( tree->b );
        break;
    }
    return table & 0xFF;

}

static void addToLogicColConfigFromHOut( tokenT *tree, uint64_t *configPtr )
{
    uint64_t newBits;

    if ( ! tree ) return;
    newBits = 0;
    switch ( tree->code ) {
     case OP_Z:
        newBits = GARP_HOUT_Z;
        break;
     case OP_D:
        newBits = GARP_HOUT_D;
        break;
    }
    *configPtr |= newBits;

}

static void addToLogicColConfigFromGOut( tokenT *tree, uint64_t *configPtr )
{
    uint64_t newBits;

    if ( ! tree ) return;
    newBits = 0;
    switch ( tree->code ) {
     case OP_Z:
        newBits = GARP_GOUT_Z;
        break;
     case OP_D:
        newBits = GARP_GOUT_D;
        break;
     case OP_GLOBAL:
        newBits = (GARP_OUT_G_0 - tree->v.m)<<GARP_OUTPUT_G;
        break;
    }
    *configPtr |= newBits;
    addToLogicColConfigFromGOut( tree->t, configPtr );

}

static bool addToLogicColConfigFromVOut( tokenT *tree, uint64_t *configPtr )
{
    uint64_t newBits;
    bool VOutLong;

    if ( ! tree ) return false;
    newBits = 0;
    VOutLong = false;
    switch ( tree->code ) {
     case OP_Z:
        newBits = GARP_VOUT_Z;
        break;
     case OP_D:
        newBits = GARP_VOUT_D;
        break;
     case OP_LONG:
        VOutLong = true;
        break;
    }
    *configPtr |= newBits;
    return VOutLong | addToLogicColConfigFromVOut( tree->t, configPtr );

}

static void
 addToLogicColConfigFromLogicBlock(
     tokenT *tree, uint_fast16_t rowNum, int colNum )
{
    rowConfigT *rowConfigPtr;
    uint64_t *configPtr;
    uint64_t newBits;
    int colNumLo, colNumHi;

    if ( ! tree ) return;
    rowConfigPtr = rowConfigPtrs[rowNum];
    configPtr = &(rowConfigPtr->logicColConfigs[colNum]);
    newBits = 0;
    switch ( tree->code ) {
     case OP_A:
        addToLogicColConfigFromInput( tree->a, rowNum, colNum, GARP_INPUT_A );
        break;
     case OP_B:
        addToLogicColConfigFromInput( tree->a, rowNum, colNum, GARP_INPUT_B );
        break;
     case OP_C:
        addToLogicColConfigFromInput( tree->a, rowNum, colNum, GARP_INPUT_C );
        break;
     case OP_D:
        addToLogicColConfigFromInput( tree->a, rowNum, colNum, GARP_INPUT_D );
        break;
     case OP_FUNCTION:
        newBits =
            ((rowConfigPtr->logicColModes[colNum] == MODE_INPUT5)
                 ? GARP_MODE_INPUT5
                 : GARP_MODE_DIRECT)
                | ((uint64_t) functionLookupTable( tree->a, rowNum, colNum ))
                      <<GARP_TABLE;
        break;
     case OP_FUNCTIONLO:
        newBits =
            GARP_MODE_SPLIT
                | ((uint64_t)
                       (functionLookupTable( tree->a, rowNum, colNum )
                            & ~GARP_TABLE_D))
                      <<GARP_TABLE;
        break;
     case OP_FUNCTIONHI:
        newBits =
            GARP_MODE_SPLIT
                | ((uint64_t)
                       (functionLookupTable( tree->a, rowNum, colNum )
                            & GARP_TABLE_D))
                      <<GARP_TABLE;
        break;
     case OP_TABLE:
        newBits =
            GARP_MODE_SPLIT
                | ((uint64_t) tableLookupTable( tree->a ))<<GARP_TABLE;
        break;
     case OP_SELECT:
        fuseRowWithAbove( rowNum, colNum, 0 );
        newBits = GARP_MODE_SELECT | GARP_MODE_SHIFTIN_X | GARP_DEFAULTTABLE;
        break;
     case OP_PARTIALSELECT:
        newBits = GARP_MODE_PPSELECT | GARP_MODE_SHIFTIN_X | GARP_DEFAULTTABLE;
        break;
     case OP_VARIABLESHIFT:
        fuseRowWithAbove( rowNum, colNum, tree );
        if ( rowNum ) {
            colNumLo = colNum;
            colNumHi = colNum;
            switch ( rowConfigPtr->HWiresDirection ) {
             case HWIRES_CENTER:
                colNumLo -= 4;
                colNumHi += 4;
                break;
             case HWIRES_LEFT:
                colNumLo -= 8;
                break;
             case HWIRES_RIGHT:
                colNumHi += 8;
                break;
            }
            if ( colNumLo < 0 ) colNumLo = 0;
            if ( NUMLOGICCOLS <= colNumHi ) colNumHi = NUMLOGICCOLS - 1;
            while ( colNumLo <= colNumHi ) {
                rowConfigPtrs[rowNum - 1]->logicColHOutUsed[colNumLo] = true;
                ++colNumLo;
            }
        }
        newBits = GARP_MODE_VARSHIFT | GARP_MODE_SHIFTIN_X | GARP_DEFAULTTABLE;
        break;
     case OP_U:
        newBits =
            GARP_MODE_CARRY | GARP_MODE_SHIFTIN_X
                | ((uint64_t) UVFunctionLookupTable( tree->a ))<<GARP_UTABLE;
        break;
     case OP_V:
        newBits =
            GARP_MODE_CARRY | GARP_MODE_SHIFTIN_X
                | ((uint64_t) UVFunctionLookupTable( tree->a ))<<GARP_VTABLE;
        break;
     case OP_RESULT:
        switch ( UVFunctionLookupTable( tree->a ) ) {
         case PSEUDOTABLE_V:
            newBits = GARP_RESULT_V;
            break;
         case PSEUDOTABLE_V | PSEUDOTABLE_K:
            if ( 2 <= GarpVersion ) goto noSuchResultFunction;
            newBits = GARP_RESULT_V_OR_K;
            break;
         case (PSEUDOTABLE_U & PSEUDOTABLE_K )
                  | ((0xFF - PSEUDOTABLE_U) & PSEUDOTABLE_V):
            if ( GarpVersion < 2 ) goto noSuchResultFunction;
            newBits = GARP_RESULT_KOUT;
            break;
         case PSEUDOTABLE_U ^ PSEUDOTABLE_K:
            newBits = GARP_RESULT_U_XOR_K;
            break;
         case 0xFF - (PSEUDOTABLE_U ^ PSEUDOTABLE_K):
            newBits = GARP_RESULT_U_EQU_K;
            break;
         default:
         noSuchResultFunction:
            markErrorAtToken( tree, "Not an available function." );
            break;
        }
        break;
     case OP_BUFFERZ:
        newBits = GARP_BUFFERZ;
        break;
     case OP_BUFFERD:
        newBits = GARP_BUFFERD;
        break;
     case OP_HOUT:
        rowConfigPtrs[rowNum]->logicColHOutUsed[colNum] = true;
        addToLogicColConfigFromHOut( tree->a, configPtr );
        break;
     case OP_GOUT:
        addToLogicColConfigFromGOut( tree->a, configPtr );
        break;
     case OP_VOUT:
        if ( addToLogicColConfigFromVOut( tree->a, configPtr ) ) {
            rowConfigPtr->logicColVOutLong[colNum] = true;
        }
        break;
    }
    *configPtr |= newBits;
    addToLogicColConfigFromLogicBlock( tree->t, rowNum, colNum );

}

static void addToLogicColConfig( tokenT *tree, uint_fast16_t rowNum, int colNum )
{

    if ( ! tree ) return;
    switch ( tree->code ) {
     case OP_LOGICBLOCK:
        if ( logicBlockCoversCol( tree->a, colNum ) ) {
            addToLogicColConfigFromLogicBlock( tree->b, rowNum, colNum );
        }
        break;
    }
    addToLogicColConfig( tree->t, rowNum, colNum );

}

static void
 adjustControlConfigFromInput( tokenT *tree, uint64_t *configPtr, int bitsShift )
{
    int permutationBits;

    if ( ! tree ) return;
    permutationBits = ((*configPtr)>>bitsShift) & 3;
    *configPtr &= ~(((uint64_t) 3)<<bitsShift);
    switch ( tree->code ) {
     case OP_LO:
        permutationBits = (permutationBits & 1) * 3;
        break;
     case OP_HI:
        permutationBits = (permutationBits>>1) * 3;
        break;
    }
    *configPtr |= ((uint64_t) permutationBits)<<bitsShift;
    adjustControlConfigFromInput( tree->t, configPtr, bitsShift );

}

static void adjustControlConfig( tokenT *tree, uint64_t *configPtr )
{

    if ( ! tree ) return;
    switch ( tree->code ) {
     case OP_CONTROLBLOCK:
        adjustControlConfig( tree->b, configPtr );
        break;
     case OP_A:
        adjustControlConfigFromInput( tree->a, configPtr, GARP_INPUT_A );
        break;
     case OP_B:
        adjustControlConfigFromInput( tree->a, configPtr, GARP_INPUT_B );
        break;
     case OP_C:
        adjustControlConfigFromInput( tree->a, configPtr, GARP_INPUT_C );
        break;
     case OP_D:
        adjustControlConfigFromInput( tree->a, configPtr, GARP_INPUT_D );
        break;
    }
    adjustControlConfig( tree->t, configPtr );

}

static void
 adjustLogicColConfigFromInput(
     tokenT *tree, uint64_t *configPtr, int bitsShift )
{
    int permutationBits;

    if ( ! tree ) return;
    permutationBits = ((*configPtr)>>bitsShift) & 3;
    *configPtr &= ~(((uint64_t) 3)<<bitsShift);
    switch ( tree->code ) {
     case OP_LO:
        permutationBits = (permutationBits & 1) * 3;
        break;
     case OP_HI:
        permutationBits = (permutationBits>>1) * 3;
        break;
     case OP_SWAP:
        permutationBits = (permutationBits & 1)<<1 | permutationBits>>1;
        break;
     case OP_SHIFT1:
        permutationBits |= GARP_IN_SHIFT1;
        break;
     case OP_NOT:
        permutationBits ^= GARP_IN_INVERT;
        break;
    }
    *configPtr |= ((uint64_t) permutationBits)<<bitsShift;
    adjustLogicColConfigFromInput( tree->t, configPtr, bitsShift );

}

static void
 adjustLogicColConfigFromLogicBlock( tokenT *tree, uint64_t *configPtr )
{

    if ( ! tree ) return;
    switch ( tree->code ) {
     case OP_A:
        adjustLogicColConfigFromInput( tree->a, configPtr, GARP_INPUT_A );
        break;
     case OP_B:
        adjustLogicColConfigFromInput( tree->a, configPtr, GARP_INPUT_B );
        break;
     case OP_C:
        adjustLogicColConfigFromInput( tree->a, configPtr, GARP_INPUT_C );
        break;
     case OP_D:
        adjustLogicColConfigFromInput( tree->a, configPtr, GARP_INPUT_D );
        break;
     case OP_ADD3:
        *configPtr += GARP_MODE_ADD3 - GARP_MODE_CARRY;
        break;
     case OP_SHIFTZEROIN:
        *configPtr += GARP_MODE_SHIFTIN_0 - GARP_MODE_SHIFTIN_X;
        break;
    }
    adjustLogicColConfigFromLogicBlock( tree->t, configPtr );

}

static void
 adjustLogicColConfig( tokenT *tree, int colNum, uint64_t *configPtr )
{

    if ( ! tree ) return;
    switch ( tree->code ) {
     case OP_LOGICBLOCK:
        if ( logicBlockCoversCol( tree->a, colNum ) ) {
            adjustLogicColConfigFromLogicBlock( tree->b, configPtr );
        }
        break;
    }
    adjustLogicColConfig( tree->t, colNum, configPtr );

}

static void constructRowConfig( tokenT *tree, uint_fast16_t rowNum )
{
    rowConfigT *rowConfigPtr;
    int colNum;

    rowConfigPtr = rowConfigPtrs[rowNum];
/*** WHERE ELSE DOES HWiresDirection GET SET? ***/
    if ( ! rowConfigPtr->HWiresDirection ) {
        if ( (rowConfigPtr->horizMin < -5) && (rowConfigPtr->horizMax <= 1) ) {
            rowConfigPtr->HWiresDirection = HWIRES_LEFT;
        } else if (
            (5 < rowConfigPtr->horizMax) && (-1 <= rowConfigPtr->horizMin)
        ) {
            rowConfigPtr->HWiresDirection = HWIRES_RIGHT;
        } else {
            rowConfigPtr->HWiresDirection = HWIRES_CENTER;
        }
    }
    switch ( rowConfigPtr->HWiresDirection ) {
     case HWIRES_LEFT:
        rowConfigPtr->controlConfig |= GARP_HDIR_LEFT;
        break;
     case HWIRES_CENTER:
        rowConfigPtr->controlConfig |= GARP_HDIR_CENTER;
        break;
     case HWIRES_RIGHT:
        rowConfigPtr->controlConfig |= GARP_HDIR_RIGHT;
        break;
    }
    addToControlConfig( tree, rowNum );
    adjustControlConfig( tree, &(rowConfigPtr->controlConfig) );
    for ( colNum = 0; colNum < NUMLOGICCOLS; ++colNum ) {
        setLogicColNamedConstants( tree, colNum );
        addToLogicColConfig( tree, rowNum, colNum );
        adjustLogicColConfig(
            tree, colNum, &(rowConfigPtr->logicColConfigs[colNum]) );
    }

}

void constructConfig( tokenT *tree )
{

    if ( ! tree ) return;
    if ( tree->code == OP_ROW ) constructRowConfig( tree->b, tree->v.m );
    constructConfig( tree->t );

}

