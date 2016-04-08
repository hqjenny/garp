
#include <stdbool.h>
#include <string.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

void
 ga_setQueueControl(
     GarpArrayT *GarpArrayPtr,
     int queueNum,
     const ga_queueControlT *queueControlPtr
 )
{
    queueT *queuePtr;

    queuePtr = &GarpArrayPtr->queues[queueNum];
    if ( ! queueControlPtr ) {
        queuePtr->control.enable = false;
    } else {
        memcpy(
            &queuePtr->control, queueControlPtr, sizeof (ga_queueControlT) );
        queuePtr->control.attributes &= ga_allocate;
        queuePtr->address = queueControlPtr->address;
        queuePtr->count = 0;
        queuePtr->inOffset = queueControlPtr->address & 7;
        queuePtr->outOffset = queuePtr->inOffset;
    }

}

