
#ifndef fail_h
#define fail_h 1

#include <stdio.h>

extern char *fail_programName;

void writeFailMessage( FILE *, const char *, const char * );
void fail( const char *, const char * );

#endif

