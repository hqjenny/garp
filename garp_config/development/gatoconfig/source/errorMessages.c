
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "platform.h"
#include "allocate.h"
#include "errorMessages.h"

bool debugErrorMessages;

uint_fast32_t errorCount;

typedef struct errorMessageListElementT {
    struct errorMessageListElementT *prevListElement, *nextListElement;
    uint_fast32_t lineNum;
    uint_fast16_t columnNum;
    const char *message;
    const char *insideMessage;
    uint_fast64_t cols;
} errorMessageListElementT;

static uint_fast32_t maxNumErrors;
static errorMessageListElementT
    *errorMessageList, *overflowErrorMessage, *lastErrorMessage;

void initErrorMessages( uint_fast32_t maxNumErrorsZ )
{

    maxNumErrors = maxNumErrorsZ;
    errorCount = 0;
    errorMessageList = 0;
    overflowErrorMessage = 0;
    lastErrorMessage = 0;

}

void
 markError2InColumn(
     uint_fast32_t lineNum,
     uint_fast16_t columnNum,
     const char *message,
     const char *insideMessage,
     int colNum
 )
{
    errorMessageListElementT *p, *q;
    uint_fast32_t internalErrorCount;
    errorMessageListElementT *errorMessageListElement;
    uint_fast64_t cols;

    if ( debugErrorMessages ) {
        printf(
            "errorMessages- markError( %lu, %u, \"%s\"",
            (unsigned long) lineNum,
            (unsigned int) columnNum,
            message
        );
        if ( insideMessage ) printf( ", \"%s\"", insideMessage );
        if ( 0 <= colNum ) printf( ", %d", colNum );
        puts( " )" );
        fflush( stdout );
    }
    if ( ! maxNumErrors ) return;
    if (
        (maxNumErrors < errorCount) && (lastErrorMessage->lineNum < lineNum)
    ) {
        ++errorCount;
        return;
    }
    cols = (colNum < 0) ? 0 : (1<<colNum);
    internalErrorCount = 0;
    p = errorMessageList;
    while ( p && (p->lineNum < lineNum) ) {
        ++internalErrorCount;
        if ( maxNumErrors <= internalErrorCount ) return;
        p = p->nextListElement;
    }
    while ( p && (p->lineNum == lineNum) && (p->columnNum < columnNum) ) {
        ++internalErrorCount;
        p = p->nextListElement;
    }
    while ( p && (p->lineNum == lineNum) && (p->columnNum == columnNum) ) {
        if ( (p->message == message) && (p->insideMessage == insideMessage) ) {
            p->cols |= cols;
            return;
        }
        ++internalErrorCount;
        p = p->nextListElement;
    }
    ++errorCount;
    if ( overflowErrorMessage ) {
        if ( internalErrorCount < maxNumErrors ) {
            overflowErrorMessage = overflowErrorMessage->prevListElement;
            if ( lastErrorMessage->lineNum == overflowErrorMessage->lineNum ) {
                goto allocNewListElement;
            }
        } else {
            if ( lastErrorMessage->lineNum == lineNum ) {
                goto allocNewListElement;
            }
            if ( p == lastErrorMessage ) p = 0;
        }
        q = lastErrorMessage->prevListElement;
        q->nextListElement = 0;
        errorMessageListElement = lastErrorMessage;
        lastErrorMessage = q;
        goto newListElementReady;
    }
 allocNewListElement:
    errorMessageListElement = allocType( errorMessageListElementT );
 newListElementReady:
    errorMessageListElement->lineNum = lineNum;
    errorMessageListElement->columnNum = columnNum;
    errorMessageListElement->message = message;
    errorMessageListElement->insideMessage = insideMessage;
    errorMessageListElement->cols = cols;
    if ( lastErrorMessage ) {
        if ( p ) {
            q = p->prevListElement;
            if ( ! q ) {
                errorMessageList = errorMessageListElement;
            } else {
                q->nextListElement = errorMessageListElement;
            }
            errorMessageListElement->prevListElement = q;
            errorMessageListElement->nextListElement = p;
            p->prevListElement = errorMessageListElement;
        } else {
            errorMessageListElement->prevListElement = lastErrorMessage;
            errorMessageListElement->nextListElement = 0;
            lastErrorMessage->nextListElement = errorMessageListElement;
            lastErrorMessage = errorMessageListElement;
        }
        if ( maxNumErrors < errorCount ) {
            if ( overflowErrorMessage ) {
                if ( internalErrorCount == maxNumErrors ) {
                    overflowErrorMessage = errorMessageListElement;
                }
            } else {
                overflowErrorMessage = lastErrorMessage;
            }
        }
    } else {
        errorMessageListElement->prevListElement = 0;
        errorMessageListElement->nextListElement = 0;
        errorMessageList = errorMessageListElement;
        lastErrorMessage = errorMessageListElement;
    }

}

