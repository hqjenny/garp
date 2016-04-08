
GARP_LIB_DIR = ../..
GARP_INCLUDE_DIR = $(GARP_LIB_DIR)/include
SOURCE_DIR = $(GARP_LIB_DIR)/source/configCacheTags

OBJ = .o
LIB = .a

DELETE = rm -f
INCLUDES = -I. -I$(SOURCE_DIR) -I$(GARP_INCLUDE_DIR)
COMPILE_C = gcc -c $(INCLUDES) -I- -O2 -o $@  # -g -pg
MAKELIB = ar crs $@

C_HEADERS = \
 platform.h \
 $(GARP_INCLUDE_DIR)/configCacheTags.h \
 $(SOURCE_DIR)/allocate.h \
 $(SOURCE_DIR)/configCacheTagsStruct.h \

OBJECTS = \
 fail_programName$(OBJ) \
 allocate$(OBJ) \
 configCacheTags_new$(OBJ) \
 configCacheTags_delete$(OBJ) \
 configCacheTags_allocRows$(OBJ) \
 configCacheTags_loadConfig$(OBJ) \

.PHONY: all clean

all: configCacheTags$(LIB)

%$(OBJ): $(SOURCE_DIR)/%.c $(C_HEADERS)
	$(COMPILE_C) $<

configCacheTags$(LIB): $(OBJECTS)
	$(DELETE) $@
	$(MAKELIB) $^

clean:
	$(DELETE) $(OBJECTS)
	$(DELETE) configCacheTags$(LIB)

