
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "platform.h"
#include "fail.h"
#include "hexTextToUint64.h"
#include "GarpArrayConfig.h"
#include "GarpArray.h"
#include "scan.h"
#include "inputArrayConfig.h"

enum { commandBufferSize = 1024 };

#define STRINGIFY_Z( a ) #a
#define STRINGIFY( a ) STRINGIFY_Z( a )

main( int argc, char *argv[] )
{
    bool prompt, argError;
    char **argPtrPtr, *argPtr;
    int i;
    char *stopPtr;
    GarpArrayT *GarpArrayPtr;
    char commandBuffer[commandBufferSize];
    char *commandPtr, *p;
    FILE *streamPtr;
    int rowNum;
    char *arg2Ptr;
    uint_fast64_t value;

    fail_programName = "ga-emulate";
    prompt = true;
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
                fputs(
"ga-emulate [<option>...]\n"
"  <option>:  (* is default)\n"
"    -help            --Write this message and exit.\n"
"    -garpver:<n>     --Garp architecture version <n>.\n"
" *  -garpver:" STRINGIFY( GARP_VERSION ) "\n"
"    -x               --Suppress prompts\n"
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
                    ga_archVersion = strtoul( argPtr, &stopPtr, 10 );
                    if ( *stopPtr ) {
                        writeFailMessage(
                            stderr, "Invalid argument to option `-garpver'.", 0
                        );
                        argError = true;
                    }
                }
            } else if ( strcmp( argPtr, "x" ) == 0 ) {
                prompt = false;
            } else {
         invalidOption:
                writeFailMessage( stderr, "Invalid option `-%s'.", argPtr );
                argError = true;
            }
        } else {
            ++argc;
        }
    }
    if ( 1 < argc ) fail( "Invalid arguments.", 0 );
    if ( argError ) return EXIT_FAILURE;
    GarpArrayPtr = ga_new();
    for (;;) {
        if ( prompt ) fputs( "ga-emulate> ", stdout );
        if ( ! fgets( commandBuffer, commandBufferSize, stdin ) ) {
            if ( ferror( stdin ) ) fail( "Error reading input.", 0 );
            if ( feof( stdin ) ) {
                fputc( '\n', stdout );
                break;
            }
        }
        commandPtr = scanSpaces( commandBuffer );
        if ( ! *commandPtr ) continue;
        p = scanNonspaces( commandPtr );
        if ( *p ) {
            *p = 0;
            p = scanSpaces( p + 1 );
        }
        argPtr = p;
        if ( strcmp( commandPtr, "help" ) == 0 ) {
            fputs(
      "  help\n"
      "  exit\n"
      "  loadconfig <file-name>\n"
      "  writeconfig <file-name>   --Write decode of configuration to file.\n"
      "  queryall                  --Write all array state and wire outputs.\n"
      "  setz <n> <hex-value>      --Set specific Z register.\n"
      "  setd <n> <hex-value>      --Set specific D register.\n"
      "  step\n"
      "  x  = exit\n"
      "  lc = loadconfig\n"
      "  wc = writeconfig\n"
      "  qa = queryall\n"
      "  sz = setz\n"
      "  sd = setd\n"
                ,
                stdout
            );
        } else if (
               (strcmp( commandPtr, "x" ) == 0)
            || (strcmp( commandPtr, "exit" ) == 0)
        ) {
            break;
        } else if (
               (strcmp( commandPtr, "lc" ) == 0)
            || (strcmp( commandPtr, "loadconfig" ) == 0)
        ) {
            if ( ! *argPtr ) {
                fputs(
                    "File name argument to `loadconfig' expected.\n", stdout );
                continue;
            }
            p = scanNonspaces( argPtr );
            if ( *p ) {
                *p = 0;
                p = scanSpaces( p + 1 );
                if ( *p ) {
                    fputs( "Too many arguments to `loadconfig'.\n", stdout );
                    continue;
                }
            }
            streamPtr = fopen( argPtr, "r" );
            if ( ! streamPtr ) goto cannotOpenFile;
            inputArrayConfig( streamPtr, GarpArrayPtr );
            fclose( streamPtr );
        } else if (
               (strcmp( commandPtr, "wc" ) == 0)
            || (strcmp( commandPtr, "writeconfig" ) == 0)
        ) {
            if ( ! *argPtr ) {
                fputs(
                    "File name argument to `writeconfig' expected.\n", stdout
                );
                continue;
            }
            p = scanNonspaces( argPtr );
            if ( *p ) {
                *p = 0;
                p = scanSpaces( p + 1 );
                if ( *p ) {
                    fputs( "Too many arguments to `writeconfig'.\n", stdout );
                    continue;
                }
            }
            streamPtr = fopen( argPtr, "w" );
            if ( ! streamPtr ) goto cannotOpenFile;
            ga_dumpConfig( streamPtr, GarpArrayPtr );
            fclose( streamPtr );
        } else if (
               (strcmp( commandPtr, "qa" ) == 0)
            || (strcmp( commandPtr, "queryall" ) == 0)
        ) {
            if ( *argPtr ) {
                fputs( "Unexpected arguments to `queryall'.\n", stdout );
                continue;
            }
            ga_dumpState( stdout, GarpArrayPtr );
/****/
#if 0
        } else if (
               (strcmp( commandPtr, "qz" ) == 0)
            || (strcmp( commandPtr, "queryz" ) == 0)
        ) {
            if ( ! *argPtr ) {
                fputs( "Row number argument to `queryz' expected.\n", stdout );
                continue;
            }
            p = scanNonspaces( argPtr );
            if ( *p ) {
                *p = 0;
                p = scanSpaces( p + 1 );
                if ( *p ) {
                    fputs( "Too many arguments to `queryz'.\n", stdout );
                    continue;
                }
            }
            sscanf( argPtr, "%i", &rowNum );
            printf( "%3d: ", rowNum );
            writeZState( rowNum );
            fputc( '\n', stdout );
        } else if (
               (strcmp( commandPtr, "qd" ) == 0)
            || (strcmp( commandPtr, "queryd" ) == 0)
        ) {
            if ( ! *argPtr ) {
                fputs( "Row number argument to `queryd' expected.\n", stdout );
                continue;
            }
            p = scanNonspaces( argPtr );
            if ( *p ) {
                *p = 0;
                p = scanSpaces( p + 1 );
                if ( *p ) {
                    fputs( "Too many arguments to `queryd'.\n", stdout );
                    continue;
                }
            }
            sscanf( argPtr, "%i", &rowNum );
            printf( "%3d: ", rowNum );
            writeDState( rowNum );
            fputc( '\n', stdout );
#endif
        } else if (
               (strcmp( commandPtr, "sz" ) == 0)
            || (strcmp( commandPtr, "setz" ) == 0)
        ) {
            if ( ! *argPtr ) {
                fputs( "Row number argument to `setz' expected.\n", stdout );
                continue;
            }
            arg2Ptr = scanSpaces( scanNonspaces( argPtr ) );
            if ( ! *arg2Ptr ) {
                fputs( "Value argument to `setz' expected.\n", stdout );
                continue;
            }
            p = scanSpaces( scanNonspaces( arg2Ptr ) );
            if ( *p ) {
                fputs( "Too many arguments to `setz'.\n", stdout );
                continue;
            }
            rowNum = strtoul( argPtr, 0, 10 );
            value = hexTextToUint64( arg2Ptr, 0 );
            ga_setRegL( GarpArrayPtr, (rowNum<<1) + 0, 16, 0, value );
            ga_setRegL( GarpArrayPtr, (rowNum<<1) + 0, 16, 16, value>>32 );
        } else if (
               (strcmp( commandPtr, "sd" ) == 0)
            || (strcmp( commandPtr, "setd" ) == 0)
        ) {
            if ( ! *argPtr ) {
                fputs( "Row number argument to `setd' expected.\n", stdout );
                continue;
            }
            arg2Ptr = scanSpaces( scanNonspaces( argPtr ) );
            if ( ! *arg2Ptr ) {
                fputs( "Value argument to `setd' expected.\n", stdout );
                continue;
            }
            p = scanSpaces( scanNonspaces( arg2Ptr ) );
            if ( *p ) {
                fputs( "Too many arguments to `setd'.\n", stdout );
                continue;
            }
            rowNum = strtoul( argPtr, 0, 10 );
            value = hexTextToUint64( arg2Ptr, 0 );
            ga_setRegL( GarpArrayPtr, (rowNum<<1) + 1, 16, 0, value );
            ga_setRegL( GarpArrayPtr, (rowNum<<1) + 1, 16, 16, value>>32 );
        } else if ( strcmp( commandPtr, "step" ) == 0 ) {
            if ( *argPtr ) {
                fputs( "Unexpected arguments to `step'.\n", stdout );
                continue;
            }
            ga_addToClock( GarpArrayPtr, 1 );
            ga_step( GarpArrayPtr, 0, 0 );
        } else {
            fputs( "Unknown command.\n", stdout );
        }
        continue;
     cannotOpenFile:
        writeFailMessage( stderr, "Cannot open file \"%s\".", argPtr );
    }
    return 0;

}

