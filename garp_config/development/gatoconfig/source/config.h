
#ifndef config_h
#define config_h 1

#include <stdint.h>

struct wireS;

extern int GarpVersion;

enum {
    NUMLOGICCOLS = 23
};

enum {
    HWIRES_CENTER = 1,
    HWIRES_LEFT,
    HWIRES_RIGHT
};

enum {
    MODE_DIRECT = 1,
    MODE_INPUT5,
    MODE_SPLIT,
    MODE_SELECT,
    MODE_PPSELECT,
    MODE_VARSHIFT,
    MODE_CARRY,
    MODE_ADD3
};

typedef struct {
    int horizMin, horizMax;
    int HWiresDirection;
    uint64_t controlConfig;
    uint32_t *rowInputs;
    char logicColHOutUsed[NUMLOGICCOLS];
    char logicColVOutLong[NUMLOGICCOLS];
    struct wireS *logicColVOutWires[NUMLOGICCOLS];
    int8_t logicColModes[NUMLOGICCOLS];
    uint64_t logicColConfigs[NUMLOGICCOLS];
} rowConfigT;

extern int numRows;
extern rowConfigT **rowConfigPtrs;
extern int *fusedRowCounts;

void dumpConfig( void );

#endif

