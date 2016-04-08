
#include <ctype.h>
#include "platform.h"
#include "scan.h"

char * scanSpaces( char *p )
{
    char ch;

    do {
        ch = *(p++);
    } while ( isspace( ch ) );
    return p - 1;

}

char * scanNonspaces( char *p )
{
    char ch;

    do {
        ch = *(p++);
    } while ( ch && ! isspace( ch ) );
    return p - 1;

}

