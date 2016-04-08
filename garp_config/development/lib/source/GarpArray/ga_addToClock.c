
#include <stdint.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

void ga_addToClock( GarpArrayT *GarpArrayPtr, int_fast32_t clockIncrement )
{

    GarpArrayPtr->clock += clockIncrement;

}

