
#include <stdint.h>
#include <stdio.h>
#include "platform.h"
#include "writeUint64AsText.h"

static char * uint64ToBasicText( uint_fast64_t a, char *p )
{
    int started;
    static const uint_fast64_t largeDecimalDivisors[11] = {
        UINT64_C( 10000000000000000000 ),
         UINT64_C( 1000000000000000000 ),
          UINT64_C( 100000000000000000 ),
           UINT64_C( 10000000000000000 ),
            UINT64_C( 1000000000000000 ),
             UINT64_C( 100000000000000 ),
              UINT64_C( 10000000000000 ),
               UINT64_C( 1000000000000 ),
                UINT64_C( 100000000000 ),
                 UINT64_C( 10000000000 ),
                  UINT64_C( 1000000000 )
    };
    const uint_fast64_t *largeDivisorPtr;
    int digitNum;
    uint_fast64_t largeDivisor;
    int digit;
    uint_fast32_t smallA;
    static const uint_fast32_t smallDecimalDivisors[8] =
        { 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10 };
    const uint_fast32_t *smallDivisorPtr;
    uint_fast32_t smallDivisor;

    started = 0;
    if ( 1000000000 <= a ) {
        largeDivisorPtr = largeDecimalDivisors;
        for ( digitNum = 19; 8 < digitNum; --digitNum ) {
            largeDivisor = *largeDivisorPtr;
            for ( digit = 0; largeDivisor <= a; ++digit ) a -= largeDivisor;
            if ( started || (started = digit) ) *(p++) = '0' + digit;
            ++largeDivisorPtr;
        }
    }
    smallA = a;
    smallDivisorPtr = smallDecimalDivisors;
    for ( digitNum = 8; 0 < digitNum; --digitNum ) {
        smallDivisor = *smallDivisorPtr;
        for ( digit = 0; smallDivisor <= smallA; ++digit ) {
            smallA -= smallDivisor;
        }
        if ( started || (started = digit) ) *(p++) = '0' + digit;
        ++smallDivisorPtr;
    }
    *(p++) = '0' + smallA;
    return p;

}

void writeUint64AsText( FILE *streamPtr, uint_fast64_t a )
{
    char textNumeral[128], *stopPtr;

    stopPtr = uint64ToBasicText( a, textNumeral );
    fwrite( textNumeral, stopPtr - textNumeral, 1, streamPtr );

}

