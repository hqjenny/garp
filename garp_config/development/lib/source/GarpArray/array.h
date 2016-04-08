
#ifndef GarpArray_array_h
#define GarpArray_array_h 1

#include <stdbool.h>
#include <stdint.h>

enum {
    numLogicCols = 23,
    numGWires = 4,
    numQueues = 3,
/*** SHOULD MATCH CACHE LINE SIZE. ***/
    queueRequestSize = 32,
    numBytesInQueue = 4 * queueRequestSize,  /* Must be a power of 2. */
    numReadBuffers = 12
};

enum {
    IN_CONSTANT,
    IN_ZREG,
    IN_DREG,
    IN_VWIRE,
    IN_VROW,
    IN_ABOVEHCOL,
    IN_ABOVEGWIRE,
    IN_HCOL,
    IN_GWIRE
};

enum {
    PERTURB_00,
    PERTURB_01,
    PERTURB_10,
    PERTURB_11,
    PERTURB_NOT,
    PERTURB_SHIFT1,
    PERTURB_SHIFT1_NOT
};

enum {
    MODE_PROCESSOR = 1,
    MODE_MEMORY_QUEUE,
    MODE_MEMORY_DEMAND
};

enum {
    MODE_DIRECT = 1,
    MODE_INPUT5,
    MODE_SPLIT,
    MODE_SELECT_1,
    MODE_SELECT,
    MODE_PARTSELECT,
    MODE_VARSHIFT,
    MODE_CARRY,
    MODE_ADD3
};

enum {
    RESULT_V = 1,
    RESULT_V_OR_K,
    RESULT_K_OUT,
    RESULT_U_XOR_K,
    RESULT_U_EQU_K
};

enum {
    SELECT_Z,
    SELECT_D
};

enum {
    HWIRES_LEFT = 0,
    HWIRES_CENTER,
    HWIRES_RIGHT
};

typedef struct {
    uint8_t off;
    uint8_t AIn, BIn, CIn, DIn;
    uint16_t AInNum, BInNum, CInNum, DInNum;
    uint8_t perturbA, perturbB, perturbC, perturbD;
    uint8_t mode;
    uint8_t attributes, requestSize, requestCount, latency;
    uint8_t queueNum;
    uint16_t matchCode;
    uint8_t dataSize, busSelect, busNum;
    uint8_t GOutNum;
    uint8_t HDir;
} controlBlockConfigT;

typedef struct {
    uint8_t off;
    uint8_t AIn, BIn, CIn, DIn;
    uint16_t AInNum, BInNum, CInNum, DInNum;
    uint8_t perturbA, perturbB, perturbC, perturbD;
    uint8_t mode, zeroIn;
    uint16_t table;
    uint8_t carryResult;
    uint8_t bufferZ, bufferD;
    uint8_t HSelect, GSelect, VSelect;
    uint8_t GOutNum;
    uint32_t temp0, temp1;
} logicBlockConfigT;

typedef struct {
    controlBlockConfigT controlCol;
    logicBlockConfigT logicCols[numLogicCols];
} rowConfigT;

typedef struct {
    ga_queueControlT control;
    uint32_t address;
    int count, inOffset, outOffset;
    uint32_t returnMainClockCycle[numBytesInQueue / 4];
    uint8_t data[numBytesInQueue];
} queueT;

typedef struct {
    int8_t enable, size, count, delay;
    uint32_t expectedMainClockCycle;
    uint32_t data[4];
} readBufferT;

typedef struct {
    uint8_t ZReg, DReg;
    uint8_t HOut, VOut;
} logicBlockStateT;

typedef struct {
    logicBlockStateT logicCols[numLogicCols];
    uint8_t GWires[numGWires];
    uint8_t controlEnable, controlB, controlC, controlD;
} rowStateT;

struct GarpArrayS {
    int numRows, configRowOffset, numConfigRows;
    rowConfigT *configPtr;
    bool branchFlag, interruptFlag;
    ga_memoryRequestT memoryRequest;
    queueT queues[numQueues];
    readBufferT readBuffers[numReadBuffers];
    int readStall;
    uint32_t addressBus, memoryBuses[4];
    rowStateT *phase0StatePtr, *phase1StatePtr;
    uint_fast32_t clock;
};

#endif

