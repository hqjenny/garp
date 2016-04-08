
#include <stdlib.h>
#include "platform.h"
#include "configCacheTags.h"
#include "configCacheTagsStruct.h"

void configCacheTags_delete( configCacheTagsT *configCacheTagsPtr )
{

    free( configCacheTagsPtr->rowAddresses );
    free( configCacheTagsPtr );

}

