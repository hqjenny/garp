
#ifndef cacheTags_allocate_h
#define cacheTags_allocate_h 1

#include <stddef.h>

void * cacheTags_z_reallocate( void *, size_t );

#define allocType( type ) ((type *) cacheTags_z_reallocate( 0, sizeof (type) ))
#define allocArrayType( type, numElems ) ((type *) cacheTags_z_reallocate( 0, (numElems) * sizeof (type) ))

#endif

