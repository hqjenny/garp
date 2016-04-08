
#include <stdint.h>
#include <stdio.h>
#include "platform.h"
#include "tokens.h"
#include "wires.h"
#include "config.h"
#include "outputConfig.h"

void outputConfig( FILE *streamPtr )
{
    int rowNum, HWiresDirection, colNum;
    uint64_t *configPtr, config;
    int count;

    fprintf( streamPtr, "{\n    0x%08X,\n", (unsigned) numRows );
    for ( rowNum = 0; rowNum < numRows; ++rowNum ) {
        config = rowConfigPtrs[rowNum]->controlConfig;
        fprintf(
            streamPtr,
            "    0x%08lX, 0x%08lX,",
            (unsigned long) (uint32_t) (config>>32),
            (unsigned long) (uint32_t) config
        );
        configPtr =
            &(rowConfigPtrs[rowNum]->logicColConfigs[NUMLOGICCOLS - 1]);
        count = 3;
        for ( colNum = NUMLOGICCOLS - 1; 0 <= colNum; --colNum ) {
            --count;
            if ( ! count ) {
                fputs( "\n   ", streamPtr );
                count = 3;
            }
            config = *configPtr;
            fprintf(
                streamPtr,
                " 0x%08lX, 0x%08lX,",
                (unsigned long) (uint32_t) (config>>32),
                (unsigned long) (uint32_t) config
            );
            --configPtr;
        }
        fputc( '\n', streamPtr );
    }
    fputs( "}\n", streamPtr );

}

