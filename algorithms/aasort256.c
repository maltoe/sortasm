#ifdef _AVX_

#include <avxintrin.h>

/*
 * This is an AVX-based version of AA-Sort.
 */

/*
 * Found at Keven Stock's page:
 * http://www.kevinstock.org/2011/03/avx-register-transpose.html
 */
#ifndef DEBUG
inline
#endif
void aasort256_vector_transpose(__m256 *xf)
{
    __m256 t[8], tt[8];
    t[0] = _mm256_unpacklo_ps(xf[0], xf[1]);
    t[1] = _mm256_unpackhi_ps(xf[0], xf[1]);
    t[2] = _mm256_unpacklo_ps(xf[2], xf[3]);
    t[3] = _mm256_unpackhi_ps(xf[2], xf[3]);
    t[4] = _mm256_unpacklo_ps(xf[4], xf[5]);
    t[5] = _mm256_unpackhi_ps(xf[4], xf[5]);
    t[6] = _mm256_unpacklo_ps(xf[6], xf[7]);
    t[7] = _mm256_unpackhi_ps(xf[6], xf[7]);
    tt[0] = _mm256_shuffle_ps(t[0], t[2], _MM_SHUFFLE(1,0,1,0));
    tt[1] = _mm256_shuffle_ps(t[0], t[2], _MM_SHUFFLE(3,2,3,2));
    tt[2] = _mm256_shuffle_ps(t[1], t[3], _MM_SHUFFLE(1,0,1,0));
    tt[3] = _mm256_shuffle_ps(t[1], t[3], _MM_SHUFFLE(3,2,3,2));
    tt[4] = _mm256_shuffle_ps(t[4], t[6], _MM_SHUFFLE(1,0,1,0));
    tt[5] = _mm256_shuffle_ps(t[4], t[6], _MM_SHUFFLE(3,2,3,2));
    tt[6] = _mm256_shuffle_ps(t[5], t[7], _MM_SHUFFLE(1,0,1,0));
    tt[7] = _mm256_shuffle_ps(t[5], t[7], _MM_SHUFFLE(3,2,3,2));
    xf[0] = _mm256_permute2f128_ps(tt[0], tt[4], 0x20);
    xf[1] = _mm256_permute2f128_ps(tt[1], tt[5], 0x20);
    xf[2] = _mm256_permute2f128_ps(tt[2], tt[6], 0x20);
    xf[3] = _mm256_permute2f128_ps(tt[3], tt[7], 0x20);
    xf[4] = _mm256_permute2f128_ps(tt[0], tt[4], 0x31);
    xf[5] = _mm256_permute2f128_ps(tt[1], tt[5], 0x31);
    xf[6] = _mm256_permute2f128_ps(tt[2], tt[6], 0x31);
    xf[7] = _mm256_permute2f128_ps(tt[3], tt[7], 0x31);
}

#ifndef DEBUG
inline
#endif
void aasort256_vector_cmpswap(__m256 *in, int i, int j)
{
    __m256 t = _mm256_min_ps(in[i], in[j]);
    in[j] = _mm256_max_ps(in[i], in[j]);
    in[i] = t;
}

#ifndef DEBUG
inline
#endif
uint32_t aasort256_is_sorted(uint32_t n, __m256 *in)
{
    uint32_t r = 1;
    for(uint32_t i = 0; r && i < (n / 8) - 1; i++)
        r &= _mm256_testc_ps(in[i + 1], _mm256_max_ps(in[i], in[i + 1]));
    return r;
}

const float aasort256_shrink_factor = 1.28f;
uint32_t aasort256_in_core(uint32_t n, __m256 *in, __m256 *out)
{
    const uint32_t nv = n / 8;
    
    /*
     * (1) Sort values within each vector in ascending order.
     *
     * NOTE: Although not explicitly stated in the paper,
     * efficient data-parallel sorting requires to rearrange
     * the data of 4 vectors, i.e., sort the first elements
     * in one vector, the second elements in the next, and so on.
     */

    for(uint32_t i = 0; i < nv; i += 8) {
        __m256 t[8];
        __m256 *x = &in[i];

        /* TODO: optimal min/max based sorting

        aasort256_vector_transpose(x);
    }

    /*
     * (2) Execute combsort to sort the values into the transposed order.
     */
    
    uint32_t gap = (int) (nv / aasort256_shrink_factor);
    while(gap > 1) {

        for(int i = 0; i < (nv - gap); i++)
            aasort256_vector_cmpswap(in, i, i + gap);

        for(int i = (nv - gap); i < nv; i++)
            aasort256_vector_cmpswap_skew(in, i, i + gap - nv);

        gap /= aasort256_shrink_factor;
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
        for(uint32_t i = 0; i < nv - 1; i++)
            aasort256_vector_cmpswap(in, i, i + 1);
        aasort256_vector_cmpswap_skew(in, nv - 1, 0);
    } while(!aasort256_is_sorted(n, in) && ++loop_count < max_loop_count);

    if(loop_count == max_loop_count) {
        printf("aasort256: In-core step 2 has reached maximum loop count %u!\n", max_loop_count);
        return 0;
    }

    /*
     * (3) Reorder the values from the transposed order into the original order.
     *
     * For us, this also means copying the data into the output array.
     */

    for(uint32_t i = 0; i < nv; i += 8)
        aasort256_vector_transpose(&in[i]);

    for(uint32_t j = 0; j < n / 64; j++) {
        for(uint32_t i = 0; i < 8; i++) {
            out[i * (n / 64) + j] = in[i + j * 8];
        }
    }

    return 1;
}

#endif /* AVX */
