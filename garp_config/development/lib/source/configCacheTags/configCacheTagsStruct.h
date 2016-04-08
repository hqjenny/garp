
#ifndef configCacheTags_configCacheTagsStruct_h
#define configCacheTags_configCacheTagsStruct_h 1

#include <stdint.h>

struct configCacheTagsS {
    int numContexts;
    uint_fast32_t numRows;
    int minAllocationAlignment;
    uint_fast32_t allocationStartRowNum, allocationEndRowNum;
    uint32_t *rowAddresses;        /* Bit 0 denotes extension of prev. row. */
    uint_fast32_t loadConfigCount;
    uint32_t *rowLastLoads;
};

#endif

