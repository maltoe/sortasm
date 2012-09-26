#include <math.h>
#include <smmintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* ***************************************************************************
 * General helper functions.
 * ***************************************************************************/

#ifndef DEBUG
inline
#endif
void swap(uint32_t *arr, uint32_t i, uint32_t j)
{
    uint32_t tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
}

#ifndef DEBUG
inline
#endif
uint32_t min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

/* ***************************************************************************
 * Sorting algorithms.
 * ***************************************************************************/

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

#ifndef DEBUG
inline
#endif
void heapsort_siftdown_helper(int start, int end, uint32_t *out)
{
    int root = start;
    
    // While the root has at least one child.
    while(root * 2 + 1 <= end) {
        int child = root * 2 + 1;
        int s = root;
        
        // Check if root is smaller than child.
        if(out[s] < out[child])
            s = child;
        
        // Check if right child exists, and if it's bigger than what we're currently swapping with.
        if(child + 1 <= end && out[s] < out[child + 1])
            s = child + 1;
        
        // Check if we need to swap at all.
        if(s != root) {
            swap(out, root, s);
            root = s;
        } else
            // No swap? Done sifting down.
            break;
    }
}

#ifndef DEBUG
inline
#endif
void heapsort_heapify_helper(uint32_t n, uint32_t *out)
{
    // Start is assigned the index in a of the last parent node.   
    for(int start = (int) (n - 2) / 2; start >= 0; start--) {
        
        // Sift down the node at index start to the proper place such that all nodes below
        // the start index are in heap order.
        heapsort_siftdown_helper(start, (int) n - 1, out);

    }
}

void heapsort(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    // First place out in max-heap order.
    heapsort_heapify_helper(n, out);
    
    int end = (int) n - 1;
    while(end > 0) {
        // Swap the root(maximum value) of the heap with the last element of the heap.
        swap(out, 0, end);
        end--;
        heapsort_siftdown_helper(0, end, out);
    }
}


/*
 * aasort implemented following instructions in paper
 * "AA-Sort: A New Parallel Sorting Algorithm for Multi-Core SIMD Processors"
 * by H. Inoue et al.
 * See: www.trl.ibm.com/people/inouehrs/pdf/PACT2007-SIMDsort.pdf
 *
 * An earlier, non published version, which provides some more detail especially
 * regarding in-core step 2, can be found here:
 * http://www.research.ibm.com/trl/people/inouehrs/pdf/SPE-SIMDsort.pdf
 *
 * Conversely to the paper, we do not use thread-level concurrency.
 *
 * NOTE: We expect the number of data elements to be dividable by 16!
 *
 * Found quite some inspiration here:
 * https://github.com/herumi/opti/blob/master/intsort.hpp
 */

#ifndef DEBUG
inline
#endif
void aasort_vector_transpose(__m128 *xf) {
    __m128 tf[4];
    tf[0] = _mm_unpacklo_ps(xf[0], xf[2]);
    tf[1] = _mm_unpacklo_ps(xf[1], xf[3]);
    tf[2] = _mm_unpackhi_ps(xf[0], xf[2]);
    tf[3] = _mm_unpackhi_ps(xf[1], xf[3]);
    xf[0] = _mm_unpacklo_ps(tf[0], tf[1]);
    xf[1] = _mm_unpackhi_ps(tf[0], tf[1]);
    xf[2] = _mm_unpacklo_ps(tf[2], tf[3]);
    xf[3] = _mm_unpackhi_ps(tf[2], tf[3]);
}

#ifndef DEBUG
inline
#endif
void aasort_vector_cmpswap(__m128i *in, int i, int j)
{
    __m128i t = _mm_min_epu32(in[i], in[j]);
    in[j] = _mm_max_epu32(in[i], in[j]);
    in[i] = t;
}

