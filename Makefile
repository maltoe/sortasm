CFLAGS_COMMON=--std=gnu99 -msse4.1 -Wall
ifdef DEBUG
CFLAGS=$(CFLAGS_COMMON) -O0 -g -ggdb -DDEBUG 
else
CFLAGS=$(CFLAGS_COMMON) -O2
endif
LDFLAGS=-lrt

sortasm: sortasm.c
	gcc $(CFLAGS) $(LDFLAGS) sortasm.c -o sortasm

asm: sortasm.c
	gcc $(CFLAGS) -S sortasm.c

all: sortasm

clean:
	rm -f sortasm.s sortasm
