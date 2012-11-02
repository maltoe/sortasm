#include <math.h> /* for log */
#include <stdlib.h>
#include "algorithms.h"

#ifndef DEBUG
inline 
#endif
uint32_t quicksort_partition_helper(uint32_t n, uint32_t *out)
{
    uint32_t pivot = out[n - 1];
    uint32_t store_idx = 0;
    for(uint32_t i = 0; i <  n - 1; i++) {
        if(out[i] < pivot) {
            swap(out, i, store_idx);
            store_idx++;
        }
    }
    out[n - 1] = out[store_idx];
    out[store_idx] = pivot;
    
    return store_idx;
}

void quicksort_recursive(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    if(n <= 1)
        return;
    
    uint32_t store_idx = quicksort_partition_helper(n, out);
    
    quicksort_recursive(store_idx, NULL, out);
    quicksort_recursive(n - store_idx - 1, NULL, &out[store_idx + 1]);
}

void quicksort_iterative(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    const uint32_t max_stack_size = ((int) log(n)/log(2)) + 1;
    uint32_t stack_count;
    uint32_t *stack_n = malloc(4 * max_stack_size);
    uint32_t **stack_out = malloc(sizeof(uint32_t*) * max_stack_size);
    
    stack_count = 1;
    stack_n[0] = n;
    stack_out[0] = out;
    
    while(stack_count > 0) {
        uint32_t cur_n = stack_n[stack_count - 1];
        uint32_t *cur_out = stack_out[stack_count - 1];
        stack_count--;
        
        if(cur_n <= 1)
            continue;
        
        uint32_t store_idx = quicksort_partition_helper(cur_n, cur_out);
        
        // Continue with the smaller part (i.e., push it onto stack last).
        // This way we can ensure that the stack size will not exceed log2(n).
        // See: http://stackoverflow.com/questions/6709055/quicksort-stack-size
        if(store_idx <= cur_n / 2) {
            stack_n[stack_count] = cur_n - store_idx - 1;
            stack_out[stack_count] = &cur_out[store_idx + 1];
            stack_count++;
            stack_n[stack_count] = store_idx;
            stack_out[stack_count] = cur_out;
            stack_count++;
        } else {
            stack_n[stack_count] = store_idx;
            stack_out[stack_count] = cur_out;
            stack_count++;
            stack_n[stack_count] = cur_n - store_idx - 1;
            stack_out[stack_count] = &cur_out[store_idx + 1];
            stack_count++;
        }
    }

    free(stack_n);
    free(stack_out);
}
