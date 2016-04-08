
#ifndef errorMessages_h
#define errorMessages_h 1

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

extern bool debugErrorMessages;

extern uint_fast32_t errorCount;

void initErrorMessages( uint_fast32_t );

void
 markError2InColumn(
     uint_fast32_t, uint_fast16_t, const char *, const char *, int );

INLINE
 void
  markError(
      uint_fast32_t lineNum, uint_fast16_t columnNum, const char *message )
{
    markError2InColumn( lineNum, columnNum, message, 0, -1 );
}

void reportErrors( FILE *, char * );

#endif

