
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "GarpArray.h"
#include "array.h"

static
 int
  getInput(
      int rowNum,
      int colNum,
      int configIn,
      int configInNum,
      rowStateT *sourceArrayStatePtr,
      logicBlockStateT *sourceLogicBlockStatePtr
  )
{

    switch ( configIn ) {
     case IN_CONSTANT:
        return configInNum;
     case IN_ZREG:
        return sourceLogicBlockStatePtr->ZReg;
     case IN_DREG:
        return sourceLogicBlockStatePtr->DReg;
     case IN_VWIRE:
        return 0;
     case IN_VROW:
        return sourceArrayStatePtr[configInNum].logicCols[colNum].VOut;
     case IN_ABOVEHCOL:
        if ( ! rowNum ) return 0;
        return sourceArrayStatePtr[rowNum - 1].logicCols[configInNum].HOut;
     case IN_ABOVEGWIRE:
        if ( ! rowNum ) return 0;
        return sourceArrayStatePtr[rowNum - 1].GWires[configInNum];
     case IN_HCOL:
        return sourceArrayStatePtr[rowNum].logicCols[configInNum].HOut;
     case IN_GWIRE:
        return sourceArrayStatePtr[rowNum].GWires[configInNum];
    }

}

static int perturbInput( int configPerturb, int input, int inputOnRight )
{
    int z;

    switch ( configPerturb ) {
     case PERTURB_00:
        z = input & 1;
        return z + z + z;
     case PERTURB_01:
        return ((input<<1) & 2) | ((input>>1) & 1);
     case PERTURB_10:
        return input;
     case PERTURB_11:
        z = (input>>1) & 1;
        return z + z + z;
     case PERTURB_NOT:
        return 3 - input;
     case PERTURB_SHIFT1:
        return ((input<<1) & 2) | ((inputOnRight>>1) & 1);
     case PERTURB_SHIFT1_NOT:
        return 3 - (((input<<1) & 2) | ((inputOnRight>>1) & 1));
    }

}

static
 void
  computeZ(
      GarpArrayT *GarpArrayPtr,
      int rowNum,
      int colNum,
      logicBlockConfigT *logicBlockConfigPtr,
      int A,
      int B,
      int C,
      int D,
      int shiftCarryIn,
      int carryIn,
      int *shiftCarryOutPtr,
      int *carryOutPtr,
      rowStateT *sourceArrayStatePtr,
      int *ZPtr
  )
{
    unsigned int table;
    rowStateT *rowStatePtr;
    int U, V, K, Z, carry, i;

    table = logicBlockConfigPtr->table;
    Z = 0;
    switch ( logicBlockConfigPtr->mode ) {
     case MODE_INPUT5:
        if ( ! rowNum ) goto exit;
        D = sourceArrayStatePtr[rowNum - 1].logicCols[colNum].HOut;
        goto directMode;
     case MODE_DIRECT:
     directMode:
        i = ((D & 1)<<3) | ((C & 1)<<2) | ((B & 1)<<1) | (A & 1);
        Z = (table>>i) & 1;
        i = ((D & 2)<<2) | ((C & 2)<<1) | (B & 2) | (A>>1);
        i = (table>>i) & 1;
        Z |= i + i;
        break;
     case MODE_SPLIT:
        i = ((C & 1)<<2) | ((B & 1)<<1) | (A & 1);
        Z = (table>>i) & 1;
        i = ((C & 2)<<1) | (B & 2) | (A>>1);
        Z |= (table>>(i + 7)) & 2;
        break;
     case MODE_SELECT_1:
        switch ( C ) {
         case 0:
            Z = A;
            break;
         case 1:
            Z = B;
            break;
         case 2:
            if ( ! rowNum ) goto exit;
            Z = sourceArrayStatePtr[rowNum - 1].logicCols[colNum].HOut;
            break;
         case 3:
            Z = D;
            break;
        }
        break;
     case MODE_SELECT:
        switch ( C ) {
         case 0:
            Z = A;
            break;
         case 1:
            Z = B;
            break;
         case 2:
            Z = D;
            break;
         case 3:
            if ( ! rowNum ) goto exit;
            Z = sourceArrayStatePtr[rowNum - 1].logicCols[colNum].HOut;
            break;
        }
        break;
     case MODE_PARTSELECT:
        switch ( C ) {
         case 0:
            Z = A;
            break;
         case 1:
            Z = B;
            break;
         case 2:
            Z = D; /* actually BIn */
            break;
         case 3:
            Z = 0;
            break;
        }
        break;
     case MODE_VARSHIFT:
        if ( ! rowNum ) goto exit;
        i = colNum;
        if ( B | C ) {
            i -= (B<<1) | (C>>1);
            switch ( GarpArrayPtr->configPtr[rowNum - 1].controlCol.HDir ) {
             case HWIRES_CENTER:
                i += 4;
                break;
             case HWIRES_RIGHT:
                i += 8;
                break;
            }
        }
        rowStatePtr = &sourceArrayStatePtr[rowNum - 1];
        if ( C & 1 ) {
            if ( (0 <= i) && (i <= numLogicCols) ) {
                if ( 0 < i ) Z = rowStatePtr->logicCols[i - 1].HOut>>1;
                if ( i < numLogicCols ) {
                    Z |= (rowStatePtr->logicCols[i].HOut<<1) & 2;
                }
            }
        } else {
            if ( (0 <= i) && (i < numLogicCols) ) {
                Z = rowStatePtr->logicCols[i].HOut;
            }
        }
        break;
     case MODE_ADD3:
        U = (A & B) | (A & C) | (B & C);
        *shiftCarryOutPtr = U>>1;
        U = ((U & 1)<<1) | shiftCarryIn;
        V = A ^ B ^ C;
        A = U;
        B = V;
        C = 0;
        goto carryMode;
     case MODE_CARRY:
     carryMode:
        i = ((C & 1)<<2) | ((B & 1)<<1) | (A & 1);
        U = (table>>(i + 8)) & 1;
        V = (table>>i) & 1;
        carry = (~U & V) | (U & carryIn);
        K = (carry<<1) | carryIn;
        i = ((C & 2)<<1) | (B & 2) | (A>>1);
        U |= (table>>(i + 7)) & 2;
        i = (table>>i) & 1;
        V |= i + i;
        carry = (~U & V) | (U & K);
        *carryOutPtr = carry>>1;
        switch ( logicBlockConfigPtr->carryResult ) {
         case RESULT_V:
            Z = V;
            break;
         case RESULT_V_OR_K:
            Z = V | K;
            break;
         case RESULT_K_OUT:
            Z = carry;
            break;
         case RESULT_U_XOR_K:
            Z = U ^ K;
            break;
         case RESULT_U_EQU_K:
            Z = 3 - (U ^ K);
            break;
        }
        break;
    }
 exit:
    *ZPtr = Z;

}

