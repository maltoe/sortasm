ifdef AVX
FLAGS_COMMON=--std=gnu99 -msse4.1 -mavx -Wall -D_AVX_
else
FLAGS_COMMON=--std=gnu99 -msse4.1 -Wall
endif

ifdef DEBUG
CFLAGS=$(FLAGS_COMMON) -O0 -g -ggdb -DDEBUG
ASMFLAGS=$(FLAGS_COMMON) -O0 -g -ggdb -DDEBUG
else
#CFLAGS=$(FLAGS_COMMON) -O2 -flto
CFLAGS=$(FLAGS_COMMON) -O2
ASMFLAGS=$(FLAGS_COMMON) -O0
endif
LDFLAGS=-lrt -lm -lpthread

include algorithms/Makefile.inc
OBJS=sortasm.o

all: sortasm

$(C_ALGORITHM_OBJS): %.o: %.c
	@ echo Compiling $@	...
	@	gcc $(CFLAGS) -c $< -o $@

$(ASM_ALGORITHM_OBJS): %.o: %.c
	@ echo Compiling $@	...
	@	gcc $(ASMFLAGS) -c $< -o $@

$(OBJS): %.o: %.c
	@ echo Compiling $@ ...
	@ gcc $(CFLAGS) -c $< -o $@

sortasm: $(C_ALGORITHM_OBJS) $(ASM_ALGORITHM_OBJS) $(OBJS)
	@ echo Linking ...
	@	gcc $(C_ALGORITHM_OBJS) $(ASM_ALGORITHM_OBJS) $(OBJS) $(LDFLAGS) -o sortasm

clean:
	rm -f *.o algorithms/*.o sortasm.s sortasm 
