
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "platform.h"
#include "allocate.h"

extern char *fail_programName;

void * GarpArrayConfig_z_reallocate( void *ptr, size_t size )
{

    ptr = realloc( ptr, size );
    if ( ! ptr ) {
        if ( fail_programName ) {
            fputs( fail_programName, stderr );
            fputs( ": ", stderr );
        }
        fputs( "Out of memory.\n", stderr );
        exit( EXIT_FAILURE );
    }
    return ptr;

}