static void arrayHalfStep( GarpArrayT *GarpArrayPtr, int phase )
{
    rowStateT *sourceArrayStatePtr, *destinationRowStatePtr;
    int configRowNum, rowNum, colNum;
    logicBlockConfigT *logicBlockConfigPtr;
    logicBlockStateT *sourceLogicBlockStatePtr, *destinationLogicBlockStatePtr;
    int AInOnRight, BInOnRight, CInOnRight, shiftCarry, carry;
    int AIn, BIn, CIn, A, B, C, D, ZOut, DOut, GOutNum;
    controlBlockConfigT *controlBlockConfigPtr;
    int DIn, i;

    if ( phase ) {
        sourceArrayStatePtr = GarpArrayPtr->phase0StatePtr;
        destinationRowStatePtr = GarpArrayPtr->phase1StatePtr;
    } else {
        sourceArrayStatePtr = GarpArrayPtr->phase1StatePtr;
        destinationRowStatePtr = GarpArrayPtr->phase0StatePtr;
    }
    rowNum = GarpArrayPtr->configRowOffset;
    for (
        configRowNum = 0;
        configRowNum < GarpArrayPtr->numConfigRows;
        ++configRowNum, ++rowNum
    ) {
        logicBlockConfigPtr =
            &GarpArrayPtr->configPtr[configRowNum].logicCols[0];
        sourceLogicBlockStatePtr = &sourceArrayStatePtr[rowNum].logicCols[0];
        destinationLogicBlockStatePtr = &destinationRowStatePtr->logicCols[0];
        AIn = 0;
        BIn = 0;
        CIn = 0;
        shiftCarry = 0;
        carry = 0;
        for ( colNum = 0; colNum < numLogicCols; ++colNum ) {
            if ( logicBlockConfigPtr->off ) {
                AIn = 0;
                BIn = 0;
                CIn = 0;
                shiftCarry = 0;
                carry = 0;
                goto next;
            }
            if ( logicBlockConfigPtr->zeroIn ) {
                AInOnRight = 0;
                BInOnRight = 0;
                CInOnRight = 0;
                shiftCarry = 0;
                carry = 0;
            } else {
                AInOnRight = AIn;
                BInOnRight = BIn;
                CInOnRight = CIn;
            }
            AIn =
                getInput(
                    rowNum,
                    colNum,
                    logicBlockConfigPtr->AIn,
                    logicBlockConfigPtr->AInNum,
                    sourceArrayStatePtr,
                    sourceLogicBlockStatePtr
                );
            A = perturbInput( logicBlockConfigPtr->perturbA, AIn, AInOnRight );
            BIn =
                getInput(
                    rowNum,
                    colNum,
                    logicBlockConfigPtr->BIn,
                    logicBlockConfigPtr->BInNum,
                    sourceArrayStatePtr,
                    sourceLogicBlockStatePtr
                );
            B = perturbInput( logicBlockConfigPtr->perturbB, BIn, BInOnRight );
            CIn =
                getInput(
                    rowNum,
                    colNum,
                    logicBlockConfigPtr->CIn,
                    logicBlockConfigPtr->CInNum,
                    sourceArrayStatePtr,
                    sourceLogicBlockStatePtr
                );
            C = perturbInput( logicBlockConfigPtr->perturbC, CIn, CInOnRight );
            DOut =
                getInput(
                    rowNum,
                    colNum,
                    logicBlockConfigPtr->DIn,
                    logicBlockConfigPtr->DInNum,
                    sourceArrayStatePtr,
                    sourceLogicBlockStatePtr
                );
            switch ( logicBlockConfigPtr->mode ) {
             case MODE_DIRECT:
                D = perturbInput( logicBlockConfigPtr->perturbD, DOut, 0 );
                break;
             case MODE_SELECT_1:
             case MODE_SELECT:
                D = DOut;
                break;
             case MODE_PARTSELECT:
                D = BIn;
                break;
            }
            computeZ(
                GarpArrayPtr,
                rowNum,
                colNum,
                logicBlockConfigPtr,
                A,
                B,
                C,
                D,
                shiftCarry,
                carry,
                &shiftCarry,
                &carry,
                sourceArrayStatePtr,
                &ZOut
            );
            if ( phase ) {
                i = sourceLogicBlockStatePtr->ZReg;
                destinationLogicBlockStatePtr->ZReg = i;
                if ( logicBlockConfigPtr->bufferZ ) ZOut = i;
                i = sourceLogicBlockStatePtr->DReg;
                destinationLogicBlockStatePtr->DReg = i;
                if ( logicBlockConfigPtr->bufferD ) DOut = i;
            } else {
                if ( logicBlockConfigPtr->bufferZ ) {
                    destinationLogicBlockStatePtr->ZReg = ZOut;
                }
                if ( logicBlockConfigPtr->bufferD ) {
                    destinationLogicBlockStatePtr->DReg = DOut;
                }
            }
            destinationLogicBlockStatePtr->VOut =
                (logicBlockConfigPtr->VSelect == SELECT_Z) ? ZOut : DOut;
            destinationLogicBlockStatePtr->HOut =
                (logicBlockConfigPtr->HSelect == SELECT_Z) ? ZOut : DOut;
            GOutNum = logicBlockConfigPtr->GOutNum;
            if ( GOutNum < numGWires ) {
                destinationRowStatePtr->GWires[GOutNum] =
                    (logicBlockConfigPtr->GSelect == SELECT_Z) ? ZOut : DOut;
            }
         next:
            ++logicBlockConfigPtr;
            ++sourceLogicBlockStatePtr;
            ++destinationLogicBlockStatePtr;
        }
        controlBlockConfigPtr =
            &GarpArrayPtr->configPtr[configRowNum].controlCol;
        if ( ! phase && ! controlBlockConfigPtr->off ) {
            AIn =
                getInput(
                    rowNum,
                    numLogicCols,
                    controlBlockConfigPtr->AIn,
                    controlBlockConfigPtr->AInNum,
                    sourceArrayStatePtr,
                    0
                );
            A = (perturbInput( controlBlockConfigPtr->perturbA, AIn, 0 ) != 0);
            destinationRowStatePtr->controlEnable = A;
            if ( ! A ) {
                destinationRowStatePtr->controlB = false;
                destinationRowStatePtr->controlC = false;
                destinationRowStatePtr->controlD = false;
            } else {
                BIn =
                    getInput(
                        rowNum,
                        numLogicCols,
                        controlBlockConfigPtr->BIn,
                        controlBlockConfigPtr->BInNum,
                        sourceArrayStatePtr,
                        0
                    );
                destinationRowStatePtr->controlB =
                    (perturbInput( controlBlockConfigPtr->perturbB, BIn, 0 )
                         != 0);
                CIn =
                    getInput(
                        rowNum,
                        numLogicCols,
                        controlBlockConfigPtr->CIn,
                        controlBlockConfigPtr->CInNum,
                        sourceArrayStatePtr,
                        0
                    );
                destinationRowStatePtr->controlC =
                    (perturbInput( controlBlockConfigPtr->perturbC, CIn, 0 )
                         != 0);
                DIn =
                    getInput(
                        rowNum,
                        numLogicCols,
                        controlBlockConfigPtr->DIn,
                        controlBlockConfigPtr->DInNum,
                        sourceArrayStatePtr,
                        0
                    );
                destinationRowStatePtr->controlD =
                    (perturbInput( controlBlockConfigPtr->perturbD, DIn, 0 )
                         != 0);
            }
        }
        ++destinationRowStatePtr;
    }

}

