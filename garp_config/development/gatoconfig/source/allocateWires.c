
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "platform.h"
#include "allocate.h"
#include "wires.h"
#include "config.h"
#include "allocateWires.h"

typedef struct {
    bool wireNeeded;
    uint_fast16_t first, last;
    bool inputsSelf;
} wireNeedT;
static wireNeedT *wireNeeds;
static uint_fast32_t *cumulativeWirePressures;

static void clearWireAllocation( int colNum )
{
    uint_fast16_t rowNum;
    wireNeedT *wireNeedPtr;
    wireArrayElementT *wireArrayElementPtr;
    wireT *wirePtr;

    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        rowConfigPtrs[rowNum]->logicColVOutWires[colNum] = 0;
        wireNeedPtr = &(wireNeeds[rowNum]);
        wireNeedPtr->wireNeeded = false;
        wireNeedPtr->first = rowNum;
        wireNeedPtr->last = rowNum;
        wireNeedPtr->inputsSelf = false;
    }
    for (
        wireArrayElementPtr = wireArray;
        wireArrayElementPtr->length;
        ++wireArrayElementPtr
    ) {
        for (
            wirePtr = wireArrayElementPtr->wires; wirePtr->nomLength; ++wirePtr
        ) {
            wirePtr->allocated = false;
        }
    }

}

static void findWireNeeds( int colNum )
{
    uint_fast16_t rowNum;
    rowConfigT *rowConfigPtr;
    uint32_t inputMask;
    uint_fast16_t inputRowNum;
    wireNeedT *wireNeedPtr;

    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        rowConfigPtr = rowConfigPtrs[rowNum];
        inputMask = 1<<colNum;
        for ( inputRowNum = 0; inputRowNum < numRows; ++inputRowNum ) {
            if ( rowConfigPtr->rowInputs[inputRowNum] & inputMask ) {
                wireNeedPtr = &(wireNeeds[inputRowNum]);
                wireNeedPtr->wireNeeded = true;
                if ( rowNum < wireNeedPtr->first ) wireNeedPtr->first = rowNum;
                if ( wireNeedPtr->last < rowNum ) wireNeedPtr->last = rowNum;
                if ( inputRowNum == rowNum ) wireNeedPtr->inputsSelf = true;
            }
        }
    }

}

static void calculateShortWirePressures( int colNum )
{
    uint_fast16_t rowNum;
    wireNeedT *wireNeedPtr;
    uint_fast16_t last, localWirePressure;
    uint_fast32_t cumulativeWirePressure;
    uint_fast16_t i;

    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        cumulativeWirePressures[rowNum] = 0;
    }
    cumulativeWirePressures[numRows] = 0;
    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        wireNeedPtr = &(wireNeeds[rowNum]);
        if (
            wireNeedPtr->wireNeeded
                && ! rowConfigPtrs[rowNum]->logicColVOutLong[colNum]
        ) {
            last = wireNeedPtr->last;
            for ( i = wireNeedPtr->first; i <= last; ++i ) {
                ++cumulativeWirePressures[i];
            }
        }
    }
    cumulativeWirePressure = 0;
    for ( rowNum = 0; rowNum <= numRows; ++rowNum ) {
        localWirePressure = cumulativeWirePressures[rowNum];
        cumulativeWirePressures[rowNum] = cumulativeWirePressure;
        cumulativeWirePressure += localWirePressure;
    }

}

static void calculateLongWirePressures( int colNum )
{
    uint_fast16_t rowNum;
    wireNeedT *wireNeedPtr;
    uint_fast16_t last, localWirePressure;
    uint_fast32_t cumulativeWirePressure;
    uint_fast16_t i;

    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        cumulativeWirePressures[rowNum] = 0;
    }
    cumulativeWirePressures[numRows] = 0;
    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        wireNeedPtr = &(wireNeeds[rowNum]);
        if (
            wireNeedPtr->wireNeeded
                && rowConfigPtrs[rowNum]->logicColVOutLong[colNum]
        ) {
            last = wireNeedPtr->last;
            for ( i = wireNeedPtr->first; i <= last; ++i ) {
                ++cumulativeWirePressures[i];
            }
        }
    }
    cumulativeWirePressure = 0;
    for ( rowNum = 0; rowNum <= numRows; ++rowNum ) {
        localWirePressure = cumulativeWirePressures[rowNum];
        cumulativeWirePressures[rowNum] = cumulativeWirePressure;
        cumulativeWirePressure += localWirePressure;
    }

}

void dumpWireNeedsInCol( int colNum )
{
    int rowNum;
    wireNeedT *wireNeedPtr;

    printf( "wireNeeds col %2d:\n", colNum );
    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        wireNeedPtr = &(wireNeeds[rowNum]);
        printf(
          "  %2d: { wireNeeded: %d; first: %2u; last: %2u, inputsSelf: %d }\n",
            rowNum,
            (int) wireNeedPtr->wireNeeded,
            (unsigned int) wireNeedPtr->first,
            (unsigned int) wireNeedPtr->last,
            (int) wireNeedPtr->inputsSelf
        );
    }

}

void dumpWireNeeds( void )
{
    int colNum, rowNum;
    wireNeedT *wireNeedPtr;

    if ( wireNeeds == 0 ) {
        wireNeeds = allocArrayType( wireNeedT, numRows );
        cumulativeWirePressures = allocArrayType( uint_fast32_t, numRows + 1 );
    }
    for ( colNum = 0; colNum < NUMLOGICCOLS; ++colNum ) {
        clearWireAllocation( colNum );
        findWireNeeds( colNum );
        dumpWireNeedsInCol( colNum );
    }

}

