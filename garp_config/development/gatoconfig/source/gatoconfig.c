
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "platform.h"
#include "fail.h"
#include "allocate.h"
#include "dialect.h"
#include "errorMessages.h"
#include "tokens.h"
#include "names.h"
#include "tokenize.h"
#include "parse.h"
#include "linkDottedNames.h"
#include "wires.h"
#include "config.h"
#include "fixSyntax.h"
#include "initConfig.h"
#include "getModesAndCheckConsistency.h"
#include "constructConfig.h"
#include "allocateWires.h"
#include "addWiresToConfig.h"
#include "outputConfig.h"

#define STRINGIFY_Z( a ) #a
#define STRINGIFY( a ) STRINGIFY_Z( a )

main( int argc, char *argv[] )
{
    int maxNumErrors;
    bool EmacsErrors;
    bool dumpTreeAfterParse, dumpTreeAfterNames, dumpTreeAfterSyntax;
    bool dumpConfigAfterInit, dumpConfigAfterModes, dumpConfigAfterConfig;
    bool dumpWiresAfterConfig, dumpConfigAfterAllocate;
    bool argError;
    char **argPtrPtr, *argPtr;
    int i;
    char *stopPtr;
    char *fileName;
    int baseNameLength;
    FILE *streamPtr;

    fail_programName = "gatoconfig";
    if ( argc <= 1 ) goto writeHelpMessage;
    maxNumErrors = 10;
    EmacsErrors = false;
    debugErrorMessages = false;
    dumpTreeAfterParse = false;
    dumpTreeAfterNames = false;
    dumpTreeAfterSyntax = false;
    dumpConfigAfterInit = false;
    dumpConfigAfterModes = false;
    dumpConfigAfterConfig = false;
    dumpWiresAfterConfig = false;
    dumpConfigAfterAllocate = false;
    argError = false;
    argPtrPtr = &argv[1];
    argc = 1;
    for (;;) {
        argPtr = *argPtrPtr++;
        if ( ! argPtr ) break;
        if ( argPtr[0] == '-' ) {
            ++argPtr;
            if (
                   (strcmp( argPtr, "h" ) == 0)
                || (strcmp( argPtr, "help" ) == 0)
                || (strcmp( argPtr, "-help" ) == 0)
            ) {
 writeHelpMessage:
                fputs(
"gatoconfig [<option>...] <source>\n"
"  <option>:  (* is default)\n"
"    -help            --Write this message and exit.\n"
"    -version         --Write version information and exit.\n"
"    -garpver:<n>     --Garp architecture version <n>.\n"
" *  -garpver:" STRINGIFY( DEFAULT_GARP_VERSION ) "\n"
"    -errors:<n>      --Exit after writing <n> error messages.\n"
" *  -errors:10\n"
"    -stderr          --Write errors in brief form to the standard error stream.\n"
                    ,
                    stdout
                );
                return 0;
            } else if ( strcmp( argPtr, "version" ) == 0 ) {
                fputs(
                    "Garp Configurator\n"
                    "By John R. Hauser\n"
                    VERSION_EXECUTABLE_TYPE "\n"
                    "Not for general distribution\n"
                    ,
                    stdout
                );
                return 0;
            } else if ( memcmp( argPtr, "garpver", 7 ) == 0 ) {
                i = argPtr[7];
                if ( i ) {
                    if ( i != ':' ) goto invalidOption;
                    argPtr += 8;
                } else {
                    argPtr = *argPtrPtr++;
                    if ( ! argPtr ) goto missingArg_garpver;
                }
                if ( ! *argPtr ) {
             missingArg_garpver:
                    writeFailMessage(
                        stderr, "Argument needed for option `-garpver'.", 0 );
                    argError = true;
                    if ( ! argPtr ) break;
                } else {
                    GarpVersion = strtoul( argPtr, &stopPtr, 10 );
                    if ( *stopPtr ) {
                        writeFailMessage(
                            stderr, "Invalid argument to option `-garpver'.", 0
                        );
                        argError = true;
                    }
                }
            } else if ( memcmp( argPtr, "errors", 6 ) == 0 ) {
                i = argPtr[6];
                if ( i ) {
                    if ( i != ':' ) goto invalidOption;
                    argPtr += 7;
                } else {
                    argPtr = *argPtrPtr++;
                    if ( ! argPtr ) goto missingArg_errors;
                }
                if ( ! *argPtr ) {
             missingArg_errors:
                    writeFailMessage(
                        stderr, "Argument needed for option `-errors'.", 0 );
                    argError = true;
                    if ( ! argPtr ) break;
                } else {
                    maxNumErrors = strtoul( argPtr, &stopPtr, 10 );
                    if ( *stopPtr ) {
                        writeFailMessage(
                            stderr, "Invalid argument to option `-errors'.", 0
                        );
                        argError = true;
                    }
                }
            } else if ( strcmp( argPtr, "stderr" ) == 0 ) {
                EmacsErrors = true;
            } else if ( strcmp( argPtr, "dscan" ) == 0 ) {
                tokenize_debug = true;
            } else if ( strcmp( argPtr, "derrors" ) == 0 ) {
                debugErrorMessages = true;
            } else if ( strcmp( argPtr, "dparse" ) == 0 ) {
                dumpTreeAfterParse = true;
            } else if ( strcmp( argPtr, "dnames" ) == 0 ) {
                dumpTreeAfterNames = true;
            } else if ( strcmp( argPtr, "dsyntax" ) == 0 ) {
                dumpTreeAfterSyntax = true;
            } else if ( strcmp( argPtr, "dinit" ) == 0 ) {
                dumpConfigAfterInit = true;
            } else if ( strcmp( argPtr, "dmodes" ) == 0 ) {
                dumpConfigAfterModes = true;
            } else if ( strcmp( argPtr, "dconfig" ) == 0 ) {
                dumpConfigAfterConfig = true;
            } else if ( strcmp( argPtr, "dwires" ) == 0 ) {
                dumpWiresAfterConfig = true;
            } else if ( strcmp( argPtr, "dallocate" ) == 0 ) {
                dumpConfigAfterAllocate = true;
            } else {
         invalidOption:
                writeFailMessage( stderr, "Invalid option `-%s'.", argPtr );
                argError = true;
            }
        } else {
            argv[argc++] = argPtr;
        }
    }
    if ( 3 < argc ) fail( "Too many arguments.", 0 );
    if ( argError ) return EXIT_FAILURE;
    argv[argc] = 0;
    fileName = argv[1];
    if ( ! fileName ) fail( "No source file name.", 0 );
    if ( ! argv[2] ) {
        baseNameLength = strlen( fileName ) - 3;
        if (
            (baseNameLength <= 0) || strcmp( fileName + baseNameLength, ".ga" )
        ) {
            fail( "Destination file name needed.", 0 );
        }
    }
    if ( ! freopen( fileName, "r", stdin ) ) {
        fail( "Cannot open source file \"%s\".", fileName );
    }
    initErrorMessages( maxNumErrors );
    initNames();
    yyparse();
    if ( dumpTreeAfterParse ) {
        dumpTokenTree( parseTokenTree );
        if ( ! errorCount ) return 0;
        goto error;
    }
    linkDottedNames( parseTokenTree );
    if ( dumpTreeAfterNames ) {
        dumpTokenTree( parseTokenTree );
        if ( ! errorCount ) return 0;
        goto error;
    }
    fixSyntax( parseTokenTree );
    if ( dumpTreeAfterSyntax ) {
        dumpTokenTree( parseTokenTree );
        if ( ! errorCount ) return 0;
        goto error;
    }
    initConfig( parseTokenTree );
    if ( dumpConfigAfterInit ) {
        dumpConfig();
        return 0;
    }
    getModesAndCheckConsistency( parseTokenTree );
    if ( dumpConfigAfterModes ) {
        dumpConfig();
        if ( ! errorCount ) return 0;
        goto error;
    }
    constructConfig( parseTokenTree );
    if ( dumpConfigAfterConfig ) {
        dumpConfig();
        if ( ! errorCount ) return 0;
        goto error;
    }
    initWireArray();
    if ( dumpWiresAfterConfig ) {
        dumpWireNeeds();
        if ( ! errorCount ) return 0;
        goto error;
    }
    if ( ! allocateWires() ) {
        writeFailMessage(
            stderr, "Cannot satisfy all vertical wire needs.", 0 );
        return 2;
    }
    addWiresToConfig( parseTokenTree );
    if ( dumpConfigAfterAllocate ) {
        dumpConfig();
        if ( ! errorCount ) return 0;
        goto error;
    }
    if ( errorCount ) goto error;
    fileName = argv[2];
    if ( ! fileName ) {
        fileName = allocate( baseNameLength + 8 );
        memcpy( fileName, argv[1], baseNameLength + 1 );
        memcpy( fileName + baseNameLength + 1, "config", 7 );
    }
    streamPtr = fopen( fileName, "w" );
    if ( ! streamPtr ) {
        fail( "Cannot open destination file \"%s\".", fileName );
    }
    outputConfig( streamPtr );
    fclose( streamPtr );
    return 0;
 error:
    reportErrors(
        EmacsErrors || (fseek( stdin, 0L, 0 ) != 0) ? 0 : stdin, fileName );
    return 1;

}

