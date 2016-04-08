
GARP_LIB_DIR = ../..
GARP_INCLUDE_DIR = $(GARP_LIB_DIR)/include
SOURCE_DIR = $(GARP_LIB_DIR)/source/cacheTags

OBJ = .o
LIB = .a

DELETE = rm -f
INCLUDES = -I. -I$(SOURCE_DIR) -I$(GARP_INCLUDE_DIR)
COMPILE_C = gcc -c $(INCLUDES) -I- -O2 -o $@  # -g -pg
MAKELIB = ar crs $@

C_HEADERS = \
 platform.h \
 $(GARP_INCLUDE_DIR)/cacheTags.h \
 $(SOURCE_DIR)/allocate.h \
 $(SOURCE_DIR)/cacheTagsX.h \

OBJECTS = \
 fail_programName$(OBJ) \
 allocate$(OBJ) \
 cacheTags_new$(OBJ) \
 cacheTags_delete$(OBJ) \
 serviceIncomingPrefetches$(OBJ) \
 cacheTags_read$(OBJ) \
 cacheTags_prefetch$(OBJ) \
 cacheTags_extendPrefetchLatency$(OBJ) \
 cacheTags_write$(OBJ) \

.PHONY: all clean

all: cacheTags$(LIB)

%$(OBJ): $(SOURCE_DIR)/%.c $(C_HEADERS)
	$(COMPILE_C) $<

cacheTags$(LIB): $(OBJECTS)
	$(DELETE) $@
	$(MAKELIB) $^

clean:
	$(DELETE) $(OBJECTS)
	$(DELETE) cacheTags$(LIB)

