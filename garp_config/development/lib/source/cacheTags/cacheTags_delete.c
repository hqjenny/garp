
#include <stdlib.h>
#include "platform.h"
#include "cacheTags.h"
#include "cacheTagsX.h"

void cacheTags_delete( cacheTagsT *cacheTagsPtr )
{

    free( cacheTagsPtr->tags );
    free( cacheTagsPtr->prefetchBuffers );
    free( cacheTagsPtr );

}

