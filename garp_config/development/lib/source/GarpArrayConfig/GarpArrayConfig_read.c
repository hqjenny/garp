
#include <stdint.h>
#include <stdio.h>
#include "platform.h"
#include "GarpArrayConfig.h"
#include "allocate.h"

enum { numCols = 24 };

uint32_t * GarpArrayConfig_read( FILE *streamPtr )
{
    int numRows;
    uint32_t *arrayConfigPtr, *p;
    int rowNum, colNum;
    long int a;

    fscanf( streamPtr, " {%i", &numRows );
    arrayConfigPtr = allocArrayType( uint32_t, 1 + numRows * numCols * 2 );
    *arrayConfigPtr = numRows;
    p = arrayConfigPtr + 1;
    for ( rowNum = numRows; rowNum; --rowNum ) {
        for ( colNum = 2 * numCols; colNum; --colNum ) {
            fscanf( streamPtr, " ,%li", &a );
            *(p++) = a;
        }
    }
    fscanf( streamPtr, " , }" );
    return arrayConfigPtr;

}

