
#include <stdint.h>
#include "platform.h"
#include "wires.h"
#include "config.h"

#include "wires-wireArray4"
#include "wires-wireArray8"
#include "wires-wireArray16"
#include "wires-wireArray32"
#include "wires-wireArray64"
#include "wires-wireArray128"
#include "wires-wireArray256"

wireArrayT wireArray;

void *const useHWire = (void *) 1;

void initWireArray( void )
{

    if ( numRows <= 4 ) {
        wireArray = wireArray4;
    } else if ( numRows <= 8 ) {
        wireArray = wireArray8;
    } else if ( numRows <= 16 ) {
        wireArray = wireArray16;
    } else if ( numRows <= 32 ) {
        wireArray = wireArray32;
    } else if ( numRows <= 64 ) {
        wireArray = wireArray64;
    } else if ( numRows <= 128 ) {
        wireArray = wireArray128;
    } else if ( numRows <= 256 ) {
        wireArray = wireArray256;
    }

}

int wireToVInOutCode( uint_fast16_t rowNum, wireT *wirePtr )
{
    uint_fast16_t nomLength, quarterNomLength, offset;
    int inOutCode;
    uint_fast16_t i;

    nomLength = wirePtr->nomLength;
    quarterNomLength = nomLength>>2;
    offset = rowNum - wirePtr->nomFirst;
    if ( (nomLength == 4) && ! (wirePtr->nomFirst & 1) ) {
        inOutCode = 0;
    } else {
        inOutCode = 2;
        i = quarterNomLength>>1;
        while ( i ) {
            inOutCode += 2;
            i >>= 1;
        }
    }
    inOutCode +=
        (quarterNomLength <= offset)
            && (offset < (nomLength - quarterNomLength));
    return 0x1F - inOutCode;

}

