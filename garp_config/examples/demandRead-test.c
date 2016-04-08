
#include <stdio.h>

void loadConfig_demandRead( void );
void demandRead( void * );

typedef unsigned int bits32;

bits32 config_demandRead[] =
#include "demandRead.config"
;

main()
{
    bits32 a;

    loadConfig_demandRead();
    a = 0x12345678;
    demandRead( &a );
    return 0;

}

