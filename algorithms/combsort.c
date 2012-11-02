#include "algorithms.h"

const float combsort_shrink_factor = 1.24733095F;
void combsort(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    uint32_t gap = n;
    uint32_t swapped = 0;

    while((gap > 1) || swapped) {
        if(gap > 1)
            gap = (uint32_t) ((double) gap / combsort_shrink_factor);
         
        swapped = 0;
        for (uint32_t i = 0; gap + i < n; i++) {
            if (out[i] > out[i + gap]) {
                swap(out, i, i + gap);
                swapped = 1;
            }
        }
    }
}
