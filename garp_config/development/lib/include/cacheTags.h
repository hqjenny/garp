
/*============================================================================
These functions provide an approximate simulation of a typical processor
cache.
*============================================================================*/

#ifndef cacheTags_h
#define cacheTags_h 1

#include <stdint.h>

typedef struct cacheTagsS cacheTagsT;

typedef struct {
    int size, lineSize;
    int readMissPenalty, readMissLatency, readMissBusy;
    int writeMissPenalty, writeMissBusy;
    int writeBackBusy;
    int numPrefetchBuffers;
} cacheTags_paramsT;

cacheTagsT * cacheTags_new( const cacheTags_paramsT * );
void cacheTags_delete( cacheTagsT * );

typedef struct {
    int stall, latency;
    int needWrite;
    uint_fast32_t writeAddress;
    int needRead;
} cacheTags_outT;

cacheTags_outT cacheTags_read( cacheTagsT *, uint_fast32_t, uint_fast32_t );
cacheTags_outT
 cacheTags_prefetch( cacheTagsT *, uint_fast32_t, uint_fast32_t );
void cacheTags_extendPrefetchLatency( cacheTagsT *, uint_fast32_t, int );
cacheTags_outT
 cacheTags_write( cacheTagsT *, uint_fast32_t, uint_fast32_t, int );

#endif

