#include "algorithms.h"

void insertionsort(uint32_t n, uint32_t *in, uint32_t *out)
{
    for(int i = 0; i < n; i++) {
        int inserted = 0;
        for(int j = 0; j < i; j++) {
            if(in[i] < out[j]) {
                // Move & insert.
                for(int k = (int) i - 1; k >= j; k--)
                    out[k + 1] = out[k];
                out[j] = in[i];
                inserted = 1;
                break;
            }
        }
        if(!inserted)
            out[i] = in[i];
    }
}
