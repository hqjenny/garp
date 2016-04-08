
#ifndef logicBlocks_h
#define logicBlocks_h 1

#include <stdbool.h>

struct tokenS;

bool logicBlockCoversCol( struct tokenS *, int );
int colOffsetInIndexList( struct tokenS *, int );

#endif

