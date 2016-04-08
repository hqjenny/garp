
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "platform.h"
#include "errorMessages.h"
#include "tokens.h"
#include "names.h"
#include "tokenize.h"
#include "wires.h"
#include "config.h"
#include "fixSyntax.h"

static bool fixKeyword( tokenT *nameToken )
{
    nameT *namePtr;
    int tokenCode;
    char *nameString;
    int numberPosition;
    uint_fast32_t n;

    namePtr = nameToken->v.name.namePtr;
    tokenCode = namePtr->tokenCode;
    if ( tokenCode ) {
        nameToken->code = tokenCode;
        nameToken->v.n = 0;
        nameToken->v.m = namePtr->m;
        return false;
    } else {
        nameString = namePtr->string;
        if ( strncmp( nameString, "col", 3 ) == 0 ) {
            tokenCode = OP_HORIZ_ABS;
            numberPosition = 3;
        } else if ( strncmp( nameString, "abovecol", 8 ) == 0 ) {
            tokenCode = OP_ABOVEHORIZ_ABS;
            numberPosition = 8;
        } else {
            while ( *nameString == '_' ) ++nameString;
            if ( *nameString ) {
                return true;
            } else {
                namePtr->tokenCode = OP_VOID;
                nameToken->code = OP_VOID;
                return false;
            }
        }
        n = strtoul( nameString + numberPosition, &nameString, 10 );
        if ( *nameString || (NUMLOGICCOLS <= n) ) return true;
        namePtr->tokenCode = tokenCode;
        namePtr->m = n;
        nameToken->code = tokenCode;
        nameToken->v.n = 0;
        nameToken->v.m = n;
        return false;
    }

}

static char argumentErrorMessage[] = "Invalid argument.";

static
 void
  fixNumericItemSyntax(
      tokenT *tree, int_fast32_t lowLimit, int_fast32_t highLimit )
{
    tokenT *t;
    int n;

    t = tree->a;
    if ( t && t->t ) {
        markErrorAtToken( tree, "Item takes a single numeric argument." );
        return;
    }
    if ( ! t ) return;
    if ( t->code == L_NAME ) (void) fixKeyword( t );
    if ( t->code == OP_VOID ) return;
    if ( t->code == L_BINARY ) {
        switch ( t->v.m ) {
         case 0:
            n = 0;
            break;
         case 1:
            n = 1;
            break;
         case 2:
            n = 10;
            break;
         case 3:
            n = 11;
            break;
        }
        t->code = L_NUMBER;
        t->v.m = 0;
        t->v.n = n;
    }
    if ( t->code != L_NUMBER ) {
        markErrorAtToken( tree, "Item takes a single numeric argument." );
        return;
    }
    if ( (t->v.n < lowLimit) || (highLimit <= t->v.n) ) {
        markErrorAtToken( t, "Argument out of range." );
        return;
    }
    tree->v.m = t->v.n;
    freeToken( tree->a );
    tree->a = 0;

}

static void fixControlInputSyntax( tokenT *tree )
{
    bool error;

    if ( ! tree ) return;
    if ( tree->code == L_NAME ) (void) fixKeyword( tree );
    error = false;
    switch ( tree->code ) {
     case L_NUMBER:
        if ( tree->v.n == 0 ) {
            tree->code = L_BINARY;
            tree->v.m = 0;
        } else if ( tree->v.n == 1 ) {
            tree->code = L_BINARY;
            tree->v.m = 3;
        } else {
            error = true;
        }
        break;
     case L_BINARY:
     case OP_VOID:
     case OP_ABOVEHORIZ_REL:
     case OP_ABOVEHORIZ_ABS:
     case OP_ABOVEGLOBAL:
     case OP_HORIZ_REL:
     case OP_HORIZ_ABS:
     case OP_GLOBAL:
     case OP_LO:
     case OP_HI:
     case OP_EITHER:
        break;
     case OP_NAMED_COL:
        tree->code = OP_HORIZ_ABS;
        tree->v.m = ((tokenT *) (tree->ref))->v.n;
        tree->ref = 0;
        break;
     default:
        error = true;
        break;
    }
    if ( error ) markErrorAtToken( tree, argumentErrorMessage );
    fixControlInputSyntax( tree->t );

}

