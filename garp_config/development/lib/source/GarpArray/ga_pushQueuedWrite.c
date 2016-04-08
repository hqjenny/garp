
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

int ga_pushQueuedWrite( GarpArrayT *GarpArrayPtr )
{
    int queueNum;
    queueT *queuePtr;
    uint32_t address;
    int count, outOffset, size, i;

    GarpArrayPtr->memoryRequest.enable = false;
    if ( ! GarpArrayPtr->configPtr ) return false;
    queuePtr = &GarpArrayPtr->queues[0];
    for ( queueNum = numQueues; queueNum; --queueNum ) {
        if (
            queuePtr->control.enable
                && (queuePtr->control.direction == ga_write)
                && queuePtr->count
        ) {
            address = queuePtr->address;
            count = queuePtr->count;
            GarpArrayPtr->memoryRequest.enable = true;
            GarpArrayPtr->memoryRequest.direction = ga_write;
            GarpArrayPtr->memoryRequest.attributes =
                queuePtr->control.attributes;
            GarpArrayPtr->memoryRequest.address = address;
            if ( ! (address & 31) && (32 <= count) ) {
                size = ga_32;
            } else if ( ! (address & 15) && (16 <= count) ) {
                size = ga_16;
            } else if ( ! (address & 7) && (8 <= count) ) {
                size = ga_8;
            } else if ( ! (address & 3) && (4 <= count) ) {
                size = ga_4;
            } else if ( ! (address & 1) && (2 <= count) ) {
                size = ga_2;
            } else {
                size = ga_1;
            }
            GarpArrayPtr->memoryRequest.size = size;
            size = 1<<size;
            queuePtr->address = address + size;
            queuePtr->count = count - size;
            outOffset = queuePtr->outOffset;
            if ( 4 <= size ) {
                i = 0;
                do {
                    GarpArrayPtr->memoryRequest.data[i++] =
                        *((uint32_t *) &queuePtr->data[outOffset]);
                    outOffset = (outOffset + 4) & (numBytesInQueue - 1);
                    size -= 4;
                } while ( size );
            } else {
                if ( size == 2 ) {
                    GarpArrayPtr->memoryRequest.data[0] =
                        *((uint16_t *) &queuePtr->data[outOffset]);
                } else {
                    GarpArrayPtr->memoryRequest.data[0] =
                        queuePtr->data[outOffset];
                }
                outOffset = (outOffset + size) & (numBytesInQueue - 1);
            }
            queuePtr->outOffset = outOffset;
            return true;
        }
        ++queuePtr;
    }
    return false;

}

