
#include <stdlib.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

void ga_delete( GarpArrayT *GarpArrayPtr )
{

    if ( GarpArrayPtr->configPtr ) free( GarpArrayPtr->configPtr );
    if ( GarpArrayPtr->phase0StatePtr ) free( GarpArrayPtr->phase0StatePtr );
    if ( GarpArrayPtr->phase1StatePtr ) free( GarpArrayPtr->phase1StatePtr );
    free( GarpArrayPtr );

}

