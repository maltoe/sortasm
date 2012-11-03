#ifndef __ALGORITHMS_H__
#define __ALGORITHMS_H__

#include <stdint.h>

/* ***************************************************************************
 * Prototypes.
 * ***************************************************************************/

void insertionsort(uint32_t, uint32_t*, uint32_t*);
void bubblesort(uint32_t, uint32_t*, uint32_t*);
void gnomesort(uint32_t, uint32_t*, uint32_t*);
void gnomesort_rewrite(uint32_t, uint32_t*, uint32_t*);
void combsort(uint32_t, uint32_t*, uint32_t*);
void mergesort(uint32_t, uint32_t*, uint32_t*);
void mergesort_parallel(uint32_t, uint32_t*, uint32_t*);
void quicksort_recursive(uint32_t, uint32_t*, uint32_t*);
void quicksort_iterative(uint32_t, uint32_t*, uint32_t*);
void quicksort_naive_parallel(uint32_t, uint32_t*, uint32_t*);
void heapsort(uint32_t, uint32_t*, uint32_t*);
void aasort(uint32_t, uint32_t*, uint32_t*);
void aasort_naive_parallel(uint32_t, uint32_t*, uint32_t*);

void insertionsort_asm(uint32_t, uint32_t*, uint32_t*);
void bubblesort_asm(uint32_t, uint32_t*, uint32_t*);
void gnomesort_asm(uint32_t, uint32_t*, uint32_t*);
void combsort_asm(uint32_t, uint32_t*, uint32_t*);
void quicksort_iterative_asm(uint32_t, uint32_t*, uint32_t*);
void heapsort_asm(uint32_t, uint32_t*, uint32_t*);

/* ***************************************************************************
 * C helper functions.
 * ***************************************************************************/
void swap(uint32_t *arr, uint32_t i, uint32_t j);
uint32_t min(uint32_t a, uint32_t b);

#endif /* __ALGORITHMS_H__ */