#ifndef DEBUG
inline
#endif
void aasort_vector_cmpswap_skew(__m128i *in, int i, int j)
{
    __m128i x = _mm_slli_si128(in[i], 4);
    __m128i y = _mm_min_epu32(in[j], x);
    in[j] = _mm_max_epu32(in[j], x);
    in[i] = _mm_srli_si128(in[i], 12);
    in[i] = _mm_alignr_epi8(in[i], y, 4);
}

#ifndef DEBUG
inline
#endif
uint32_t aasort_is_sorted(uint32_t n, __m128i *in)
{
    uint32_t r = 1;
    for(uint32_t i = 0; r && i < (n / 4) - 1; i++)
        r &= _mm_testc_si128(in[i + 1], _mm_max_epu32(in[i], in[i + 1]));
    return r;
}

const float aasort_shrink_factor = 1.28f;
uint32_t aasort_in_core(uint32_t n, __m128i *in, __m128i *out)
{
    /*
     * (1) Sort values within each vector in ascending order.
     *
     * NOTE: Although not explicitly stated in the paper,
     * efficient data-parallel sorting requires to rearrange
     * the data of 4 vectors, i.e., sort the first elements
     * in one vector, the second elements in the next, and so on.
     */

    for(uint32_t i = 0; i < n / 4; i += 4) {
        __m128i t[4];
        __m128i *x = &in[i];
        t[0] = _mm_min_epu32(x[0], x[1]);
        t[1] = _mm_max_epu32(x[0], x[1]);
        t[2] = _mm_min_epu32(x[2], x[3]);
        t[3] = _mm_max_epu32(x[2], x[3]);
        x[0] = _mm_min_epu32(t[0], t[2]);
        x[3] = _mm_max_epu32(t[1], t[3]);
        t[0] = _mm_max_epu32(t[0], t[2]);
        t[1] = _mm_min_epu32(t[1], t[3]);
        x[1] = _mm_min_epu32(t[0], t[1]);
        x[2] = _mm_max_epu32(t[0], t[1]);

        aasort_vector_transpose((__m128*) x);
    }

    /*
     * (2) Execute combsort to sort the values into the transposed order.
     */
    
    uint32_t gap = (int) ((n / 4) / aasort_shrink_factor);
    while(gap > 1) {

        for(int i = 0; i < ((n / 4) - gap); i++)
            aasort_vector_cmpswap(in, i, i + gap);

        for(int i = ((n / 4) - gap); i < n / 4; i++)
            aasort_vector_cmpswap_skew(in, i, i + gap - (n / 4));

        gap /= aasort_shrink_factor;
    }

    /*
     * As with combsort, bubblesort is executed at the end to make sure the array
     * is sorted. However, in the pre-version of the paper the authors state that
     * they have limited the number of bubblesort iterations to 10 and would fallback
     * to a vectorized merge sort if that limit would ever be reached. Here, we simply
     * output a warning and stop the execution.
     */

    uint32_t loop_count = 0;
    const uint32_t max_loop_count = 15;
    do {
        for(uint32_t i = 0; i < (n / 4) - 1; i++)
            aasort_vector_cmpswap(in, i, i + 1);
        aasort_vector_cmpswap_skew(in, (n / 4) - 1, 0);
    } while(!aasort_is_sorted(n, in) && ++loop_count < max_loop_count);

    if(loop_count == max_loop_count) {
        printf("aasort: In-core step 2 has reached maximum loop count %u!\n", max_loop_count);
        return 0;
    }

    /*
     * (3) Reorder the values from the transposed order into the original order.
     *
     * For us, this also means copying the data into the output array.
     */

    for(uint32_t i = 0; i < n / 4; i += 4)
        aasort_vector_transpose((__m128*) &in[i]);

    for(uint32_t j = 0; j < n / 16; j++) {
        for(uint32_t i = 0; i < 4; i++) {
            out[i * (n / 16) + j] = in[i + j * 4];
        }
    }

    return 1;
}

typedef union {
    __m128i v;
    uint32_t i[4];
} m128i_u;

