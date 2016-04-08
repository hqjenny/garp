
#include <string.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

void
 ga_getQueueControl(
     GarpArrayT *GarpArrayPtr, int queueNum, ga_queueControlT *queueControlPtr
 )
{

    if ( queueControlPtr ) {
        memcpy(
            queueControlPtr,
            &GarpArrayPtr->queues[queueNum].control,
            sizeof (ga_queueControlT)
        );
    }

}

