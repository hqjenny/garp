
#include <stdlib.h>
#include <stdio.h>
#include "platform.h"
#include "fail.h"

char *fail_programName = "";

void
 writeFailMessage(
     FILE *streamPtr, const char *messagePtr, const char *argPtr )
{

    fputs( fail_programName, streamPtr );
    fputs( ": ", streamPtr );
    fprintf( streamPtr, messagePtr, argPtr );
    fputc( '\n', streamPtr );

}

void fail( const char *messagePtr, const char *argPtr )
{

    writeFailMessage( stderr, messagePtr, argPtr );
    exit( EXIT_FAILURE );

}