static
 void doControlBeforeStep( GarpArrayT *GarpArrayPtr, uint32_t mainClockCycle )
{
    int queueNum;
    bool accessQueues[numQueues];
    rowConfigT *rowConfigPtr;
    rowStateT *rowStatePtr;
    int i;
    queueT *queuePtr;
    int size, offset, wordOffset;
    int_fast32_t latency;
    int rowNum, attributes, count;
    readBufferT *readBufferPtr;
    int readBufferNum;
    uint32_t word;
    int wordNum;
    uint8_t *dataPtr;

    /*------------------------------------------------------------------------
    | Note all queue read requests.  (Multiple requests to the same queue in
    | the same cycle are idempotent.)
    *------------------------------------------------------------------------*/
    for ( queueNum = 0; queueNum < numQueues; ++queueNum ) {
        accessQueues[queueNum] = false;
    }
    rowConfigPtr = &GarpArrayPtr->configPtr[0];
    rowStatePtr = &GarpArrayPtr->phase0StatePtr[0];
    for ( i = GarpArrayPtr->numConfigRows; i; --i ) {
        if (
            rowStatePtr->controlEnable
                && (rowConfigPtr->controlCol.mode == MODE_MEMORY_QUEUE)
                && rowStatePtr->controlB
        ) {
            queueNum = rowConfigPtr->controlCol.queueNum;
            if (
                GarpArrayPtr->queues[queueNum].control.direction == ga_read
            ) {
                accessQueues[queueNum] = true;
            }
        }
        ++rowConfigPtr;
        ++rowStatePtr;
    }
    /*------------------------------------------------------------------------
    | If any queue read request must stall, force array stall and return.
    *------------------------------------------------------------------------*/
    queuePtr = &GarpArrayPtr->queues[0];
    for ( queueNum = 0; queueNum < numQueues; ++queueNum ) {
        if ( accessQueues[queueNum] ) {
            size = 1<<(queuePtr->control.size + queuePtr->control.count);
            offset = queuePtr->outOffset;
            wordOffset = offset>>2;
            for ( i = 1 + ((offset + size - 1)>>2) - wordOffset; i; --i ) {
                latency =
                    queuePtr->returnMainClockCycle[wordOffset]
                        - mainClockCycle;
                if ( 0 < latency ) {
                    GarpArrayPtr->readStall = latency;
                    return;
                }
                wordOffset = (wordOffset + 1) & ((numBytesInQueue>>2) - 1);
            }
        }
        ++queuePtr;
    }
    /*------------------------------------------------------------------------
    | Can no longer stall.  Drive memory buses from array registers before
    | responding to any write requests.
    *------------------------------------------------------------------------*/
    rowNum = GarpArrayPtr->configRowOffset;
    rowConfigPtr = &GarpArrayPtr->configPtr[0];
    rowStatePtr = &GarpArrayPtr->phase0StatePtr[0];
    for ( i = GarpArrayPtr->numConfigRows; i; --i ) {
        if ( rowStatePtr->controlEnable ) {
            switch ( rowConfigPtr->controlCol.mode ) {
             case MODE_MEMORY_QUEUE:
             case MODE_MEMORY_DEMAND:
                if ( rowStatePtr->controlC && rowStatePtr->controlD ) {
                    GarpArrayPtr->
                        memoryBuses[rowConfigPtr->controlCol.busNum] =
                        ga_reg(
                            GarpArrayPtr,
                            (rowNum<<1) | rowConfigPtr->controlCol.busSelect
                        );
                }
                break;
            }
        }
        ++rowNum;
        ++rowConfigPtr;
        ++rowStatePtr;
    }
    /*------------------------------------------------------------------------
    | Search for a demand access request and convert it to a memory request.
    | Additional demand requests after the first one found are ignored.  Also
    | note all queue access requests for the next phase.
    *------------------------------------------------------------------------*/
    rowNum = GarpArrayPtr->configRowOffset;
    rowConfigPtr = &GarpArrayPtr->configPtr[0];
    rowStatePtr = &GarpArrayPtr->phase0StatePtr[0];
    for ( i = GarpArrayPtr->numConfigRows; i; --i ) {
        if ( rowStatePtr->controlEnable ) {
            switch ( rowConfigPtr->controlCol.mode ) {
             case MODE_MEMORY_QUEUE:
                if ( rowStatePtr->controlB ) {
                    accessQueues[rowConfigPtr->controlCol.queueNum] = true;
                }
                break;
             case MODE_MEMORY_DEMAND:
                if (
                    rowStatePtr->controlB
                        && ! GarpArrayPtr->memoryRequest.enable
                ) {
                    attributes = rowConfigPtr->controlCol.attributes;
                    size = rowConfigPtr->controlCol.requestSize;
                    count = rowConfigPtr->controlCol.requestCount;
                    GarpArrayPtr->memoryRequest.enable = true;
                    GarpArrayPtr->memoryRequest.size = size + count;
                    GarpArrayPtr->memoryRequest.address =
                        ga_reg( GarpArrayPtr, rowNum<<1 );
                    if ( ! rowStatePtr->controlD ) {
                        attributes &= ~ga_prefetch;
                        GarpArrayPtr->memoryRequest.direction = ga_read;
                        readBufferPtr = &GarpArrayPtr->readBuffers[0];
                        readBufferNum = 0;
                        while ( readBufferPtr->enable ) {
                            ++readBufferPtr;
                            ++readBufferNum;
                        }
                        GarpArrayPtr->memoryRequest.source = readBufferNum;
                        readBufferPtr->enable = true;
                        readBufferPtr->size = size;
                        readBufferPtr->count = count;
                        latency = rowConfigPtr->controlCol.latency;
                        readBufferPtr->delay = latency;
                        readBufferPtr->expectedMainClockCycle =
/*** CHECK THIS. ***/
                            mainClockCycle + latency - 3;
                    } else if ( attributes & ga_prefetch ) {
                        GarpArrayPtr->memoryRequest.direction = ga_read;
                    } else {
                        GarpArrayPtr->memoryRequest.direction = ga_write;
                        switch ( size ) {
                         case ga_1:
                            count = 1<<count;
                            word = 0;
                            for ( wordNum = 0; wordNum < count; ++wordNum ) {
                                word =
                                    (word<<8)
                                        | (GarpArrayPtr->memoryBuses[wordNum]
                                               & 0xFF);
                            }
                            GarpArrayPtr->memoryRequest.data[0] = word;
                            break;
                         case ga_2:
                            switch ( count ) {
                             case ga_1:
                                GarpArrayPtr->memoryRequest.data[0] =
                                    GarpArrayPtr->memoryBuses[0] & 0xFFFF;
                                break;
                             case ga_2:
                                GarpArrayPtr->memoryRequest.data[0] =
                                    (GarpArrayPtr->memoryBuses[0]<<16)
                                        | (GarpArrayPtr->memoryBuses[1]
                                               & 0xFFFF);
                                break;
                             case ga_4:
                                GarpArrayPtr->memoryRequest.data[0] =
                                    (GarpArrayPtr->memoryBuses[0]<<16)
                                        | (GarpArrayPtr->memoryBuses[1]
                                               & 0xFFFF);
                                GarpArrayPtr->memoryRequest.data[1] =
                                    (GarpArrayPtr->memoryBuses[2]<<16)
                                        | (GarpArrayPtr->memoryBuses[3]
                                               & 0xFFFF);
                                break;
                            }
                            break;
                         case ga_4:
                            count = 1<<count;
                            for ( wordNum = 0; wordNum < count; ++wordNum ) {
                                GarpArrayPtr->memoryRequest.data[wordNum] =
                                    GarpArrayPtr->memoryBuses[wordNum];
                            }
                            break;
                        }
                    }
                    GarpArrayPtr->memoryRequest.attributes = attributes;
                }
                break;
            }
        }
        ++rowNum;
        ++rowConfigPtr;
        ++rowStatePtr;
    }
    /*------------------------------------------------------------------------
    | Perform queue accesses.
    *------------------------------------------------------------------------*/
    queuePtr = &GarpArrayPtr->queues[0];
    for ( queueNum = 0; queueNum < numQueues; ++queueNum ) {
        if ( accessQueues[queueNum] ) {
            size = queuePtr->control.size;
            count = 1<<(queuePtr->control.count);
            switch ( queuePtr->control.direction ) {
             case ga_read:
                offset = queuePtr->outOffset;
                dataPtr = &queuePtr->data[0];
                switch ( size ) {
                 case ga_1:
                    for ( wordNum = 0; wordNum < count; ++wordNum ) {
                        GarpArrayPtr
                            ->memoryBuses[queuePtr->control.busNums[wordNum]] =
                            dataPtr[offset];
                        offset = (offset + 1) & (numBytesInQueue - 1);
                    }
                    break;
                 case ga_2:
                    for ( wordNum = 0; wordNum < count; ++wordNum ) {
                        GarpArrayPtr
                            ->memoryBuses[queuePtr->control.busNums[wordNum]] =
                            (dataPtr[offset]<<8)
                                | dataPtr
                                      [(offset + 1) & (numBytesInQueue - 1)];
                        offset = (offset + 2) & (numBytesInQueue - 1);
                    }
                    break;
                 case ga_4:
                    if ( ! (offset & 3) ) {
                        for ( wordNum = 0; wordNum < count; ++wordNum ) {
/***
printf(
 "Reading from queue, offset %d:  %08X\n",
 offset,
 *((uint32_t *) &dataPtr[offset])
);
***/
                            GarpArrayPtr
                                ->memoryBuses
                                    [queuePtr->control.busNums[wordNum]] =
                                *((uint32_t *) &dataPtr[offset]);
                            offset = (offset + 4) & (numBytesInQueue - 1);
                        }
                    } else {
                        for ( wordNum = 0; wordNum < count; ++wordNum ) {
                            GarpArrayPtr
                                ->memoryBuses
                                    [queuePtr->control.busNums[wordNum]] =
                                  (dataPtr[offset]<<24)
                                | (dataPtr
                                       [(offset + 1) & (numBytesInQueue - 1)]
                                       <<16)
                                | (dataPtr
                                       [(offset + 2) & (numBytesInQueue - 1)]
                                       <<8)
                                | dataPtr
                                      [(offset + 3) & (numBytesInQueue - 1)];
                            offset = (offset + 4) & (numBytesInQueue - 1);
                        }
                    }
                    break;
                }
                size = count<<size;
                queuePtr->control.address += size;
                queuePtr->count -= size;
                queuePtr->outOffset = offset;
                break;
             case ga_write:
                offset = queuePtr->inOffset;
                dataPtr = &queuePtr->data[0];
                switch ( size ) {
                 case ga_1:
                    for ( wordNum = 0; wordNum < count; ++wordNum ) {
                        dataPtr[offset] =
                            GarpArrayPtr
                                ->memoryBuses
                                    [queuePtr->control.busNums[wordNum]];
                        offset = (offset + 1) & (numBytesInQueue - 1);
                    }
                    break;
                 case ga_2:
                    for ( wordNum = 0; wordNum < count; ++wordNum ) {
                        word =
                            GarpArrayPtr
                                ->memoryBuses
                                    [queuePtr->control.busNums[wordNum]];
                        dataPtr[offset] = word>>8;
                        dataPtr[(offset + 1) & (numBytesInQueue - 1)] = word;
                        offset = (offset + 2) & (numBytesInQueue - 1);
                    }
                    break;
                 case ga_4:
                    if ( ! (offset & 3) ) {
                        for ( wordNum = 0; wordNum < count; ++wordNum ) {
                            *((uint32_t *) &dataPtr[offset]) =
                                GarpArrayPtr
                                    ->memoryBuses
                                        [queuePtr->control.busNums[wordNum]];
                            offset = (offset + 4) & (numBytesInQueue - 1);
                        }
                    } else {
                        for ( wordNum = 0; wordNum < count; ++wordNum ) {
                            word =
                                GarpArrayPtr
                                    ->memoryBuses
                                        [queuePtr->control.busNums[wordNum]];
                            dataPtr[offset] = word>>24;
                            dataPtr[(offset + 1) & (numBytesInQueue - 1)] =
                                word>>16;
                            dataPtr[(offset + 2) & (numBytesInQueue - 1)] =
                                word>>8;
                            dataPtr[(offset + 3) & (numBytesInQueue - 1)] =
                                word;
                            offset = (offset + 4) & (numBytesInQueue - 1);
                        }
                    }
                    break;
                }
                size = count<<size;
                queuePtr->control.address += size;
                queuePtr->count += size;
                queuePtr->inOffset = offset;
                break;
            }
        }
        ++queuePtr;
    }
    /*------------------------------------------------------------------------
    | Update pending demand reads.
    *------------------------------------------------------------------------*/
    readBufferPtr = &GarpArrayPtr->readBuffers[0];
    for ( i = numReadBuffers; i; --i ) {
        if ( readBufferPtr->enable ) {
            if ( ! --readBufferPtr->delay ) {
                switch ( readBufferPtr->size ) {
                 case ga_1:
                    count = 1<<(readBufferPtr->count);
                    word = readBufferPtr->data[0];
                    while ( count-- ) {
                        GarpArrayPtr->memoryBuses[count] = word & 0xFF;
                        word >>= 8;
                    }
                    break;
                 case ga_2:
                    word = readBufferPtr->data[0];
                    switch ( readBufferPtr->count ) {
                     case ga_1:
                        GarpArrayPtr->memoryBuses[0] = word & 0xFFFF;
                        break;
                     case ga_2:
                        GarpArrayPtr->memoryBuses[0] = word>>16;
                        GarpArrayPtr->memoryBuses[1] = word & 0xFFFF;
                        break;
                     case ga_4:
                        GarpArrayPtr->memoryBuses[0] = word>>16;
                        GarpArrayPtr->memoryBuses[1] = word & 0xFFFF;
                        word = readBufferPtr->data[1];
                        GarpArrayPtr->memoryBuses[2] = word>>16;
                        GarpArrayPtr->memoryBuses[3] = word & 0xFFFF;
                        break;
                    }
                    break;
                 case ga_4:
                    count = 1<<(readBufferPtr->count);
                    while ( count-- ) {
                        GarpArrayPtr->memoryBuses[count] =
                            readBufferPtr->data[count];
                    }
                    break;
                }
                readBufferPtr->enable = false;
            }
        }
        ++readBufferPtr;
    }

}

