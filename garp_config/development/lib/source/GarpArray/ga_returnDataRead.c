
#include <stdint.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

void
 ga_returnDataRead(
     GarpArrayT *GarpArrayPtr,
     int source,
     int size,
     const uint32_t *dataPtr,
     uint32_t returnMainClockCycle
 )
{
    queueT *queuePtr;
    uint32_t address;
    int inOffset;
    uint32_t data;
    int_fast32_t i;
    readBufferT *readBufferPtr;

    if ( source < 0 ) {
        source = ~ source;
        if ( numQueues <= source ) return;
        queuePtr = &GarpArrayPtr->queues[source];
        if ( queuePtr->control.direction != ga_read ) return;
        size = 1<<size;
        queuePtr->address += size;
        queuePtr->count += size;
        inOffset = queuePtr->inOffset;
        if ( 4 <= size ) {
            i = 0;
            do {
                queuePtr->returnMainClockCycle[inOffset>>2] =
                    returnMainClockCycle;
                *((uint32_t *) &queuePtr->data[inOffset]) = dataPtr[i++];
                inOffset = (inOffset + 4) & (numBytesInQueue - 1);
                size -= 4;
            } while ( size );
        } else {
            queuePtr->returnMainClockCycle[inOffset>>2] = returnMainClockCycle;
            if ( size == 2 ) {
                *((uint16_t *) &queuePtr->data[inOffset]) = dataPtr[0];
            } else {
                queuePtr->data[inOffset] = dataPtr[0];
            }
            inOffset = (inOffset + size) & (numBytesInQueue - 1);
        }
        queuePtr->inOffset = inOffset;
    } else {
        if ( numReadBuffers <= source ) return;
        readBufferPtr = &GarpArrayPtr->readBuffers[source];
        i = returnMainClockCycle - readBufferPtr->expectedMainClockCycle;
        if ( 0 < i ) {
            if ( GarpArrayPtr->readStall < i ) GarpArrayPtr->readStall = i;
        }
        readBufferPtr->data[0] = dataPtr[0];
        switch ( size ) {
         case ga_8:
            readBufferPtr->data[1] = dataPtr[1];
            break;
         case ga_16:
            readBufferPtr->data[1] = dataPtr[1];
            readBufferPtr->data[2] = dataPtr[2];
            readBufferPtr->data[3] = dataPtr[3];
            break;
        }
    }

}

