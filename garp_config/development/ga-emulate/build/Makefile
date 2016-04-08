
GARP_LIB_DIR = ../../lib
GARP_INCLUDE_DIR = $(GARP_LIB_DIR)/include
SOURCE_DIR = ../source

OBJ = .o
LIB = .a
EXE =

DELETE = rm -f
INCLUDES = -I. -I$(SOURCE_DIR) -I$(GARP_INCLUDE_DIR)
COMPILE_C = gcc -c $(INCLUDES) -I- -O2 -o $@  # -g -pg -pedantic
LINK = gcc -o $@

C_HEADERS = \
 platform.h \
 $(GARP_INCLUDE_DIR)/GarpArrayConfig.h \
 $(GARP_INCLUDE_DIR)/GarpArray.h \
 $(SOURCE_DIR)/fail.h \
 $(SOURCE_DIR)/hexTextToUint64.h \
 $(SOURCE_DIR)/scan.h \
 $(SOURCE_DIR)/inputArrayConfig.h \

OBJECTS = \
 fail$(OBJ) \
 hexTextToUint64$(OBJ) \
 scan$(OBJ) \
 inputArrayConfig$(OBJ) \
 ga-emulate$(OBJ) \

LIBRARIES = \
 $(GARP_LIB_DIR)/build/GarpArrayConfig$(LIB) \
 $(GARP_LIB_DIR)/build/GarpArray$(LIB) \

.PHONY: all clean

all: ga-emulate$(EXE)

%$(OBJ): $(SOURCE_DIR)/%.c $(C_HEADERS)
	$(COMPILE_C) $<

ga-emulate$(EXE): $(OBJECTS) $(LIBRARIES)
	$(LINK) $^

clean:
	$(DELETE) $(OBJECTS)
	$(DELETE) ga-emulate$(EXE)

