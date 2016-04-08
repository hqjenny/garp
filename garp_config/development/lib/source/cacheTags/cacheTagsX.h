
#ifndef cacheTags_cacheTagsX_h
#define cacheTags_cacheTagsX_h 1

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t address;
    bool dirty;
    int prefetchBufferNum;
} cacheTagT;

typedef struct {
    int_fast32_t clockCycle;
    int_fast32_t tagNum;
} prefetchBufferT;

struct cacheTagsS {
    int_fast32_t numTags;
    int lineSize;
    int logLineSize;
    int readMissPenalty, readMissLatency, readMissBusy;
    int writeMissPenalty, writeMissBusy;
    int writeBackBusy;
    int_fast32_t lastBusyClockCycle;
    cacheTagT *tags;
    int numPrefetchBuffers;
    prefetchBufferT *prefetchBuffers;
};

uint_fast32_t
 cacheTags_z_serviceIncomingPrefetches( cacheTagsT *, uint_fast32_t );

#endif

