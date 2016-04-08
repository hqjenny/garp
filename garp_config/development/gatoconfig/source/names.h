
#ifndef names_h
#define names_h 1

#include <stdint.h>

struct tokenS;

typedef struct nameS {
    uint16_t length;
    char *string;
    uint8_t tokenCode;
    int16_t m;
    struct tokenS *declaration;
} nameT;

nameT * stringToName( uint_fast16_t, char * );

#endif

