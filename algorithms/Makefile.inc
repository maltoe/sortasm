C_ALGORITHMS=insertionsort.o bubblesort.o gnomesort.o combsort.o mergesort.o quicksort.o heapsort.o aasort.o helper.o
ASM_ALGORITHMS=insertionsort_asm.o bubblesort_asm.o gnomesort_asm.o combsort_asm.o quicksort_asm.o heapsort_asm.o

C_ALGORITHM_OBJS=$(addprefix algorithms/, $(C_ALGORITHMS))
ASM_ALGORITHM_OBJS=$(addprefix algorithms/, $(ASM_ALGORITHMS))
