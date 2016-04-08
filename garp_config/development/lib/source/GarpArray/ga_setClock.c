
#include <stdint.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

void ga_setClock( GarpArrayT *GarpArrayPtr, uint32_t clock )
{

    GarpArrayPtr->clock = clock;

}

