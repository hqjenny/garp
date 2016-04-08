
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "platform.h"
#include "allocate.h"
#include "writeUint64AsText.h"
#include "tokens.h"
#include "names.h"
#include "tokenize.h"

tokenT *freeTokenList = 0;

enum { numAllocatedTokensIncrement = 64 };

tokenT * newTokenZ( void )
{
    tokenT *tokenPtr;
    int i;

    if ( freeTokenList ) {
        tokenPtr = freeTokenList;
        freeTokenList = freeTokenList->t;
    } else {
        tokenPtr = allocArrayType( tokenT, numAllocatedTokensIncrement );
        tokenPtr->t = 0;
        tokenPtr->a = 0;
        tokenPtr->b = 0;
        for ( i = 1; i < (numAllocatedTokensIncrement - 1); ++i ) {
            ++tokenPtr;
            tokenPtr->t = tokenPtr - 1;
            tokenPtr->a = 0;
            tokenPtr->b = 0;
        }
        freeTokenList = tokenPtr;
        ++tokenPtr;
        tokenPtr->a = 0;
        tokenPtr->b = 0;
    }
    tokenPtr->lineNum = 0;
    tokenPtr->columnNum = 0;
    tokenPtr->v.n = 0;
    tokenPtr->ref = 0;
    tokenPtr->t = 0;
    return tokenPtr;

}

void freshenToken( tokenT *tokenPtr, uint_fast16_t tokenCode )
{

    tokenPtr->lineNum = 0;
    tokenPtr->columnNum = 0;
    tokenPtr->code = tokenCode;
    tokenPtr->v.n = 0;
    tokenPtr->ref = 0;
    tokenPtr->t = 0;
    tokenPtr->a = 0;
    tokenPtr->b = 0;

}

tokenT * newCopyToken( tokenT *tokenPtr )
{
    tokenT *newTokenPtr;

    newTokenPtr = newTokenZ();
    *newTokenPtr = *tokenPtr;
    return newTokenPtr;

}

tokenT * newCopyTokens( tokenT *tokenPtr )
{
    tokenT *t, *a, *b;

    if ( tokenPtr == 0 ) return 0;
    t = newCopyTokens( tokenPtr->t );
    a = newCopyTokens( tokenPtr->a );
    b = newCopyTokens( tokenPtr->b );
    tokenPtr = newCopyToken( tokenPtr );
    tokenPtr->t = t;
    tokenPtr->a = a;
    tokenPtr->b = b;
    return tokenPtr;

}

void freeToken( tokenT *tokenPtr )
{

    tokenPtr->t = freeTokenList;
    tokenPtr->a = 0;
    tokenPtr->b = 0;
    freeTokenList = tokenPtr;

}

static tokenT * linearizeTokensAndReturnTail( tokenT *tree )
{
    tokenT *treeT, *treeA, *treeB;
    tokenT *tail;

    treeT = tree->t;
    treeA = tree->a;
    treeB = tree->b;
    if ( treeT == 0 ) {
        if ( treeA ) {
            treeT = treeA;
            treeA = 0;
        } else if ( treeB ) {
            treeT = treeB;
            treeB = 0;
        } else {
            return tree;
        }
    }
    tree->t = treeT;
    tree->a = 0;
    tree->b = 0;
    tail = linearizeTokensAndReturnTail( treeT );
    if ( treeA ) {
        tail->t = treeA;
        tail = linearizeTokensAndReturnTail( treeA );
    }
    if ( treeB ) {
        tail->t = treeB;
        tail = linearizeTokensAndReturnTail( treeB );
    }
    return tail;

}

void freeTokens( tokenT *tree )
{
    tokenT *tail;

    if ( tree ) {
        tail = linearizeTokensAndReturnTail( tree );
        tail->t = freeTokenList;
        freeTokenList = tree;
    }

}

void insertTokenA( tokenT *tokenPtr, uint_fast16_t tokenCode )
{
    tokenT *tokenPtrA;

    tokenPtrA = newCopyToken( tokenPtr );
    freshenToken( tokenPtr, tokenCode );
    tokenPtr->a = tokenPtrA;

}

void removeTokenA( tokenT *tokenPtr )
{
    tokenT *tokenPtrA;

    tokenPtrA = tokenPtr->a;
    *tokenPtr = *tokenPtrA;
    freeToken( tokenPtrA );

}

