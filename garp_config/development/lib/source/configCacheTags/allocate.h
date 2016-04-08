
#ifndef configCacheTags_allocate_h
#define configCacheTags_allocate_h 1

#include <stddef.h>

void * configCacheTags_z_reallocate( void *, size_t );

#define allocType( type ) ((type *) configCacheTags_z_reallocate( 0, sizeof (type) ))
#define allocArrayType( type, numElems ) ((type *) configCacheTags_z_reallocate( 0, (numElems) * sizeof (type) ))

#endif

