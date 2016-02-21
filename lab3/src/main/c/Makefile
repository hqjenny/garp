CC = gcc
LDFLAGS=-lc
src_files=sha3.c sha3_example.c
OBJECTS=$(src_files:.c=.o)

all: sha3

sha3: Makefile $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

run:
	sha3

clean:
	rm $(OBJECTS) sha3
