
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "platform.h"
#include "allocate.h"
#include "dialect.h"
#include "tokens.h"
#include "names.h"

enum {
    nameHashSize = 4096                 /* Must be a power of 2.        */
};

typedef struct nameHashChainElementS {
    struct nameHashChainElementS *next;
    nameT *namePtr;
} nameHashChainElementT;

static nameHashChainElementT *nameHashTable[nameHashSize];

static nameT * newName( uint_fast16_t length, const char *string )
{
    nameT *namePtr;

    namePtr = allocType( nameT );
    namePtr->length = length; 
    namePtr->string = allocate( length + 1 );
    memcpy( namePtr->string, string, length );
    namePtr->string[length] = 0;
    namePtr->tokenCode = 0;
    namePtr->declaration = 0;
    return namePtr;

}

INLINE int stringToHashValue( int length, char *string )
{
    uint_fast32_t hashValue;

    hashValue = 0;
    while ( 0 < length ) {
        hashValue ^= (hashValue>>20) & 0xF;
        hashValue = (hashValue<<4) + *string ;
        ++string;
        --length;
    }
    return hashValue & (nameHashSize - 1);

}

nameT * stringToName( uint_fast16_t length, char *string )
{
    uint_fast16_t hashValue;
    nameHashChainElementT *chainElement;
    nameT *namePtr;

    hashValue = stringToHashValue( length, string );
    for (
        chainElement = nameHashTable[hashValue];
        chainElement;
        chainElement = chainElement->next
    ) {
        namePtr = chainElement->namePtr;
        if (
            (length == namePtr->length)
                && (memcmp( string, namePtr->string, length ) == 0)
        ) {
            return namePtr;
        }
    }
    namePtr = newName( length, string );
    chainElement = allocType( nameHashChainElementT );
    chainElement->next = nameHashTable[hashValue];
    chainElement->namePtr = namePtr;
    nameHashTable[hashValue] = chainElement;
    return namePtr;

}

static void initKeywordNameStruct( char *string, int tokenCode, int m )
{
    nameT *namePtr;

    namePtr = stringToName( strlen( string ), string );
    namePtr->tokenCode = tokenCode;
    namePtr->m = m;

}

