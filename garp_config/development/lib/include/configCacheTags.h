
#ifndef configCacheTags_h
#define configCacheTags_h 1

#include <stdint.h>

typedef struct configCacheTagsS configCacheTagsT;

configCacheTagsT * configCacheTags_new( int, uint_fast32_t, int );
void configCacheTags_delete( configCacheTagsT * );

void
 configCacheTags_allocRows(
     configCacheTagsT *, uint_fast32_t, uint_fast32_t, uint_fast32_t );
int
 configCacheTags_loadConfig(
     configCacheTagsT *, uint_fast32_t, uint_fast32_t, uint_fast32_t );

#endif

