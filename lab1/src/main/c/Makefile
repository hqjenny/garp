#CC = riscv-gcc
CC = gcc
#LDFLAGS=-lc
LDFLAGS=
src_files=sha3.c sha3_example.c 
OBJECTS=$(src_files:.c=.o)

#all: sha3.rv
all: sha3

#sha3.rv: Makefile $(OBJECTS)
sha3: Makefile $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

run:
	#spike pk sha3.rv
	./sha3

clean:
	#rm $(OBJECTS) sha3.rv
	rm $(OBJECTS) sha3
