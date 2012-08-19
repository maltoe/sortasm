ifdef DEBUG
CFLAGS=-lrt --std=gnu99 -msse4.1 -O0 -g -ggdb -DDEBUG 
else
CFLAGS=-lrt --std=gnu99 -msse4.1 -O2
endif

sortasm: sortasm.c
	gcc $(CFLAGS)  sortasm.c -o sortasm

asm: sortasm.c
	gcc $(CFLAGS) -S sortasm.c

all: sortasm

clean:
	rm -f sortasm.s sortasm
