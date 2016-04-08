
GARP_INCLUDE_DIR = ../../lib/include
SOURCE_DIR = ../source

OBJ = .o
EXE =

PARSE_TAB_H = parse.tab.h
PARSE_TAB_C = parse.tab.c

FWRITE = cat
DELETE = rm -f
LEX = flex
YACC = bison
INCLUDES = -I. -I$(SOURCE_DIR) -I$(GARP_INCLUDE_DIR)
COMPILE_C = gcc -c $(INCLUDES) -I- -O2 -o $@  # -g -pg -pedantic
LINK = gcc -o $@

C_HEADERS = \
 platform.h \
 $(GARP_INCLUDE_DIR)/GarpArrayConfig.h \
 $(SOURCE_DIR)/fail.h \
 $(SOURCE_DIR)/allocate.h \
 $(SOURCE_DIR)/writeUint64AsText.h \
 $(SOURCE_DIR)/dialect.h \
 $(SOURCE_DIR)/errorMessages.h \
 $(SOURCE_DIR)/tokens.h \
 $(SOURCE_DIR)/names.h \
 $(SOURCE_DIR)/tokenize.h \
 $(PARSE_TAB_H) \
 $(SOURCE_DIR)/parse.h \
 $(SOURCE_DIR)/linkDottedNames.h \
 $(SOURCE_DIR)/wires.h \
 $(SOURCE_DIR)/config.h \
 $(SOURCE_DIR)/fixSyntax.h \
 $(SOURCE_DIR)/initConfig.h \
 $(SOURCE_DIR)/logicBlocks.h \
 $(SOURCE_DIR)/getModesAndCheckConsistency.h \
 $(SOURCE_DIR)/constructConfig.h \
 $(SOURCE_DIR)/allocateWires.h \
 $(SOURCE_DIR)/addWiresToConfig.h \
 $(SOURCE_DIR)/outputConfig.h \

OBJECTS = \
 fail$(OBJ) \
 allocate$(OBJ) \
 writeUint64AsText$(OBJ) \
 dialect$(OBJ) \
 errorMessages$(OBJ) \
 tokens$(OBJ) \
 names$(OBJ) \
 tokenize$(OBJ) \
 parse$(OBJ) \
 linkDottedNames$(OBJ) \
 wires$(OBJ) \
 config$(OBJ) \
 fixSyntax$(OBJ) \
 initConfig$(OBJ) \
 logicBlocks$(OBJ) \
 getModesAndCheckConsistency$(OBJ) \
 constructConfig$(OBJ) \
 allocateWires$(OBJ) \
 addWiresToConfig$(OBJ) \
 outputConfig$(OBJ) \
 gatoconfig$(OBJ) \

.PHONY: all clean

all: gatoconfig$(EXE)

tokenize.c: $(SOURCE_DIR)/tokenize.l $(C_HEADERS)
	$(LEX) -t $< >$@

tokenize$(OBJ): tokenize.c $(C_HEADERS)
	$(COMPILE_C) $<

parse.y: $(SOURCE_DIR)/parse.y.head $(SOURCE_DIR)/parse.y.tokens $(SOURCE_DIR)/parse.y.body
	$(FWRITE) $(SOURCE_DIR)/parse.y.head >parse.y
	$(FWRITE) $(SOURCE_DIR)/parse.y.tokens >>parse.y
	$(FWRITE) $(SOURCE_DIR)/parse.y.body >>parse.y

parse.output: parse.y
	$(YACC) -v $<

$(PARSE_TAB_H) $(PARSE_TAB_C): parse.y
	$(YACC) -d $<

parse$(OBJ): $(PARSE_TAB_C) $(C_HEADERS)
	$(COMPILE_C) $<

makeWireArray$(OBJ): $(SOURCE_DIR)/makeWireArray.c platform.h
	$(COMPILE_C) $<

makeWireArray$(EXE): makeWireArray$(OBJ)
	$(LINK) $^

wires-wireArray%: makeWireArray$(EXE)
	./makeWireArray$(EXE) $* >wires-wireArray$*

wires$(OBJ): \
 wires-wireArray4 \
 wires-wireArray8 \
 wires-wireArray16 \
 wires-wireArray32 \
 wires-wireArray64 \
 wires-wireArray128 \
 wires-wireArray256 \

%$(OBJ): $(SOURCE_DIR)/%.c $(C_HEADERS)
	$(COMPILE_C) $<

gatoconfig$(EXE): $(OBJECTS)
	$(LINK) $^

clean:
	$(DELETE) tokenize.c
	$(DELETE) parse.y parse.output $(PARSE_TAB_H) $(PARSE_TAB_C)
	$(DELETE) makeWireArray$(OBJ) makeWireArray$(EXE)
	$(DELETE) wires-wireArray4 wires-wireArray8 wires-wireArray16
	$(DELETE) wires-wireArray32 wires-wireArray64 wires-wireArray128
	$(DELETE) wires-wireArray256
	$(DELETE) $(OBJECTS)
	$(DELETE) gatoconfig$(EXE)