static tokenT * findLeftmostToken( tokenT *tree )
{
    tokenT *leftmostToken, *leftmostTokenT, *leftmostTokenA, *leftmostTokenB;

    leftmostToken = tree;
    if ( tree->t ) {
        leftmostTokenT = findLeftmostToken( tree->t );
        if (
            (leftmostTokenT->lineNum < leftmostToken->lineNum)
                || ((leftmostTokenT->lineNum == leftmostToken->lineNum)
                        && (leftmostTokenT->columnNum
                                < leftmostToken->columnNum))
        ) {
            leftmostToken = leftmostTokenT;
        }
    }
    if ( tree->a ) {
        leftmostTokenA = findLeftmostToken( tree->a );
        if (
            (leftmostTokenA->lineNum < leftmostToken->lineNum)
                || ((leftmostTokenA->lineNum == leftmostToken->lineNum)
                        && (leftmostTokenA->columnNum
                                < leftmostToken->columnNum))
        ) {
            leftmostToken = leftmostTokenA;
        }
    }
    if ( tree->b ) {
        leftmostTokenB = findLeftmostToken( tree->b );
        if (
            (leftmostTokenB->lineNum < leftmostToken->lineNum)
                || ((leftmostTokenB->lineNum == leftmostToken->lineNum)
                        && (leftmostTokenB->columnNum
                                < leftmostToken->columnNum))
        ) {
            leftmostToken = leftmostTokenB;
        }
    }
    return leftmostToken;

}

void markErrorAtLeftmostToken( tokenT *tree, const char *message )
{

    markErrorAtToken( findLeftmostToken( tree ), message );

}

static void writeNameToken( char *tokenCodeName, tokenT *tokenPtr )
{

    fputs( tokenCodeName, stdout );
    if ( tokenPtr->v.name.namePtr ) {
        printf( " \"%s\"", tokenPtr->v.name.namePtr->string );
        if ( tokenPtr->v.name.namePtr->tokenCode ) {
            printf( " (%d)", tokenPtr->v.name.namePtr->tokenCode );
        }
    }

}

#define CASETOKEN( name ) case OP_##name: tokenName = #name; break

