
#include <stddef.h>
#include <stdlib.h>
#include "platform.h"
#include "fail.h"
#include "allocate.h"

void * reallocate( void *ptr, size_t size )
{

    ptr = realloc( ptr, size );
    if ( ! ptr ) fail( "Out of memory.", 0 );
    return ptr;

}

