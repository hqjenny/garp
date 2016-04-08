
#include <stdbool.h>
#include "platform.h"
#include "tokens.h"
#include "tokenize.h"
#include "logicBlocks.h"
#include "config.h"

bool logicBlockCoversCol( tokenT *indexList, int colNum )
{

    while ( indexList ) {
        switch ( indexList->code ) {
         case L_NUMBER:
            if ( indexList->v.n == colNum ) return true;
            break;
         case OP_COLRANGE:
            if (
                (indexList->a->v.n <= colNum) && (colNum <= indexList->b->v.n)
            ) {
                return true;
            }
            break;
        }
        indexList = indexList->t;
    }
    return false;

}

int colOffsetInIndexList( tokenT *indexList, int colNum )
{
    bool colFlags[NUMLOGICCOLS];
    int i;

    for ( i = 0; i < NUMLOGICCOLS; ++i ) colFlags[i] = false;
    while ( indexList ) {
        switch ( indexList->code ) {
         case L_NUMBER:
            colFlags[indexList->v.n] = true;
            break;
         case OP_COLRANGE:
            for ( i = indexList->a->v.n; i <= indexList->b->v.n; ++i ) {
                colFlags[i] = true;
            }
            break;
        }
        indexList = indexList->t;
    }
    if ( ! colFlags[colNum] ) return -1;
    i = 0;
    while ( colNum ) {
        --colNum;
        if ( colFlags[colNum] ) ++i;
    }
    return i;

}

