
#ifndef tokens_h
#define tokens_h 1

#include <stdint.h>
#include "errorMessages.h"

struct nameS;

enum {
    OP_VOID = 1,
    OP_ROW,
    OP_LOGICBLOCK,
    OP_CONTROLBLOCK,
    OP_COLRANGE,
    OP_ITEM,			/* Removed by `fixSyntax'.		*/
    OP_ITEMWITHARGS,		/* Removed by `fixSyntax'.		*/
    OP_NAMED_CONSTANT,
    OP_NAMED_COL,
    OP_NAMED_ROW,
    OP_ZREG,
    OP_DREG,
    OP_ABOVEHORIZ_REL,
    OP_ABOVEHORIZ_ABS,
    OP_ABOVEGLOBAL,
    OP_HORIZ_REL,
    OP_HORIZ_ABS,
    OP_GLOBAL,
    OP_LO,
    OP_HI,
    OP_SWAP,
    OP_SHIFT1,
    OP_EITHER,
    OP_A,
    OP_B,
    OP_C,
    OP_D,
    OP_ABOVEH,
    OP_NOT,
    OP_AND,
    OP_XOR,
    OP_OR,
    OP_TIMES,
    OP_FUNCTION,
    OP_FUNCTIONLO,
    OP_FUNCTIONHI,
    OP_TABLE,
    OP_SELECT,
    OP_PARTIALSELECT,
    OP_VARIABLESHIFT,
    OP_ADD3,
    OP_SUM,
    OP_CARRY,
    OP_U,
    OP_V,
    OP_K,
    OP_KOUT,
    OP_RESULT,
    OP_SHIFTZEROIN,
    OP_BUFFERZ,
    OP_BUFFERD,
    OP_Z,
    OP_HOUT,
    OP_GOUT,
    OP_LONG,
    OP_VOUT,
    OP_PROCESSOR,
    OP_MATCH,
    OP_MEMORY,
    OP_QUEUE,
    OP_SIZE,
    OP_DEMAND,
    OP_DEMANDREAD,
    OP_LATENCY,
    OP_ALLOCATE,
    OP_ALIGNED,
    OP_BUS,
    OP_REGBUS,
    OP_HDIR
};

typedef struct tokenS {
    uint32_t lineNum;
    uint16_t columnNum;
    uint16_t code;
    union {
        uint64_t n;
        struct {
            struct nameS *namePtr;
            uint8_t status;
        } name;
        int16_t m;
    } v;
    struct tokenS *ref;
    struct tokenS *t, *a, *b;
} tokenT;

tokenT * newTokenZ( void );

INLINE tokenT * newToken( uint_fast16_t tokenCode )
{
    tokenT *token = newTokenZ();
    token->code = tokenCode;
    return token;
}

void freshenToken( tokenT *, uint_fast16_t );

INLINE tokenT * renameToken( tokenT *tokenPtr, uint_fast16_t tokenCode )
{
    tokenPtr->code = tokenCode;
    return tokenPtr;
}

INLINE tokenT * copyTokenPosition( tokenT *tokenPtrA, tokenT *tokenPtrB )
{
    tokenPtrA->lineNum = tokenPtrB->lineNum;
    tokenPtrA->columnNum = tokenPtrB->columnNum;
    return tokenPtrA;
}

tokenT * newCopyToken( tokenT * );

tokenT * newCopyTokens( tokenT * );

void freeToken( tokenT * );

void freeTokens( tokenT * );

INLINE tokenT * linkTokensT( tokenT *parentTokenPtr, tokenT *tTokenPtr )
{
    parentTokenPtr->t = tTokenPtr;
    return parentTokenPtr;
}

INLINE
 tokenT *
  linkTokensTA( tokenT *parentTokenPtr, tokenT *tTokenPtr, tokenT *aTokenPtr )
{
    parentTokenPtr->t = tTokenPtr;
    parentTokenPtr->a = aTokenPtr;
    return parentTokenPtr;
}

INLINE tokenT * linkTokensA( tokenT *parentTokenPtr, tokenT *aTokenPtr )
{
    parentTokenPtr->a = aTokenPtr;
    return parentTokenPtr;
}

INLINE tokenT * linkTokensB( tokenT *parentTokenPtr, tokenT *bTokenPtr )
{
    parentTokenPtr->b = bTokenPtr;
    return parentTokenPtr;
}

INLINE
 tokenT *
  linkTokensAB( tokenT *parentTokenPtr, tokenT *aTokenPtr, tokenT *bTokenPtr )
{
    parentTokenPtr->a = aTokenPtr;
    parentTokenPtr->b = bTokenPtr;
    return parentTokenPtr;
}

void insertTokenA( tokenT *, uint_fast16_t );

void removeTokenA( tokenT * );

INLINE void markErrorAtToken( tokenT *tokenPtr, const char *message )
{
    markError2InColumn(
        tokenPtr->lineNum, tokenPtr->columnNum, message, 0, -1 );
}

INLINE
 void
  markError2AtToken(
      tokenT *tokenPtr, const char *message, const char *insideMessage )
{
    markError2InColumn(
        tokenPtr->lineNum, tokenPtr->columnNum, message, insideMessage, -1 );
}

INLINE
 void
  markErrorInColumnAtToken( tokenT *tokenPtr, const char *message, int colNum )
{
    markError2InColumn(
        tokenPtr->lineNum, tokenPtr->columnNum, message, 0, colNum );
}

void markErrorAtLeftmostToken( tokenT *, const char * );

void dumpTokenTree( tokenT * );

#endif

