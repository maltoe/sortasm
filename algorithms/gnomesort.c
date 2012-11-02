#include "algorithms.h"

void gnomesort(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    uint32_t i = 1;
    while(i < n) {
        if(out[i] >= out[i - 1])
            i++;
        else {
            swap(out, i, i - 1);
            
            if(i > 1)
                i--;
            else
                i++;
        }
    }
}

void gnomesort_rewrite(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    uint32_t i = 0;
    while(i < n) {
        i++;
        if(out[i] < out[i - 1]) {
            swap(out, i, i - 1);

            if(i > 1)
                i -= 2;
        }
    }
}
