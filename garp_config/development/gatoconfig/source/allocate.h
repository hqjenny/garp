
#ifndef allocate_h
#define allocate_h 1

#include <stddef.h>

void * reallocate( void *, size_t );

#define allocate( size ) reallocate( 0, size )
#define allocType( type ) ((type *) reallocate( 0, sizeof (type) ))
#define allocArrayType( type, numElems ) ((type *) reallocate( 0, (numElems) * sizeof (type) ))

#endif