void initNames( void )
{
    int i;

    for ( i = 0; i < nameHashSize; ++i ) nameHashTable[i] = 0;
    initKeywordNameStruct( "control", OP_CONTROLBLOCK, 0 );
    initKeywordNameStruct( "Zreg", OP_ZREG, 0 );
    initKeywordNameStruct( "Dreg", OP_DREG, 0 );
    initKeywordNameStruct( "aboveright9", OP_ABOVEHORIZ_REL, -9 );
    initKeywordNameStruct( "aboveright8", OP_ABOVEHORIZ_REL, -8 );
    initKeywordNameStruct( "aboveright7", OP_ABOVEHORIZ_REL, -7 );
    initKeywordNameStruct( "aboveright6", OP_ABOVEHORIZ_REL, -6 );
    initKeywordNameStruct( "aboveright5", OP_ABOVEHORIZ_REL, -5 );
    initKeywordNameStruct( "aboveright4", OP_ABOVEHORIZ_REL, -4 );
    initKeywordNameStruct( "aboveright3", OP_ABOVEHORIZ_REL, -3 );
    initKeywordNameStruct( "aboveright2", OP_ABOVEHORIZ_REL, -2 );
    initKeywordNameStruct( "aboveright1", OP_ABOVEHORIZ_REL, -1 );
    initKeywordNameStruct( "above", OP_ABOVEHORIZ_REL, 0 );
    initKeywordNameStruct( "aboveleft1", OP_ABOVEHORIZ_REL, 1 );
    initKeywordNameStruct( "aboveleft2", OP_ABOVEHORIZ_REL, 2 );
    initKeywordNameStruct( "aboveleft3", OP_ABOVEHORIZ_REL, 3 );
    initKeywordNameStruct( "aboveleft4", OP_ABOVEHORIZ_REL, 4 );
    initKeywordNameStruct( "aboveleft5", OP_ABOVEHORIZ_REL, 5 );
    initKeywordNameStruct( "aboveleft6", OP_ABOVEHORIZ_REL, 6 );
    initKeywordNameStruct( "aboveleft7", OP_ABOVEHORIZ_REL, 7 );
    initKeywordNameStruct( "aboveleft8", OP_ABOVEHORIZ_REL, 8 );
    initKeywordNameStruct( "aboveleft9", OP_ABOVEHORIZ_REL, 9 );
    initKeywordNameStruct( "aboveglobal0", OP_ABOVEGLOBAL, 0 );
    initKeywordNameStruct( "aboveglobal1", OP_ABOVEGLOBAL, 1 );
    initKeywordNameStruct( "aboveglobal2", OP_ABOVEGLOBAL, 2 );
    initKeywordNameStruct( "aboveglobal3", OP_ABOVEGLOBAL, 3 );
    initKeywordNameStruct( "right9", OP_HORIZ_REL, -9 );
    initKeywordNameStruct( "right8", OP_HORIZ_REL, -8 );
    initKeywordNameStruct( "right7", OP_HORIZ_REL, -7 );
    initKeywordNameStruct( "right6", OP_HORIZ_REL, -6 );
    initKeywordNameStruct( "right5", OP_HORIZ_REL, -5 );
    initKeywordNameStruct( "right4", OP_HORIZ_REL, -4 );
    initKeywordNameStruct( "right3", OP_HORIZ_REL, -3 );
    initKeywordNameStruct( "right2", OP_HORIZ_REL, -2 );
    initKeywordNameStruct( "right1", OP_HORIZ_REL, -1 );
    initKeywordNameStruct( "self", OP_HORIZ_REL, 0 );
    initKeywordNameStruct( "left1", OP_HORIZ_REL, 1 );
    initKeywordNameStruct( "left2", OP_HORIZ_REL, 2 );
    initKeywordNameStruct( "left3", OP_HORIZ_REL, 3 );
    initKeywordNameStruct( "left4", OP_HORIZ_REL, 4 );
    initKeywordNameStruct( "left5", OP_HORIZ_REL, 5 );
    initKeywordNameStruct( "left6", OP_HORIZ_REL, 6 );
    initKeywordNameStruct( "left7", OP_HORIZ_REL, 7 );
    initKeywordNameStruct( "left8", OP_HORIZ_REL, 8 );
    initKeywordNameStruct( "left9", OP_HORIZ_REL, 9 );
    initKeywordNameStruct( "global0", OP_GLOBAL, 0 );
    initKeywordNameStruct( "global1", OP_GLOBAL, 1 );
    initKeywordNameStruct( "global2", OP_GLOBAL, 2 );
    initKeywordNameStruct( "global3", OP_GLOBAL, 3 );
    initKeywordNameStruct( "lo", OP_LO, 0 );
    initKeywordNameStruct( "hi", OP_HI, 0 );
    initKeywordNameStruct( "swap", OP_SWAP, 0 );
    initKeywordNameStruct( "shift1", OP_SHIFT1, 0 );
    initKeywordNameStruct( "either", OP_EITHER, 0 );
    initKeywordNameStruct( "A", OP_A, 0 );
    initKeywordNameStruct( "B", OP_B, 0 );
    initKeywordNameStruct( "C", OP_C, 0 );
    initKeywordNameStruct( "D", OP_D, 0 );
    initKeywordNameStruct( "aboveH", OP_ABOVEH, 0 );
    initKeywordNameStruct( "function", OP_FUNCTION, 0 );
    initKeywordNameStruct( "functionlo", OP_FUNCTIONLO, 0 );
    initKeywordNameStruct( "functionhi", OP_FUNCTIONHI, 0 );
    initKeywordNameStruct( "table", OP_TABLE, 0 );
    initKeywordNameStruct( "select", OP_SELECT, 0 );
    initKeywordNameStruct( "partialselect", OP_PARTIALSELECT, 0 );
    if ( GarpVersion < 2 ) {
        initKeywordNameStruct( "variableshift", OP_VARIABLESHIFT, 0 );
    }
    initKeywordNameStruct( "add3", OP_ADD3, 0 );
    initKeywordNameStruct( "sum", OP_SUM, 0 );
    initKeywordNameStruct( "carry", OP_CARRY, 0 );
    initKeywordNameStruct( "U", OP_U, 0 );
    initKeywordNameStruct( "V", OP_V, 0 );
    initKeywordNameStruct( "K", OP_K, 0 );
    initKeywordNameStruct( "Kout", OP_KOUT, 0 );
    initKeywordNameStruct( "result", OP_RESULT, 0 );
    initKeywordNameStruct( "shiftzeroin", OP_SHIFTZEROIN, 0 );
    initKeywordNameStruct( "bufferZ", OP_BUFFERZ, 0 );
    initKeywordNameStruct( "bufferD", OP_BUFFERD, 0 );
    initKeywordNameStruct( "Z", OP_Z, 0 );
    initKeywordNameStruct( "Hout", OP_HOUT, 0 );
    initKeywordNameStruct( "Gout", OP_GOUT, 0 );
    initKeywordNameStruct( "long", OP_LONG, 0 );
    initKeywordNameStruct( "Vout", OP_VOUT, 0 );
    initKeywordNameStruct( "processor", OP_PROCESSOR, 0 );
    initKeywordNameStruct( "match", OP_MATCH, 0 );
    initKeywordNameStruct( "memory", OP_MEMORY, 0 );
    initKeywordNameStruct( "queue0", OP_QUEUE, 0 );
    initKeywordNameStruct( "queue1", OP_QUEUE, 1 );
    initKeywordNameStruct( "queue2", OP_QUEUE, 2 );
    initKeywordNameStruct( "size8", OP_SIZE, 0 );
    initKeywordNameStruct( "size16", OP_SIZE, 1 );
    initKeywordNameStruct( "size32", OP_SIZE, 2 );
    initKeywordNameStruct( "demand", OP_DEMAND, 0 );
    initKeywordNameStruct( "demandread", OP_DEMANDREAD, 0 );
    initKeywordNameStruct( "latency", OP_LATENCY, 0 );
    initKeywordNameStruct( "allocate", OP_ALLOCATE, true );
    initKeywordNameStruct( "noallocate", OP_ALLOCATE, false );
    initKeywordNameStruct( "aligned", OP_ALIGNED, true );
    initKeywordNameStruct( "nonaligned", OP_ALIGNED, false );
    initKeywordNameStruct( "bus0", OP_BUS, 0 );
    initKeywordNameStruct( "bus1", OP_BUS, 1 );
    initKeywordNameStruct( "bus2", OP_BUS, 2 );
    initKeywordNameStruct( "bus3", OP_BUS, 3 );
    initKeywordNameStruct( "regbus", OP_REGBUS, 0 );
    initKeywordNameStruct( "Hleft", OP_HDIR, 1 );
    initKeywordNameStruct( "Hcenter", OP_HDIR, 0 );
    initKeywordNameStruct( "Hright", OP_HDIR, -1 );

}

