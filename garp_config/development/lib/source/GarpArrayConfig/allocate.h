
#ifndef allocate_h
#define allocate_h 1

#include <stddef.h>

void * GarpArrayConfig_z_reallocate( void *, size_t );

#define allocArrayType( type, numElems ) ((type *) GarpArrayConfig_z_reallocate( 0, (numElems) * sizeof (type) ))

#endif