static void fixDemandItemSyntax( tokenT *tree )
{
    bool error;
    tokenT *t;

    if ( ! tree ) return;
    if ( tree->code == L_NAME ) (void) fixKeyword( tree );
    error = false;
    switch ( tree->code ) {
     case OP_VOID:
     case OP_ALLOCATE:
     case OP_ALIGNED:
        break;
     case OP_TIMES:
        t = tree->a;
        if ( t->code == L_NAME ) (void) fixKeyword( t );
        if ( t->code != OP_SIZE ) {
            error = true;
            break;
        }
        t = tree->b;
        if ( t->code != L_NUMBER ) {
            error = true;
            break;
        }
        if ( t->v.n == 1 ) {
            tree->v.m = 0;
        } else if ( t->v.n == 2 ) {
            tree->v.m = 1;
        } else if ( t->v.n == 4 ) {
            tree->v.m = 2;
        } else {
            error = true;
            break;
        }
        tree->b = 0;
        freeToken( t );
        break;
     default:
        t = tree;
        error = true;
        break;
    }
    if ( error ) markErrorAtToken( t, argumentErrorMessage );
    fixDemandItemSyntax( tree->t );

}

static void fixRegbusItemSyntax( tokenT *tree )
{
    tokenT *t;

    t = tree->a;
    if ( ! t ) return;
    if ( ! t->t || ! t->t->t || t->t->t->t ) {
        markErrorAtToken( tree, "Item requires three arguments." );
        return;
    }
    if ( t->code == L_NAME ) (void) fixKeyword( t );
    switch ( t->code ) {
     case OP_VOID:
     case OP_ZREG:
     case OP_DREG:
        break;
     default:
        markErrorAtToken( t, argumentErrorMessage );
        break;
    }
    t = t->t;
    if ( t->code == L_NAME ) (void) fixKeyword( t );
    switch ( t->code ) {
     case OP_VOID:
     case OP_SIZE:
        break;
     default:
        markErrorAtToken( t, argumentErrorMessage );
        break;
    }
    t = t->t;
    if ( t->code == L_NAME ) (void) fixKeyword( t );
    switch ( t->code ) {
     case OP_VOID:
     case OP_BUS:
        break;
     default:
        markErrorAtToken( t, argumentErrorMessage );
        break;
    }

}

static void fixControlGoutSyntax( tokenT *tree )
{
    bool error;

    if ( ! tree ) return;
    if ( tree->code == L_NAME ) (void) fixKeyword( tree );
    error = false;
    switch ( tree->code ) {
     case OP_VOID:
     case OP_GLOBAL:
        break;
     default:
        error = true;
        break;
    }
    if ( error ) markErrorAtToken( tree, argumentErrorMessage );
    fixControlGoutSyntax( tree->t );

}

static bool fixControlItemSyntax( tokenT *tree, bool hasArgs )
{

    switch ( tree->code ) {
     case OP_A:
     case OP_B:
     case OP_C:
     case OP_D:
     case OP_MATCH:
     case OP_DEMAND:
     case OP_DEMANDREAD:
     case OP_LATENCY:
     case OP_REGBUS:
     case OP_GOUT:
        if ( ! hasArgs ) {
            markErrorAtToken( tree, "Item requires arguments." );
            return false;
        }
        break;
     case OP_PROCESSOR:
     case OP_MEMORY:
     case OP_QUEUE:
     case OP_HDIR:
        if ( hasArgs ) markErrorAtToken( tree, "Item takes no arguments." );
        return false;
     default:
        return true;
    }
    switch ( tree->code ) {
     case OP_A:
     case OP_B:
     case OP_C:
     case OP_D:
        fixControlInputSyntax( tree->a );
        break;
     case OP_MATCH:
        fixNumericItemSyntax( tree, 0, 64 );
        break;
     case OP_DEMAND:
     case OP_DEMANDREAD:
        fixDemandItemSyntax( tree->a );
        break;
     case OP_LATENCY:
        fixNumericItemSyntax( tree, 1, 9 );
        break;
     case OP_REGBUS:
        fixRegbusItemSyntax( tree );
        break;
     case OP_GOUT:
        fixControlGoutSyntax( tree->a );
        break;
    }
    return false;

}

void fixControlSyntax( tokenT *tree )
{

    if ( ! tree ) return;
    switch ( tree->code ) {
     case OP_ITEM:
        if ( fixKeyword( tree ) || fixControlItemSyntax( tree, false ) ) {
            markErrorAtToken( tree, "Not a valid item." );
        }
        break;
     case OP_ITEMWITHARGS:
        if ( fixKeyword( tree ) || fixControlItemSyntax( tree, true ) ) {
            markErrorAtToken( tree, "Not a valid item." );
        }
        break;
     default:
        fixControlSyntax( tree->a );
        fixControlSyntax( tree->b );
        break;
    }
    fixControlSyntax( tree->t );

}

