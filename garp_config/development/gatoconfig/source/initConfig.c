
#include <stdint.h>
#include "platform.h"
#include "allocate.h"
#include "tokens.h"
#include "wires.h"
#include "config.h"
#include "initConfig.h"

void initConfig( tokenT *tree )
{
    tokenT *rowTokenPtr;
    int rowNum;
    rowConfigT *rowConfigPtr;
    int i;

    rowNum = 0;
    for ( rowTokenPtr = tree; rowTokenPtr; rowTokenPtr = rowTokenPtr->t ) {
        rowTokenPtr->v.m = rowNum;
        ++rowNum;
    }
    numRows = rowNum;
    rowConfigPtrs = allocArrayType( rowConfigT *, numRows );
    fusedRowCounts = allocArrayType( int, numRows );
    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        rowConfigPtr = allocType( rowConfigT );
        rowConfigPtr->horizMin = 0;
        rowConfigPtr->horizMax = 0;
        rowConfigPtr->HWiresDirection = 0;
        rowConfigPtr->controlConfig = 0;
        rowConfigPtr->rowInputs = allocArrayType( uint32_t, numRows );
        for ( i = 0; i < numRows; ++i ) rowConfigPtr->rowInputs[i] = 0;
        for ( i = 0; i < NUMLOGICCOLS; ++i ) {
            rowConfigPtr->logicColHOutUsed[i] = false;
            rowConfigPtr->logicColVOutLong[i] = false;
            rowConfigPtr->logicColVOutWires[i] = 0;
            rowConfigPtr->logicColModes[i] = 0;
            rowConfigPtr->logicColConfigs[i] = 0;
        }
        rowConfigPtrs[rowNum] = rowConfigPtr;
        fusedRowCounts[rowNum] = 1;
    }

}

