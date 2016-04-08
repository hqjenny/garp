
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

void ga_flushQueuedReads( GarpArrayT *GarpArrayPtr )
{
    queueT *queuePtr;
    int i;

    queuePtr = &GarpArrayPtr->queues[0];
    for ( i = numQueues; i; --i ) {
        if (
            queuePtr->control.enable
                && (queuePtr->control.direction == ga_read)
        ) {
            queuePtr->address = queuePtr->control.address;
            queuePtr->count = 0;
            queuePtr->inOffset = queuePtr->outOffset;
        }
        ++queuePtr;
    }

}