static void doControlAfterStep( GarpArrayT *GarpArrayPtr )
{
    int rowNum;
    rowConfigT *rowConfigPtr;
    rowStateT *rowStatePtr;
    int i;

    rowNum = GarpArrayPtr->configRowOffset;
    rowConfigPtr = &GarpArrayPtr->configPtr[0];
    rowStatePtr = &GarpArrayPtr->phase0StatePtr[0];
    for ( i = GarpArrayPtr->numConfigRows; i; --i ) {
        if ( rowStatePtr->controlEnable ) {
            switch ( rowConfigPtr->controlCol.mode ) {
             case MODE_PROCESSOR:
                if ( rowStatePtr->controlB ) GarpArrayPtr->branchFlag = true;
                if ( rowStatePtr->controlC ) GarpArrayPtr->clock = 0;
                if ( rowStatePtr->controlD ) {
                    GarpArrayPtr->interruptFlag = true;
                }
                break;
             case MODE_MEMORY_QUEUE:
             case MODE_MEMORY_DEMAND:
                if ( rowStatePtr->controlC && ! rowStatePtr->controlD ) {
                    ga_setRegL(
                        GarpArrayPtr,
                        (rowNum<<1) | rowConfigPtr->controlCol.busSelect,
                        4<<(rowConfigPtr->controlCol.dataSize),
                        4,
                        GarpArrayPtr
                            ->memoryBuses[rowConfigPtr->controlCol.busNum]
                    );
                }
                break;
            }
        }
        ++rowNum;
        ++rowConfigPtr;
        ++rowStatePtr;
    }

}

