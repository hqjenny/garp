
LIB = .a

COPY = cp -f
DELETE = rm -f

LIBS = \
 GarpArrayConfig$(LIB) \
 GarpArray$(LIB) \
 configCacheTags$(LIB) \
 cacheTags$(LIB) \

.PHONY: \
 all \
 make_GarpArrayConfig \
 make_GarpArray \
 make_configCacheTags \
 make_cacheTags \
 clean \

all: $(LIBS)

make_GarpArrayConfig:
	make -C GarpArrayConfig

make_GarpArray:
	make -C GarpArray

make_configCacheTags:
	make -C configCacheTags

make_cacheTags:
	make -C cacheTags

GarpArrayConfig/GarpArrayConfig$(LIB): make_GarpArrayConfig
GarpArray/GarpArray$(LIB): make_GarpArray
configCacheTags/configCacheTags$(LIB): make_configCacheTags
cacheTags/cacheTags$(LIB): make_cacheTags

GarpArrayConfig$(LIB): GarpArrayConfig/GarpArrayConfig$(LIB)
	$(COPY) $< $@

GarpArray$(LIB): GarpArray/GarpArray$(LIB)
	$(COPY) $< $@

configCacheTags$(LIB): configCacheTags/configCacheTags$(LIB)
	$(COPY) $< $@

cacheTags$(LIB): cacheTags/cacheTags$(LIB)
	$(COPY) $< $@

clean:
	make -C GarpArrayConfig clean
	make -C GarpArray clean
	make -C configCacheTags clean
	make -C cacheTags clean
	$(DELETE) $(LIBS)

