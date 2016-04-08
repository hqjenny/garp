
#include <stdbool.h>
#include <string.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

void
 ga_getMemoryRequest(
     GarpArrayT *GarpArrayPtr, ga_memoryRequestT *memoryRequestPtr )
{

    if ( GarpArrayPtr->memoryRequest.enable ) {
        memcpy(
            memoryRequestPtr,
            &GarpArrayPtr->memoryRequest,
            sizeof (ga_memoryRequestT)
        );
    } else {
        memoryRequestPtr->enable = false;
    }

}

