
#ifndef allocate_h
#define allocate_h 1

#include <stddef.h>

void * ga_z_reallocate( void *, size_t );

#define allocType( type ) ((type *) ga_z_reallocate( 0, sizeof (type) ))
#define allocArrayType( type, numElems ) ((type *) ga_z_reallocate( 0, (numElems) * sizeof (type) ))

#endif

