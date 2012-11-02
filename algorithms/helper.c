#include "algorithms.h"

void swap(uint32_t *arr, uint32_t i, uint32_t j)
{
    uint32_t tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
}

uint32_t min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}
