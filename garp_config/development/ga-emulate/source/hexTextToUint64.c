
#include <stdint.h>
#include "platform.h"
#include "hexTextToUint64.h"

uint_fast64_t hexTextToUint64( const char *ptr, const char **stopPtrPtr )
{
    uint64_t z;
    char ch;
    int digit;

    z = 0;
    for (;;) {
        ch = *ptr;
        if ( ('0' <= ch) && (ch <= '9') ) {
            digit = ch - '0';
        } else if ( ('A' <= ch) && (ch <= 'F') ) {
            digit = ch - 'A' + 10;
        } else if ( ('a' <= ch) && (ch <= 'f') ) {
            digit = ch - 'a' + 10;
        } else {
            break;
        }
        if ( z>>60 ) {
            z = -1;
        } else {
            z = (z<<4) + digit;
        }
        ++ptr;
    }
    if ( stopPtrPtr ) *stopPtrPtr = ptr;
    return z;

}

