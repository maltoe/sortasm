#include <pthread.h>
#include <smmintrin.h>
#include <stdio.h>
#include <string.h> /* for memcpy */
#include <unistd.h> /* for sysconf */
#include "algorithms.h"

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

static uint32_t l2_cache = 0;

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

void aasort_vector_merge(__m128i *a, __m128i *b)
{
    __m128i m = _mm_min_epu32(*a, *b); // [h:f:d:m00] = [m33:m22:m11:m00]
    __m128i M = _mm_max_epu32(*a, *b);      // [M33:g:e:c] = [M33:m22:M11:M00]
    __m128i s0 = _mm_unpackhi_epi64(m, m); // [  h:f:h:f]
    __m128i s1 = _mm_min_epu32(s0, M);    // [  h:f:u:s]
    __m128i s2 = _mm_max_epu32(s0, M);    // [M33:g:v:t]
    __m128i s3 = _mm_unpacklo_epi64(s1, _mm_unpackhi_epi64(M, M)); // [M33:g:u:s]
    __m128i s4 = _mm_unpacklo_epi64(s2, m); // [d:m00:v:t]
    s4 = _mm_shuffle_epi32(s4, _MM_SHUFFLE(2, 1, 0, 3)); // [m00:v:t:d]
    __m128i s5 = _mm_min_epu32(s3, s4); // [m00:mgv:mtu:msd]
    __m128i s6 = _mm_max_epu32(s3, s4); // [M33:Mgv:Mtu:Msd]
    __m128i s7 = _mm_insert_epi32(s5, _mm_cvtsi128_si32(s6), 2); // [m00:Msd:mtu:msd]
    __m128i s8 = _mm_insert_epi32(s6, _mm_extract_epi32(s5, 2), 0); // [M33:Mgv:Mtu:mgv]
    *a = _mm_shuffle_epi32(s7, _MM_SHUFFLE(1, 2, 0, 3));
    *b = _mm_shuffle_epi32(s8, _MM_SHUFFLE(3, 2, 0, 1));
}

void aasort_out_of_core(uint32_t an, __m128i *a, uint32_t bn, __m128i *b, __m128i *out)
{
    uint32_t ap = 0, bp = 0, op = 0;
    
    __m128i vmin = a[ap++];
    __m128i vmax = b[bp++];
    while(ap < (an / 4) && bp < (bn / 4)) { 
        aasort_vector_merge(&vmin, &vmax); 
        out[op++] = vmin;
        if(_mm_cvtsi128_si32(a[ap]) <= _mm_cvtsi128_si32(b[bp]))
            vmin = a[ap++];
        else
            vmin = b[bp++];
    }

    if(ap < (an / 4)) {
        aasort_vector_merge(&vmin, &vmax);
        out[op++] = vmin;
        while(ap < (an / 4)) {
            vmin = a[ap++];
            aasort_vector_merge(&vmin, &vmax);
            out[op++] = vmin;
        }
    } else if(bp < (bn / 4)) {
        aasort_vector_merge(&vmin, &vmax);
        out[op++] = vmin;
        while(bp < (bn / 4)) {
            vmin = b[bp++];
            aasort_vector_merge(&vmin, &vmax);
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
    if(!l2_cache)
        l2_cache = (uint32_t) sysconf(_SC_LEVEL2_CACHE_SIZE);
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

/* 
 * As AA-Sort's out-of-core algorithm is essentially mergesort, we
 * naively parallelize it using a divide-and-conquer approach. We
 * - for now - ignore the paper's advice on a parallel implementation
 * (esp. the part about threads cooperating on the out-of-core algorithm).
 */

static const uint32_t thread_depth = 2;
typedef struct {
        uint32_t td;
    uint32_t n;
    uint32_t *in;
    uint32_t *out;
} algorithm_params_st;

void* aasort_naive_parallel_run(void *data)
{
    algorithm_params_st *params = (algorithm_params_st*) data;

    if(params->n <= 1) {
        *(params->out) = *(params->in);
    } else if(params->td == 0) {
        aasort(params->n, params->in, params->out);
    }   else {
        if(!l2_cache)
            l2_cache = (uint32_t) sysconf(_SC_LEVEL2_CACHE_SIZE);
        uint32_t block_size = l2_cache / 2;
        uint32_t block_elements = block_size / 4;

        uint32_t nl = (params->n / block_elements) / 2 * block_elements;
        uint32_t nu = params->n - nl;

        pthread_t t1, t2;
        algorithm_params_st p1, p2;

        p1.td = params->td - 1;
        p1.n = nl;
        p1.in = params->out;
        p1.out = params->in;

        pthread_create(&t1, NULL, aasort_naive_parallel_run, &p1);

        p2.td = params->td - 1;
        p2.n = nu;
        p2.in = &(params->out[nl]);
        p2.out = &(params->in[nl]);

        pthread_create(&t2, NULL, aasort_naive_parallel_run, &p2);

        pthread_join(t1, NULL);
        pthread_join(t2, NULL);

        aasort_out_of_core(nl, (__m128i*) params->in, nu,
            (__m128i*) &(params->in[nl]), (__m128i*) params->out);
    }

    return NULL;
}

void aasort_naive_parallel(uint32_t n, uint32_t *in, uint32_t *out)
{
    algorithm_params_st s;
    s.td = thread_depth;
    s.n = n;
    s.in = in;
    s.out = out;
    aasort_naive_parallel_run(&s);
}
