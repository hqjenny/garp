
GARP_LIB_DIR = ../..
GARP_INCLUDE_DIR = $(GARP_LIB_DIR)/include
SOURCE_DIR = $(GARP_LIB_DIR)/source/GarpArrayConfig

OBJ = .o
LIB = .a

DELETE = rm -f
INCLUDES = -I. -I$(SOURCE_DIR) -I$(GARP_INCLUDE_DIR)
COMPILE_C = gcc -c $(INCLUDES) -I- -O2 -o $@  # -g -pg
MAKELIB = ar crs $@

C_HEADERS = \
 platform.h \
 $(GARP_INCLUDE_DIR)/GarpArrayConfig.h \
 $(SOURCE_DIR)/allocate.h \

OBJECTS = \
 fail_programName$(OBJ) \
 allocate$(OBJ) \
 GarpArrayConfig_read$(OBJ) \

.PHONY: all clean

all: GarpArrayConfig$(LIB)

%$(OBJ): $(SOURCE_DIR)/%.c $(C_HEADERS)
	$(COMPILE_C) $<

GarpArrayConfig$(LIB): $(OBJECTS)
	$(DELETE) $@
	$(MAKELIB) $^

clean:
	$(DELETE) $(OBJECTS)
	$(DELETE) GarpArrayConfig$(LIB)