static void requestQueueRead( GarpArrayT *GarpArrayPtr, int queueNum )
{
    queueT *queuePtr;
    uint32_t address;
    int size;

    queuePtr = &GarpArrayPtr->queues[queueNum];
    address = queuePtr->address;
    GarpArrayPtr->memoryRequest.enable = true;
    GarpArrayPtr->memoryRequest.direction = ga_read;
    GarpArrayPtr->memoryRequest.attributes = queuePtr->control.attributes;
    GarpArrayPtr->memoryRequest.source = ~queueNum;
/*** SHOULD MATCH CACHE LINE SIZE.
    if ( ! (address & 63) ) {
        size = ga_64;
    } else
***/
    if ( ! (address & 31) ) {
        size = ga_32;
    } else if ( ! (address & 15) ) {
        size = ga_16;
    } else if ( ! (address & 7) ) {
        size = ga_8;
    } else if ( ! (address & 3) ) {
        size = ga_4;
    } else if ( ! (address & 1) ) {
        size = ga_2;
    } else {
        size = ga_1;
    }
    GarpArrayPtr->memoryRequest.size = size;
    GarpArrayPtr->memoryRequest.address = address;

}

static void requestQueueWrite( GarpArrayT *GarpArrayPtr, int queueNum )
{
    queueT *queuePtr;
    uint32_t address;
    int outOffset, size, i;

    queuePtr = &GarpArrayPtr->queues[queueNum];
    address = queuePtr->address;
    GarpArrayPtr->memoryRequest.enable = true;
    GarpArrayPtr->memoryRequest.direction = ga_write;
    GarpArrayPtr->memoryRequest.attributes = queuePtr->control.attributes;
    GarpArrayPtr->memoryRequest.address = address;
/*** SHOULD MATCH CACHE LINE SIZE.
    if ( ! (address & 63) ) {
        size = ga_64;
    } else
***/
    if ( ! (address & 31) ) {
        size = ga_32;
    } else if ( ! (address & 15) ) {
        size = ga_16;
    } else if ( ! (address & 7) ) {
        size = ga_8;
    } else if ( ! (address & 3) ) {
        size = ga_4;
    } else if ( ! (address & 1) ) {
        size = ga_2;
    } else {
        size = ga_1;
    }
    GarpArrayPtr->memoryRequest.size = size;
    size = 1<<size;
    queuePtr->address = address + size;
    queuePtr->count -= size;
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
            GarpArrayPtr->memoryRequest.data[0] = queuePtr->data[outOffset];
        }
        outOffset = (outOffset + size) & (numBytesInQueue - 1);
    }
    queuePtr->outOffset = outOffset;

}