static void fixInputSyntax( tokenT *tree, bool isD )
{
    bool error;
    tokenT *t;

    if ( ! tree ) return;
    if ( tree->code == L_NAME ) (void) fixKeyword( tree );
    error = false;
    switch ( tree->code ) {
     case L_BINARY:
        error = isD && (tree->v.m & 1);
        break;
     case OP_VOID:
     case OP_NAMED_CONSTANT:
     case OP_NAMED_ROW:
     case OP_ZREG:
     case OP_DREG:
     case OP_ABOVEHORIZ_REL:
     case OP_ABOVEHORIZ_ABS:
     case OP_ABOVEGLOBAL:
     case OP_HORIZ_REL:
     case OP_HORIZ_ABS:
     case OP_GLOBAL:
     case OP_LO:
     case OP_HI:
     case OP_SWAP:
        break;
     case OP_NAMED_COL:
        tree->code = OP_HORIZ_ABS;
        tree->v.m = ((tokenT *) (tree->ref))->v.n;
        tree->ref = 0;
        break;
     case OP_SHIFT1:
        if ( isD ) markErrorAtToken( tree, argumentErrorMessage );
        break;
     case OP_NOT:
        if ( isD ) {
            markErrorAtToken( tree, argumentErrorMessage );
        } else {
            t = tree->a;
            if ( t ) {
                if ( t->code == L_NAME ) {
                    (void) fixKeyword( t );
                    if ( t->code == OP_SHIFT1 ) return;
                }
                markErrorAtToken( t, argumentErrorMessage );
            }
        }
        break;
     default:
        error = true;
        break;
    }
    if ( error ) markErrorAtToken( tree, argumentErrorMessage );
    fixInputSyntax( tree->t, isD );

}

static
 void
  fixFunctionSyntaxZ(
      tokenT *tree,
      bool allowABC,
      bool allowInput4,
      bool allowCarrySum,
      bool allowUVK
  )
{
    bool error;
    tokenT *ref;

    if ( ! tree ) return;
    if ( tree->code == L_NAME ) (void) fixKeyword( tree );
    error = false;
    switch ( tree->code ) {
     case L_BINARY:
        error = (tree->v.m == 1) || (tree->v.m == 2);
        break;
     case OP_NAMED_CONSTANT:
        break;
     case OP_NOT:
     case OP_AND:
     case OP_XOR:
     case OP_OR:
        fixFunctionSyntaxZ(
            tree->a, allowABC, allowInput4, allowCarrySum, allowUVK );
        fixFunctionSyntaxZ(
            tree->b, allowABC, allowInput4, allowCarrySum, allowUVK );
        break;
     case OP_A:
     case OP_B:
     case OP_C:
        error = ! allowABC;
        break;
     case OP_D:
     case OP_ABOVEH:
        error = ! allowInput4;
        break;
     case OP_CARRY:
     case OP_SUM:
        error = ! allowCarrySum;
        break;
     case OP_U:
     case OP_V:
     case OP_K:
     case OP_KOUT:
        error = ! allowUVK;
        break;
     default:
        error = true;
        break;
    }
    if ( error ) markErrorAtToken( tree, "Invalid expression component." );

}

static
 void
  fixFunctionSyntax(
      tokenT *tree,
      bool allowABC,
      bool allowInput4,
      bool allowCarrySum,
      bool allowUVK
  )
{
    tokenT *t;

    t = tree->a;
    if ( ! t ) return;
    if ( t->t ) {
        markErrorAtToken( tree, "Item takes a single expression argument." );
        return;
    }
    if ( t->code == L_NAME ) (void) fixKeyword( t );
    if ( t->code != OP_VOID ) {
        fixFunctionSyntaxZ(
            t, allowABC, allowInput4, allowCarrySum, allowUVK );
    }

}

static bool fixTableSyntax( tokenT *tree, int argCount )
{
    bool error;
    tokenT *ref;

    if ( ! tree ) return argCount != 0;
    if ( tree->code == L_NAME ) (void) fixKeyword( tree );
    error = false;
    switch ( tree->code ) {
     case L_BINARY:
     case OP_VOID:
     case OP_NAMED_CONSTANT:
        break;
     default:
        error = true;
        break;
    }
    if ( error ) markErrorAtToken( tree, argumentErrorMessage );
    return fixTableSyntax( tree->t, argCount - 1 );

}

