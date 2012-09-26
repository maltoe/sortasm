FLAGS_COMMON=--std=gnu99 -msse4.1 -Wall
ifdef DEBUG
CFLAGS=$(FLAGS_COMMON) -O0 -g -ggdb -DDEBUG
else
CFLAGS=$(FLAGS_COMMON) -O2
endif
ASMFLAGS=$(FLAGS_COMMON) -O0
LDFLAGS=-lrt -lm

sortasm: sortasm.c algorithms_c.c algorithms_asm.c
	gcc $(CFLAGS) algorithms_c.c -c -o algorithms_c.o
	gcc $(ASMFLAGS) algorithms_asm.c -c -o algorithms_asm.o
	gcc $(CFLAGS) sortasm.c -c -o sortasm.o
	gcc $(LDFLAGS) algorithms_c.o algorithms_asm.o sortasm.o -o sortasm

asm: algorithms_c.c algorithms_asm.c
	gcc $(CFLAGS) -S algorithms_c.c
	gcc $(CFLAGS) -S algorithms_asm.c

all: sortasm

clean:
	rm -f *.o sortasm.s sortasm