static void writeTokenCode( tokenT *tokenPtr )
{
    int tokenCode;
    char * tokenName;
    bool writeM;

    tokenCode = tokenPtr->code;
    tokenName = "";
    writeM = false;
    switch ( tokenCode ) {
     case L_BINARY:
        tokenName = "_BINARY";
        writeM = true;
        break;
     case L_NUMBER:
        printf(
            "_NUMBER 0x%08lX%08lX (",
            (unsigned long) (tokenPtr->v.n>>32),
            (unsigned long) (uint32_t) tokenPtr->v.n
        );
        writeUint64AsText( stdout, tokenPtr->v.n );
        fputc( ')', stdout );
        break;
     case L_NAME:
        writeNameToken( "_NAME", tokenPtr );
        break;
     case L_DOTTEDNAME:
        writeNameToken( "_DOTTEDNAME", tokenPtr );
        break;
     CASETOKEN( VOID );
     case OP_ROW:
        tokenName = "ROW";
        writeM = true;
        break;
     CASETOKEN( LOGICBLOCK );
     CASETOKEN( CONTROLBLOCK );
     CASETOKEN( COLRANGE );
     case OP_ITEM:
     case OP_ITEMWITHARGS:
        fputs( (tokenCode == OP_ITEM) ? "ITEM" : "ITEMWITHARGS", stdout );
        if ( tokenPtr->v.name.namePtr ) {
            printf( " \"%s\"", tokenPtr->v.name.namePtr->string );
            if ( tokenPtr->v.name.namePtr->tokenCode ) {
                printf( " (%d)", tokenPtr->v.name.namePtr->tokenCode );
            }
        }
        break;
     case OP_NAMED_CONSTANT:
        writeNameToken( "NAMED_CONSTANT", tokenPtr );
        break;
     case OP_NAMED_COL:
        writeNameToken( "NAMED_COL", tokenPtr );
        break;
     case OP_NAMED_ROW:
        writeNameToken( "NAMED_ROW", tokenPtr );
        break;
     CASETOKEN( ZREG );
     CASETOKEN( DREG );
     case OP_ABOVEHORIZ_REL:
        tokenName = "ABOVEHORIZ_REL";
        writeM = true;
        break;
     case OP_ABOVEHORIZ_ABS:
        tokenName = "ABOVEHORIZ_ABS";
        writeM = true;
        break;
     case OP_ABOVEGLOBAL:
        tokenName = "ABOVEGLOBAL";
        writeM = true;
        break;
     case OP_HORIZ_REL:
        tokenName = "HORIZ_REL";
        writeM = true;
        break;
     case OP_HORIZ_ABS:
        tokenName = "HORIZ_ABS";
        writeM = true;
        break;
     case OP_GLOBAL:
        tokenName = "GLOBAL";
        writeM = true;
        break;
     CASETOKEN( LO );
     CASETOKEN( HI );
     CASETOKEN( SWAP );
     CASETOKEN( SHIFT1 );
     CASETOKEN( EITHER );
     CASETOKEN( A );
     CASETOKEN( B );
     CASETOKEN( C );
     CASETOKEN( D );
     CASETOKEN( ABOVEH );
     CASETOKEN( NOT );
     CASETOKEN( AND );
     CASETOKEN( XOR );
     CASETOKEN( OR );
     case OP_TIMES:
        tokenName = "TIMES";
        writeM = true;
        break;
     CASETOKEN( FUNCTION );
     CASETOKEN( FUNCTIONLO );
     CASETOKEN( FUNCTIONHI );
     CASETOKEN( TABLE );
     CASETOKEN( SELECT );
     CASETOKEN( PARTIALSELECT );
     CASETOKEN( VARIABLESHIFT );
     CASETOKEN( ADD3 );
     CASETOKEN( SUM );
     CASETOKEN( CARRY );
     CASETOKEN( U );
     CASETOKEN( V );
     CASETOKEN( K );
     CASETOKEN( RESULT );
     CASETOKEN( SHIFTZEROIN );
     CASETOKEN( BUFFERZ );
     CASETOKEN( BUFFERD );
     CASETOKEN( Z );
     CASETOKEN( HOUT );
     CASETOKEN( GOUT );
     CASETOKEN( LONG );
     CASETOKEN( VOUT );
     CASETOKEN( PROCESSOR );
     case OP_MATCH:
        tokenName = "MATCH";
        writeM = true;
        break;
     CASETOKEN( MEMORY );
     case OP_QUEUE:
        tokenName = "QUEUE";
        writeM = true;
        break;
     case OP_SIZE:
        tokenName = "SIZE";
        writeM = true;
        break;
     CASETOKEN( DEMAND );
     CASETOKEN( DEMANDREAD );
     case OP_LATENCY:
        tokenName = "LATENCY";
        writeM = true;
        break;
     case OP_ALLOCATE:
        tokenName = "ALLOCATE";
        writeM = true;
        break;
     case OP_ALIGNED:
        tokenName = "ALIGNED";
        writeM = true;
        break;
     case OP_BUS:
        tokenName = "BUS";
        writeM = true;
        break;
     CASETOKEN( REGBUS );
     case OP_HDIR:
        tokenName = "HDIR";
        writeM = true;
        break;
     default:
        tokenName = "unknown";
        break;
    }
    fputs( tokenName, stdout );
    if ( writeM ) printf( " %d", tokenPtr->v.m );
    if ( tokenPtr->ref ) printf( " ref:%p", (void *) tokenPtr->ref );
    printf( " <--%p\n", (void *) tokenPtr );

}

static void dumpTokenTreeZ( int indentation, char ch, tokenT *tree )
{
    int i;

    if ( tree ) {
        for ( i = 0; i < indentation; ++i ) {
            fputc( ((i & 3) == 3) ? '|' : ' ', stdout );
        }
        fputc( ch, stdout );
        fputc( ' ', stdout );
        writeTokenCode( tree );
        ++indentation;
        dumpTokenTreeZ( indentation, 'a', tree->a );
        dumpTokenTreeZ( indentation, 'b', tree->b );
        dumpTokenTreeZ( indentation, 't', tree->t );
    }

}

void dumpTokenTree( tokenT *tree )
{

    dumpTokenTreeZ( 0, ' ', tree );
    fflush( stdout );

}