bool
 allocateWire(
     int colNum,
     uint_fast16_t rowNum,
     uint_fast16_t first,
     uint_fast16_t last,
     uint_fast16_t maxLength
 )
{
    uint_fast16_t length;
    wireArrayElementT *wireArrayElementPtr;
    wireT *bestWirePtr, *wirePtr;
    uint_fast16_t wireLast;
    uint_fast32_t bestWirePressure, wirePressure;
    uint_fast16_t i;

    length = last - first + 1;
    for (
        wireArrayElementPtr = wireArray;
        wireArrayElementPtr->length < length;
        ++wireArrayElementPtr
    ) {}
    bestWirePtr = 0;
    for ( ; wireArrayElementPtr->length; ++wireArrayElementPtr ) {
        for (
            wirePtr = wireArrayElementPtr->wires; wirePtr->nomLength; ++wirePtr
        ) {
            if (
                ! wirePtr->allocated
                    && (! maxLength || (wirePtr->nomLength <= maxLength))
                    && (wirePtr->first <= first) && (last <= wirePtr->last)
            ) {
                wireLast = wirePtr->last + 1;
                if ( numRows < wireLast ) wireLast = numRows;
                wirePressure =
                    cumulativeWirePressures[wireLast]
                        - cumulativeWirePressures[wirePtr->first];
                if (
                    (bestWirePtr == 0) || (wirePressure < bestWirePressure)
                ) {
                    bestWirePtr = wirePtr;
                    bestWirePressure = wirePressure;
                }
            }
        }
    }
    if ( bestWirePtr == 0 ) return false;
    bestWirePtr->allocated = true;
    rowConfigPtrs[rowNum]->logicColVOutWires[colNum] = bestWirePtr;
    wirePressure = 0;
    for ( i = first; i <= last; ++i ) {
        ++wirePressure;
        cumulativeWirePressures[i + 1] -= wirePressure;
    }
    for ( ; i < numRows; ++i ) {
        cumulativeWirePressures[i + 1] -= wirePressure;
    }
    return true;

}

bool allocateWires( void )
{
    int colNum;
    uint_fast16_t length, rowNum;
    wireNeedT *wireNeedPtr;
    uint_fast16_t first, last;
    uint_fast16_t i;

    if ( wireNeeds == 0 ) {
        wireNeeds = allocArrayType( wireNeedT, numRows );
        cumulativeWirePressures = allocArrayType( uint_fast32_t, numRows + 1 );
    }
    for ( colNum = 0; colNum < NUMLOGICCOLS; ++colNum ) {
        clearWireAllocation( colNum );
        findWireNeeds( colNum );
        for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
            wireNeedPtr = &(wireNeeds[rowNum]);
            if (
                   wireNeedPtr->wireNeeded
                && (wireNeedPtr->first == rowNum)
                && (wireNeedPtr->last == rowNum + 1)
                && ! wireNeedPtr->inputsSelf
                && ! rowConfigPtrs[rowNum]->logicColHOutUsed[colNum]
            ) {
                wireNeedPtr->wireNeeded = false;
                rowConfigPtrs[rowNum]->logicColVOutWires[colNum] = useHWire;
            }
        }
        calculateShortWirePressures( colNum );
        for ( length = 4; 1 <= length; --length ) {
            for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
                wireNeedPtr = &(wireNeeds[rowNum]);
                if ( wireNeedPtr->wireNeeded ) {
                    first = wireNeedPtr->first;
                    last = wireNeedPtr->last;
                    if (
                        (last - first + 1 == length)
                            && ! rowConfigPtrs[rowNum]
                                     ->logicColVOutLong[colNum]
                    ) {
                        if ( allocateWire( colNum, rowNum, first, last, 4 ) ) {
                            wireNeedPtr->wireNeeded = false;
                        }
                    }
                }
            }
        }
        for ( length = numRows; 1 <= length; --length ) {
            for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
                wireNeedPtr = &(wireNeeds[rowNum]);
                if ( wireNeedPtr->wireNeeded ) {
                    first = wireNeedPtr->first;
                    last = wireNeedPtr->last;
                    if (
                        (last - first + 1 == length)
                            && ! rowConfigPtrs[rowNum]
                                     ->logicColVOutLong[colNum]
                    ) {
                        if (
                            ! allocateWire( colNum, rowNum, first, last, 8 )
                        ) {
                            goto unableToAllocate;
                        }
                        wireNeedPtr->wireNeeded = false;
                    }
                }
            }
        }
        calculateLongWirePressures( colNum );
        for ( length = numRows; 1 <= length; --length ) {
            for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
                wireNeedPtr = &(wireNeeds[rowNum]);
                if ( wireNeedPtr->wireNeeded ) {
                    first = wireNeedPtr->first;
                    last = wireNeedPtr->last;
                    if ( last - first + 1 == length ) {
                        if (
                            ! allocateWire( colNum, rowNum, first, last, 0 )
                        ) {
                            goto unableToAllocate;
                        }
                        wireNeedPtr->wireNeeded = false;
                    }
                }
            }
        }
    }
    return true;
 unableToAllocate:
    printf(
        "Unable to allocate a vertical wire for block in row %u column %d.\n",
        (unsigned int) rowNum,
        colNum
    );
    dumpWireNeedsInCol( colNum );
    return false;

}

