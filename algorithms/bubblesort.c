#include "algorithms.h"

void bubblesort(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    for(uint32_t i = n; i > 1; i--) {
        for(uint32_t j = 0; j < i - 1; j++) {
            if(out[j] > out[j + 1])
                swap(out, j, j + 1);
        }
    }
}