static void fixOutputSyntax( tokenT *tree, bool allowGlobal, bool allowLong )
{
    bool error;

    if ( ! tree ) return;
    if ( tree->code == L_NAME ) (void) fixKeyword( tree );
    error = false;
    switch ( tree->code ) {
     case OP_VOID:
     case OP_Z:
     case OP_D:
        break;
     case OP_GLOBAL:
        error = ! allowGlobal;
        break;
     case OP_LONG:
        error = ! allowLong;
        break;
     default:
        error = true;
        break;
    }
    if ( error ) markErrorAtToken( tree, argumentErrorMessage );
    fixOutputSyntax( tree->t, allowGlobal, allowLong );

}

static bool fixItemSyntax( tokenT *tree, bool hasArgs )
{

    switch ( tree->code ) {
     case OP_A:
     case OP_B:
     case OP_C:
     case OP_D:
     case OP_FUNCTION:
     case OP_FUNCTIONLO:
     case OP_FUNCTIONHI:
     case OP_TABLE:
     case OP_U:
     case OP_V:
     case OP_RESULT:
     case OP_HOUT:
     case OP_GOUT:
     case OP_VOUT:
        if ( ! hasArgs ) {
            markErrorAtToken( tree, "Item requires arguments." );
            return false;
        }
        break;
     case OP_SELECT:
     case OP_PARTIALSELECT:
     case OP_VARIABLESHIFT:
     case OP_ADD3:
     case OP_SHIFTZEROIN:
     case OP_BUFFERZ:
     case OP_BUFFERD:
        if ( hasArgs ) markErrorAtToken( tree, "Item takes no arguments." );
        return false;
     default:
        return true;
    }
    switch ( tree->code ) {
     case OP_A:
     case OP_B:
     case OP_C:
        fixInputSyntax( tree->a, false );
        break;
     case OP_D:
        fixInputSyntax( tree->a, true );
        break;
     case OP_FUNCTION:
        fixFunctionSyntax( tree, true, true, false, false );
        break;
     case OP_FUNCTIONLO:
     case OP_FUNCTIONHI:
        fixFunctionSyntax( tree, true, false, false, false );
        break;
     case OP_TABLE:
        if ( fixTableSyntax( tree->a, 8 ) ) {
            markErrorAtToken( tree, "Item takes 8 constant arguments." );
        }
        break;
     case OP_U:
     case OP_V:
        fixFunctionSyntax( tree, true, false, true, false );
        break;
     case OP_RESULT:
        fixFunctionSyntax( tree, false, false, false, true );
        break;
     case OP_HOUT:
        fixOutputSyntax( tree->a, false, false );
        break;
     case OP_GOUT:
        fixOutputSyntax( tree->a, true, false );
        break;
     case OP_VOUT:
        fixOutputSyntax( tree->a, false, true );
        break;
    }
    return false;

}

static void fixColNumbers( tokenT *tree )
{
    tokenT *t;

    if ( ! tree ) return;
    switch ( tree->code ) {
     case L_NUMBER:
        if ( NUMLOGICCOLS <= tree->v.n ) {
            markErrorAtToken( tree, "Column number too large." );
        }
        break;
     case OP_COLRANGE:
        t = tree->a;
        if ( tree->b->v.n < t->v.n ) {
            tree->a = tree->b;
            tree->b = t;
        }
        fixColNumbers( tree->a );
        fixColNumbers( tree->b );
        break;
    }
    fixColNumbers( tree->t );

}

void fixSyntax( tokenT *tree )
{

    if ( ! tree ) return;
    switch ( tree->code ) {
     case OP_ROW:
        fixSyntax( tree->b );
        break;
     case OP_LOGICBLOCK:
        if ( tree->a->code == L_NAME ) {
            if (
                fixKeyword( tree->a ) || (tree->a->code != OP_CONTROLBLOCK)
            ) {
                markErrorAtToken( tree->a, "Column number expected." );
                fixSyntax( tree->b );
            } else {
                tree->code = OP_CONTROLBLOCK;
                freeToken( tree->a );
                tree->a = 0;
                fixControlSyntax( tree->b );
            }
        } else {
            fixColNumbers( tree->a );
            fixSyntax( tree->b );
        }
        break;
     case OP_ITEM:
        if ( fixKeyword( tree ) || fixItemSyntax( tree, false ) ) {
            markErrorAtToken( tree, "Not a valid item." );
        }
        break;
     case OP_ITEMWITHARGS:
        if ( fixKeyword( tree ) || fixItemSyntax( tree, true ) ) {
            markErrorAtToken( tree, "Not a valid item." );
        }
        break;
     default:
        fixSyntax( tree->a );
        fixSyntax( tree->b );
        break;
    }
    fixSyntax( tree->t );

}