void aasort_vector_merge(m128i_u *a, m128i_u *b)
{
    uint32_t o[8];
    uint32_t ap = 0, bp = 0, op = 0;
    while(ap < 4 && bp < 4) {
        if((*a).i[ap] < (*b).i[bp])
            o[op++] = (*a).i[ap++];
        else 
            o[op++] = (*b).i[bp++];
    }

    while(ap < 4)
        o[op++] = (*a).i[ap++];

    while(bp < 4)
        o[op++] = (*b).i[bp++];

    (*a).v = _mm_load_si128((__m128i*) &o[0]);
    (*b).v = _mm_load_si128((__m128i*) &o[4]);
}

void aasort_out_of_core(uint32_t an, __m128i *a, uint32_t bn, __m128i *b, __m128i *out)
{
    uint32_t ap = 0, bp = 0, op = 0;
    
    __m128i vmin = a[ap++];
    __m128i vmax = b[bp++];
    while(ap < (an / 4) && bp < (bn / 4)) { 
        aasort_vector_merge((m128i_u*) &vmin, (m128i_u*) &vmax); 
        out[op++] = vmin;
        if(_mm_cvtsi128_si32(a[ap]) <= _mm_cvtsi128_si32(b[bp]))
            vmin = a[ap++];
        else
            vmin = b[bp++];
    }

    if(ap < (an / 4)) {
        aasort_vector_merge((m128i_u*) &vmin, (m128i_u*) &vmax);
        out[op++] = vmin;
        while(ap < (an / 4)) {
            vmin = a[ap++];
            aasort_vector_merge((m128i_u*) &vmin, (m128i_u*) &vmax);
            out[op++] = vmin;
        }
    } else if(bp < (bn / 4)) {
        aasort_vector_merge((m128i_u*) &vmin, (m128i_u*) &vmax);
        out[op++] = vmin;
        while(bp < (bn / 4)) {
            vmin = b[bp++];
            aasort_vector_merge((m128i_u*) &vmin, (m128i_u*) &vmax);
            out[op++] = vmin;
        }
    }

    out[op] = vmax;
}

void aasort(uint32_t n, uint32_t *in, uint32_t *out)
{
    /*
     * (1) Divide all of the data to be sorted into blocks that
     * fit in the cache or the local memory of the processor.
     */

    // As stated in the paper, we use half the L2 cache as block size.
    uint32_t l2_cache = (uint32_t) sysconf(_SC_LEVEL2_CACHE_SIZE);
    uint32_t block_size = l2_cache / 2;
    uint32_t block_elements = block_size / 4;

    for(uint32_t i = 0; i < n; i += block_elements) {
        uint32_t k = min(n - i, block_elements);

        /*
         * (2) Sort each block with the in-core sorting algorithm.
         */

        if(!aasort_in_core(k, (__m128i*) &in[i], (__m128i*) &out[i]))
            return;
    }

    /*
     * (3) Merge the sorted blocks with the out-of-core sorting algorithm.
     */

    int currently_in_in = 0;
    uint32_t *tin = out;
    uint32_t *tout = in;

    while(block_elements < n) {
        for(uint32_t i = 0; i < n; i += block_elements * 2) {
            if(n - i <= block_elements) {
                // Last block? Copy. This will get squashed in at the end.
                memcpy(&tout[i], &tin[i], (n - i) * 4);
            } else {
                // Merge two blocks.
                __m128i *a = (__m128i*) &tin[i];
                uint32_t an = block_elements;
                __m128i *b = (__m128i*) &tin[i + block_elements];
                uint32_t bn = min(n - (i + block_elements), block_elements);
                aasort_out_of_core(an, a, bn, b, (__m128i*) &tout[i]);
            }
        }

        block_elements *= 2;

        if(currently_in_in) {
            tin = out;
            tout = in;
        } else {
            tin = in;
            tout = out;
        }
        currently_in_in = !currently_in_in;
    }

    if(currently_in_in) {
        memcpy(out, in, n * 4);
    }
}
