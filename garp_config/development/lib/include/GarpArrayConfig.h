
#ifndef GarpArrayConfig_h
#define GarpArrayConfig_h 1

#include <stdint.h>
#include <stdio.h>

#define GARP_VERSION 2

#define GARP_IN_00          0x00
#define GARP_IN_10          0x04
#define GARP_IN_ZREG        0x08
#define GARP_IN_DREG        0x0C
#define GARP_IN_V_0         0x7C
#define GARP_IN_ABOVEH_MIN  0xA8
#define GARP_IN_ABOVEG_0    0xBC
#define GARP_IN_H_MIN       0xE8
#define GARP_IN_G_0         0xFC

#define GARP_IN_XBAR_00     0x00
#define GARP_IN_XBAR_01     0x01
#define GARP_IN_XBAR_10     0x02
#define GARP_IN_XBAR_11     0x03

#define GARP_IN_SHIFT1      0x02
#define GARP_IN_INVERT      0x01

#define GARP_INPUT_A        56
#define GARP_INPUT_B        48
#define GARP_INPUT_C        40
#define GARP_INPUT_D        32

#define GARP_MODE_DIRECT    UINT64_C( 0x0000000000000000 )
#define GARP_MODE_INPUT5    UINT64_C( 0x0000000000002000 )
#define GARP_MODE_SPLIT     UINT64_C( 0x0000000100002000 )
#define GARP_MODE_SELECT    UINT64_C( 0x0000000000004000 )
#define GARP_MODE_PPSELECT  UINT64_C( 0x0000000100004000 )
#define GARP_MODE_VARSHIFT  UINT64_C( 0x0000000200004000 ) /* v. 1 only */
#define GARP_MODE_CARRY     UINT64_C( 0x0000000000008000 )
#define GARP_MODE_ADD3      UINT64_C( 0x000000000000C000 )

#define GARP_MODE_SHIFTIN_0 UINT64_C( 0x0000000000000000 )
#define GARP_MODE_SHIFTIN_X UINT64_C( 0x0000000000002000 )

#define GARP_RESULT_V       UINT64_C( 0x0000000000000000 )
#define GARP_RESULT_V_OR_K  UINT64_C( 0x0000000100000000 ) /* v. 1 only */
#define GARP_RESULT_KOUT    UINT64_C( 0x0000000100000000 ) /* v. 2, later */
#define GARP_RESULT_U_XOR_K UINT64_C( 0x0000000200000000 )
#define GARP_RESULT_U_EQU_K UINT64_C( 0x0000000300000000 )

#define GARP_TABLE_A        0xAAAA
#define GARP_TABLE_B        0xCCCC
#define GARP_TABLE_C        0xF0F0
#define GARP_TABLE_D        0xFF00
#define GARP_TABLE_ABOVEH   0xFF00

#define GARP_TABLE          16

#define GARP_UVTABLE_A      0xAA
#define GARP_UVTABLE_B      0xCC
#define GARP_UVTABLE_C      0xF0
#define GARP_UVTABLE_CARRY  0xAA
#define GARP_UVTABLE_SUM    0xCC

#define GARP_UTABLE         24
#define GARP_VTABLE         16

#define GARP_DEFAULTTABLE   UINT64_C( 0x00000000CCCC0000 )

#define GARP_BUFFERZ        UINT64_C( 0x0000000000001000 )
#define GARP_BUFFERD        UINT64_C( 0x0000000000000800 )
#define GARP_HOUT_Z         UINT64_C( 0x0000000000000000 )
#define GARP_HOUT_D         UINT64_C( 0x0000000000000400 )
#define GARP_GOUT_Z         UINT64_C( 0x0000000000000000 )
#define GARP_GOUT_D         UINT64_C( 0x0000000000000200 )
#define GARP_VOUT_Z         UINT64_C( 0x0000000000000000 )
#define GARP_VOUT_D         UINT64_C( 0x0000000000000100 )

#define GARP_OUT_G_0        0x07
#define GARP_OUT_V_0        0x1F

#define GARP_OUTPUT_G        5
#define GARP_OUTPUT_V        0

#define GARP_X_PROCESSOR    UINT64_C( 0x0000000000000002 )
#define GARP_X_MEMORY       UINT64_C( 0x0000000000000006 )
#define GARP_QUEUE          UINT64_C( 0x0000000000000000 )
#define GARP_DEMAND         UINT64_C( 0x0000000080000000 )
#define GARP_DEMANDREAD     UINT64_C( 0x0000000040000000 )

#define GARP_ALLOCATE       UINT64_C( 0x0000000000000000 )
#define GARP_NOALLOCATE     UINT64_C( 0x0000000040000000 )
#define GARP_ALIGNED        UINT64_C( 0x0000000000000000 )
#define GARP_NONALIGNED     UINT64_C( 0x0000000000200000 )

#define GARP_MATCH          16
#define GARP_LATENCY        24
#define GARP_SIZE_A         22
#define GARP_K              16
#define GARP_SIZE_B         14
#define GARP_BUS             8

#define GARP_REGBUS_Z       UINT64_C( 0x0000000000000000 )
#define GARP_REGBUS_D       UINT64_C( 0x0000000000002000 )

#define GARP_HDIR_LEFT      UINT64_C( 0x0000000000000000 )
#define GARP_HDIR_CENTER    UINT64_C( 0x0000000000000008 )
#define GARP_HDIR_RIGHT     UINT64_C( 0x0000000000000010 )

uint32_t * GarpArrayConfig_read( FILE * );

#endif