void
 ga_step( GarpArrayT *GarpArrayPtr, uint32_t mainClockCycle, bool DRAMIdle )
{
    queueT *queuePtr;
    int queueNum, queueNumWithBestCount, bestCount, count;

/****/ GarpArrayPtr->memoryBuses[0] = 0;
    GarpArrayPtr->memoryRequest.enable = false;
    queuePtr = &GarpArrayPtr->queues[0];
    for ( queueNum = 0; queueNum < numQueues; ++queueNum ) {
        if (
            queuePtr->control.enable
                && (queuePtr->control.direction == ga_read)
                && (queuePtr->count < 16)
        ) {
            requestQueueRead( GarpArrayPtr, queueNum );
            return;
        }
        ++queuePtr;
    }
    queuePtr = &GarpArrayPtr->queues[0];
    for ( queueNum = 0; queueNum < numQueues; ++queueNum ) {
        if (
            queuePtr->control.enable
                && (queuePtr->control.direction == ga_write)
                && (numBytesInQueue - 16 < queuePtr->count)
        ) {
            requestQueueWrite( GarpArrayPtr, queueNum );
            return;
        }
        ++queuePtr;
    }
    if ( GarpArrayPtr->clock ) {
        if ( GarpArrayPtr->readStall ) {
/***
puts( "Stalled." );
***/
            --GarpArrayPtr->readStall;
        } else {
            doControlBeforeStep( GarpArrayPtr, mainClockCycle );
            if ( ! GarpArrayPtr->readStall ) {
/***
printf( "memory bus: %08X %08X %08X %08X\n",
GarpArrayPtr->memoryBuses[0],
GarpArrayPtr->memoryBuses[1],
GarpArrayPtr->memoryBuses[2],
GarpArrayPtr->memoryBuses[3]
);
***/
                arrayHalfStep( GarpArrayPtr, 1 );
                arrayHalfStep( GarpArrayPtr, 0 );
                GarpArrayPtr->clock =
                    (GarpArrayPtr->clock & 0x80000000UL)
                        | ((GarpArrayPtr->clock & 0x7FFFFFFFUL) - 1);
                GarpArrayPtr->branchFlag = false;
                GarpArrayPtr->interruptFlag = false;
/***
printf( "memory bus: %08X %08X %08X %08X\n",
GarpArrayPtr->memoryBuses[0],
GarpArrayPtr->memoryBuses[1],
GarpArrayPtr->memoryBuses[2],
GarpArrayPtr->memoryBuses[3]
);
***/
                doControlAfterStep( GarpArrayPtr );
            }
            if ( GarpArrayPtr->memoryRequest.enable ) return;
        }
    }
    if ( DRAMIdle ) {
        queueNumWithBestCount = -1;
        bestCount = queueRequestSize - 1;
        queuePtr = &GarpArrayPtr->queues[0];
        for ( queueNum = 0; queueNum < numQueues; ++queueNum ) {
            if ( queuePtr->control.enable ) {
                count = queuePtr->count;
                if ( queuePtr->control.direction == ga_read ) {
                    count = numBytesInQueue - count;
                }
                if ( bestCount < count ) {
                    queueNumWithBestCount = queueNum;
                    bestCount = count;
                }
            }
            ++queuePtr;
        }
        if ( 0 <= queueNumWithBestCount ) {
            switch (
                GarpArrayPtr->queues[queueNumWithBestCount].control.direction
            ) {
             case ga_read:
                requestQueueRead( GarpArrayPtr, queueNumWithBestCount );
                break;
             case ga_write:
                requestQueueWrite( GarpArrayPtr, queueNumWithBestCount );
                break;
            }
        }
    }

}

