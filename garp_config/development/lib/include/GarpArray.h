
#ifndef GarpArray_h
#define GarpArray_h 1

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*----------------------------------------------------------------------------
Changes to function modes:
 - Variable shift mode (mode bits = 01x, mx bits = 10):  This mode does not
   exist after version 1.
 - Select mode (mode bits = 01x, mx bits = 00):  In version 1, ``Hout
   above'' is selected when C = 10, and D is selected when C = 11.  Starting
   with version 2, these are reversed:  D is selected when C = 10, and
   ``Hout above'' is selected when C = 11.
 - Carry chain mode (mode bits = 10x) and triple add mode (mode bits = 11x):
   In version 1, result function 01 is Z = V|K.  Starting with version 2,
   result function 01 is Z = Kout = (U&K)|(~U&V).
*----------------------------------------------------------------------------*/
extern int ga_archVersion;

typedef struct GarpArrayS GarpArrayT;

enum {
    ga_maxNumRows = 1024,
    ga_rowConfigSize = 192,
    ga_numQueues = 3
};

GarpArrayT * ga_new( void );
void ga_delete( GarpArrayT * );

void ga_allocRows( GarpArrayT *, int );
void ga_zero( GarpArrayT * );
void ga_loadConfig( GarpArrayT *, int, int, const uint32_t * );

int ga_branchFlag( GarpArrayT * );
int ga_interruptFlag( GarpArrayT * );

enum { ga_read = 0, ga_write = 1 };
enum { ga_allocate = 1, ga_prefetch = 4 };
enum {
    ga_1 = 0, ga_2 = 1, ga_4 = 2, ga_8 = 3, ga_16 = 4, ga_32 = 5, ga_64 = 6
};

typedef struct {
    int8_t enable, direction, attributes, reserved1, size, count;
    uint32_t address, reserved2;
    int8_t busNums[4];
} ga_queueControlT;

typedef struct {
    int8_t enable, direction, attributes, source, size;
    uint32_t address;
    uint32_t data[16];
} ga_memoryRequestT;

void ga_getQueueControl( GarpArrayT *, int, ga_queueControlT * );
void ga_setQueueControl( GarpArrayT *, int, const ga_queueControlT * );
void ga_flushQueuedReads( GarpArrayT * );
int ga_pushQueuedWrite( GarpArrayT * );
void ga_getMemoryRequest( GarpArrayT *, ga_memoryRequestT * );
void ga_returnDataRead( GarpArrayT *, int, int, const uint32_t *, uint32_t );

int_fast32_t ga_regL( GarpArrayT *, int, int );
void ga_setRegL( GarpArrayT *, int, int, int, int_fast32_t );
/*** NOT SUPPORTED: ***/
//void ga_setRegsByMatchCode( GarpArrayT *, int, int, int );

#define ga_reg( GarpArrayPtr, regNum ) ga_regL( GarpArrayPtr, regNum, 4 )
#define ga_setReg( GarpArrayPtr, regNum, a ) ga_setRegL( GarpArrayPtr, regNum, 16, 4, a )

uint32_t ga_clock( GarpArrayT * );
void ga_setClock( GarpArrayT *, uint32_t );
void ga_addToClock( GarpArrayT *, int_fast32_t );
void ga_step( GarpArrayT *, uint32_t, bool );

void ga_dumpState( FILE *, GarpArrayT * );
void ga_dumpConfig( FILE *, GarpArrayT * );

void ga_writeLogicBlockStats( FILE *, GarpArrayT * );
void ga_writeGranularityStats( FILE *, GarpArrayT * );
void ga_writeRoutingStats( FILE *, GarpArrayT * );

#endif

