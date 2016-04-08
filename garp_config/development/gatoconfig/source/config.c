
#include <stdint.h>
#include <stdio.h>
#include "platform.h"
#include "wires.h"
#include "config.h"

int numRows;
rowConfigT **rowConfigPtrs;
int *fusedRowCounts;

void dumpConfig( void )
{
    int rowNum;
    rowConfigT *rowConfigPtr;
    int i, mode;
    uint64_t config;
    wireT *wirePtr;

    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        rowConfigPtr = rowConfigPtrs[rowNum];
        printf(
           "row %2d:\n"
           "  fusedRowCount: %2d\n"
           "  rowConfig:\n"
           "    horizMin: %d; horizMax: %d, HWiresDirection: %d, %08lX %08lX\n"
           "    rowInputs:\n",
            rowNum,
            fusedRowCounts[rowNum],
            rowConfigPtr->horizMin,
            rowConfigPtr->horizMax,
            rowConfigPtr->HWiresDirection,
            (unsigned long) (rowConfigPtr->controlConfig>>32),
            (unsigned long) (rowConfigPtr->controlConfig & 0xFFFFFFFF)
        );
        for ( i = 0; i < numRows; ++i ) {
            if ( rowConfigPtr->rowInputs[i] ) {
                printf(
                    "     %3d: %06lX\n",
                    i,
                    (unsigned long) rowConfigPtr->rowInputs[i]
                );
            }
        }
        for ( i = 0; i < NUMLOGICCOLS; ++i ) {
            mode = rowConfigPtr->logicColModes[i];
            config = rowConfigPtr->logicColConfigs[i];
            wirePtr = rowConfigPtr->logicColVOutWires[i];
            if ( mode || config || wirePtr ) {
                printf(
                    "    col %2d:  mode %2d, %08lX %08lX",
                    i,
                    mode,
                    (unsigned long) (config>>32),
                    (unsigned long) (config & 0xFFFFFFFF)
                );
                if ( rowConfigPtr->logicColHOutUsed[i] ) {
                    fputs( ", H", stdout );
                }
                if ( wirePtr == useHWire ) {
                    fputs( ", V->H", stdout );
                } else if ( wirePtr ) {
                    printf(
                        ", V->{ nomLength: %2d; nomFirst: %2d }",
                        wirePtr->nomLength,
                        wirePtr->nomFirst
                    );
                } else if( rowConfigPtr->logicColVOutLong[i] ) {
                    fputs( ", V long", stdout );
                }
                puts( "" );
            }
        }
    }

}

