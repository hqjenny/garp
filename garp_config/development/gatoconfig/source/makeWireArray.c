
#include <stdlib.h>
#include <stdio.h>
#include "platform.h"

typedef struct wireS {
    struct wireS *next;
    int nomLength, nomFirst, first, last;
} wireT;

static int numRows;
static wireT **wires;

static void addWire( int nomLength, int nomFirst )
{
    wireT *wire;
    int length, first, last;

    first = nomFirst < 0 ? 0 : nomFirst;
    last = nomFirst + nomLength - 1;
    if ( numRows <= last ) last = numRows - 1;
    length = last - first + 1;
    wire = malloc( sizeof (wireT) );
    wire->next = wires[length];
    wire->nomLength = nomLength;
    wire->nomFirst = nomFirst;
    wire->first = first;
    wire->last = last;
    wires[length] = wire;

}

static void writeWires( int length )
{
    wireT *wire;

    printf( "static wireT wires%dLength%d[] = {\n", numRows, length );
    wire = wires[length];
    while ( wire ) {
        printf(
            "    { 0, %d, %d, %d, %d },\n",
            wire->nomLength,
            wire->nomFirst,
            wire->first,
            wire->last
        );
        wire = wire->next;
    }
    puts( "    { 0, 0, 0, 0, 0 }\n};" );

}

int main( int argc, char **argv )
{
    int nomLength, nomFirst, length, numInfinityWires, halfNomLength, i;

    if ( argc != 2 ) return EXIT_FAILURE;
    numRows = atoi( argv[1] );

    wires = malloc( sizeof (wireT *) * (numRows + 1) );
    for ( length = 0; length < numRows; ++length ) wires[length] = 0;

    numInfinityWires = 0;
    for ( i = 8; i < numRows; i += i ) ++numInfinityWires;
    nomLength = numRows + numRows;
    for ( i = numInfinityWires; 0 < i; i -= 2 ) nomLength += nomLength;
    if ( numInfinityWires & 1 ) {
        addWire( nomLength, -nomLength + (nomLength>>2) );
        nomLength >>= 1;
    }
    for ( i = numInfinityWires & ~1; 0 < i; i -= 2 ) {
        addWire( nomLength, -(nomLength>>2) );
        addWire( nomLength, -nomLength + (nomLength>>2) );
        nomLength >>= 1;
    }

    for ( nomLength = numRows + numRows; 8 <= nomLength; nomLength >>= 1 ) {
        halfNomLength = nomLength>>1;
        for (
            nomFirst = -nomLength + (nomLength>>2);
            nomFirst < numRows;
            nomFirst += halfNomLength
        ) {
            addWire( nomLength, nomFirst );
        }
    }

    for ( nomFirst = -2; nomFirst <= numRows - 2; ++nomFirst ) {
        addWire( 4, nomFirst );
    }

    for ( i = 2; i < numRows; i += i ) {
        writeWires( i );
        writeWires( i + (i>>1) );
    }
    writeWires( numRows );

    printf( "static wireArrayElementT wireArray%d[] = {\n", numRows );
    for ( i = 2; i < numRows; i += i ) {
        printf(
            "    { %d, wires%dLength%d },\n    { %d, wires%dLength%d },\n",
            i,
            numRows,
            i,
            i + (i>>1),
            numRows,
            i + (i>>1)
        );
    }
    printf(
        "    { %d, wires%dLength%d },\n    { 0, 0 }\n",
        numRows,
        numRows,
        numRows
    );
    puts( "};" );
    return 0;

}

