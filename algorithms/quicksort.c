#include <pthread.h>
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


const uint32_t num_threads = 8;
typedef struct {
    uint32_t n;
    uint32_t *in;
    uint32_t *out;
} algorithm_params_st;

void* quicksort_naive_parallel_run(void *data) {
    algorithm_params_st *params = (algorithm_params_st*) data;
    quicksort_iterative(params->n, params->in, params->out);
    return NULL;
}

void quicksort_naive_parallel(uint32_t n, uint32_t *in, uint32_t *out)
{
    uint32_t stack_n[num_threads];
    uint32_t *stack_out[num_threads];
    stack_n[0] = n;
    stack_out[0] = out;
    
    for(uint32_t i = 1; i < num_threads; i *= 2) {
        for(uint32_t j = 0; j < i; j++) {
            uint32_t cur_n = stack_n[j];
            uint32_t *cur_out = stack_out[j];
            uint32_t store_idx = quicksort_partition_helper(cur_n, cur_out);
            stack_n[j] = store_idx;
            // stack_out[j] = cur_out;
            stack_n[i + j] = cur_n - store_idx - 1;
            stack_out[i + j] = &cur_out[store_idx + 1];
        }
    }

    pthread_t threads[num_threads];
    algorithm_params_st params[num_threads];
    for(uint32_t i = 0; i < num_threads; i++) {
        params[i].n = stack_n[i];
        params[i].in = stack_out[i];
        params[i].out = stack_out[i];
        pthread_create(&threads[i], NULL, quicksort_naive_parallel_run, &params[i]);
    }

    for(uint32_t i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

