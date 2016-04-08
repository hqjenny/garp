
#ifndef wires_h
#define wires_h 1

#include <stdint.h>

typedef struct wireS {
    char allocated;
    int nomLength, nomFirst, first, last;
} wireT;

typedef struct {
    int length;
    wireT *const wires;
} wireArrayElementT;

typedef wireArrayElementT * wireArrayT;

extern wireArrayT wireArray;

extern void *const useHWire;

void initWireArray( void );

int wireToVInOutCode( uint_fast16_t, wireT * );

#endif

