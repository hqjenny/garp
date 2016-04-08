
GARP_LIB_DIR = ../..
GARP_INCLUDE_DIR = $(GARP_LIB_DIR)/include
SOURCE_DIR = $(GARP_LIB_DIR)/source/GarpArray

OBJ = .o
LIB = .a

DELETE = rm -f
INCLUDES = -I. -I$(SOURCE_DIR) -I$(GARP_INCLUDE_DIR)
COMPILE_C = gcc -c $(INCLUDES) -I- -O2 -o $@  # -g -pg
MAKELIB = ar crs $@

C_HEADERS = \
 platform.h \
 $(GARP_INCLUDE_DIR)/GarpArray.h \
 $(SOURCE_DIR)/allocate.h \
 $(SOURCE_DIR)/array.h \

OBJECTS = \
 fail_programName$(OBJ) \
 allocate$(OBJ) \
 ga_new$(OBJ) \
 ga_delete$(OBJ) \
 ga_allocRows$(OBJ) \
 ga_zero$(OBJ) \
 ga_loadConfig$(OBJ) \
 ga_branchFlag$(OBJ) \
 ga_interruptFlag$(OBJ) \
 ga_getQueueControl$(OBJ) \
 ga_setQueueControl$(OBJ) \
 ga_flushQueuedReads$(OBJ) \
 ga_pushQueuedWrite$(OBJ) \
 ga_getMemoryRequest$(OBJ) \
 ga_returnDataRead$(OBJ) \
 ga_regL$(OBJ) \
 ga_setRegL$(OBJ) \
 ga_clock$(OBJ) \
 ga_setClock$(OBJ) \
 ga_addToClock$(OBJ) \
 ga_step$(OBJ) \
 ga_dumpState$(OBJ) \
 ga_dumpConfig$(OBJ) \
 ga_writeLogicBlockStats$(OBJ) \
 ga_writeGranularityStats$(OBJ) \
 ga_writeRoutingStats$(OBJ) \

.PHONY: all clean

all: GarpArray$(LIB)

%$(OBJ): $(SOURCE_DIR)/%.c $(C_HEADERS)
	$(COMPILE_C) $<

GarpArray$(LIB): $(OBJECTS)
	$(DELETE) $@
	$(MAKELIB) $^

clean:
	$(DELETE) $(OBJECTS)
	$(DELETE) GarpArray$(LIB)