void reportErrors( FILE *sourceFile, char *sourceFileName )
{
    errorMessageListElementT *p;
    uint_fast32_t errorCount;
    uint_fast32_t currentLine, lineNum;
    int_fast32_t columnNum, numLeadingSpaces;
    char sourceLine[128];
    char currentLineAsText[16];
    char message[1024];
    int messageLength, totalLength;
    uint_fast64_t cols;
    char tensDigit, onesDigit;
    int len, ch;
    int_fast32_t i;
    char *q;

    if ( debugErrorMessages ) return;
    p = errorMessageList;
    if ( ! p ) return;
    errorCount = 0;
    if ( sourceFile ) {
        currentLine = 0;
        while ( p ) {
            lineNum = p->lineNum;
            if ( currentLine < lineNum ) {
                if ( maxNumErrors <= errorCount ) break;
                ++currentLine;
                while ( currentLine < lineNum ) {
                    if ( ! fgets( sourceLine, 128, sourceFile ) ) break;
                    len = strlen( sourceLine );
                    currentLine += (sourceLine[len - 1] == '\n');
                }
                putchar( '\n' );
                sprintf(
                    currentLineAsText, "%3lu:", (unsigned long) currentLine );
                fputs( currentLineAsText, stdout );
                columnNum = 0;
                for (;;) {
                    while ( (ch = getc( sourceFile )) == ' ' ) ++columnNum;
                    numLeadingSpaces = columnNum;
                    switch ( ch ) {
                     case '\t':
                        columnNum = (columnNum | 7) + 1;
                        continue;
                     case '\n':
                     case '\v':
                     case '\f':
                     case EOF:
                        putchar( '\n' );
                        goto lineWritten;
                    }
                    printf( "  %c", ch );
                    ++columnNum;
                    break;
                }
                for (;;) {
                    while ( isprint( ch = getc( sourceFile ) ) ) {
                        putchar( ch );
                        ++columnNum;
                    }
                    switch ( ch ) {
                     case '\t':
                        do {
                            putchar( ' ' );
                            ++columnNum;
                        } while ( columnNum & 7 );
                        break;
                     case '\n':
                     case '\v':
                     case '\f':
                     case EOF:
                        putchar( '\n' );
                        goto lineWritten;
                    }
                }
            }
         lineWritten:
            columnNum = p->columnNum - numLeadingSpaces;
            if ( columnNum < 0 ) columnNum = -1;
            len = strlen( currentLineAsText ) + 2;
            for ( i = -len; i < columnNum; ++i ) putchar( '-' );
            putchar( '^' );
            sprintf( message, p->message, p->insideMessage );
            message[0] = toupper( message[0] );
            messageLength = strlen( message );
            cols = p->cols;
            if ( cols ) {
                q = strcpy( message + messageLength, " in column" ) + 10;
                if ( (cols & -cols) != cols ) *(q++) = 's';
                *(q++) = ' ';
                tensDigit = '0';
                onesDigit = '0';
                while( cols ) {
                    if ( cols & 1 ) {
                        if ( '0' < tensDigit ) *(q++) = tensDigit;
                        *(q++) = onesDigit;
                        if ( (cols & 6) == 6 ) {
                            do {
                                cols >>= 1;
                                if ( onesDigit < '9' ) {
                                    ++onesDigit;
                                } else {
                                    ++tensDigit;
                                    onesDigit = '0';
                                }
                            } while ( (cols & 6) == 6 );
                            ch = '-';
                        } else {
                            ch = (cols ^ 1) ? ',' : '.';
                        }
                        *(q++) = ch;
                    }
                    cols >>= 1;
                    if ( onesDigit < '9' ) {
                        ++onesDigit;
                    } else {
                        ++tensDigit;
                        onesDigit = '0';
                    }
                }
                *q = 0;
                messageLength = q - message;
            }
            totalLength = len + columnNum + messageLength;
            if ( totalLength < 79 ) {
                putchar( ' ' );
            } else if ( 79 < totalLength ) {
                putchar( '\n' );
                for ( i = messageLength; i < 79; ++i ) putchar( ' ' );
            }
            puts( message );
            p = p->nextListElement;
            ++errorCount;
        }
        putchar( '\n' );
    } else {
        while ( p && (errorCount < maxNumErrors) ) {
            if ( sourceFileName ) printf( "%s:", sourceFileName );
            sprintf( message, p->message, p->insideMessage );
            message[0] = toupper( message[0] );
            printf( "%3lu: %s\n", (unsigned long) p->lineNum, message );
            p = p->nextListElement;
            ++errorCount;
        }
    }

}

