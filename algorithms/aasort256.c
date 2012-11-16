#ifdef _AVX_

#include <stdint.h>
#include <stdio.h>
#include <immintrin.h>
#include <unistd.h>

#include "algorithms.h"

/*
 * This is an AVX-based version of AA-Sort.
 */

static uint32_t l2_cache = 0;

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

/* 
 * Replacements for missing AVX shift-left instructions.
 *
 * Found here: http://masm32.com/board/index.php?topic=465.0
 */
#ifndef DEBUG
inline
#endif
__m256 _mm256_slli_ps(__m256 arg, int count)
{
    __m128 arg_low =  _mm256_castps256_ps128(arg);
    __m128 arg_hi =   _mm256_extractf128_ps(arg, 1);
    __m128 newlow = (__m128)_mm_slli_epi32((__m128i)arg_low, count);
    __m128 newhi  = (__m128)_mm_slli_epi32((__m128i)arg_hi,  count);
    __m256 result = _mm256_castps128_ps256(newlow);
    result = _mm256_insertf128_ps(result,  newhi, 1);
    return result;
}

#ifndef DEBUG
inline
#endif
__m256 _mm256_srli_ps(__m256 arg, int count)
{
    __m128 arg_low =  _mm256_castps256_ps128(arg);
    __m128 arg_hi =   _mm256_extractf128_ps(arg, 1);

    __m128 newlow = (__m128)_mm_srli_epi32((__m128i)arg_low, count);
    __m128 newhi  = (__m128)_mm_srli_epi32((__m128i)arg_hi,  count);

    __m256 result = _mm256_castps128_ps256(newlow);
    result = _mm256_insertf128_ps(result,  newhi, 1);
    return result;
}

/*
 * Replacements for missing AVX palignr instruction.
 *
 * Found here:
 */
#ifndef DEBUG
inline
#endif
__m256 _mm256_alignr_epi8(__m256 v0, __m256 v1, const int n)
{
    float buffer[16] __attribute__((aligned(32)));

    // Two aligned stores to fill the buffer.
    _mm256_store_ps(buffer, v0);
    _mm256_store_ps(&buffer[8], v1);

    // Misaligned load to get the data we want.
    return _mm256_loadu_ps(&buffer[n]);
}

#ifndef DEBUG
inline
#endif
void aasort256_vector_cmpswap_skew(__m256 *in, int i, int j)
{
    __m256 x = _mm256_slli_ps(in[i], 4);
    __m256 y = _mm256_min_ps(in[j], x);
    in[j] = _mm256_max_ps(in[j], x);
    in[i] = _mm256_srli_ps(in[i], 12);
    in[i] = _mm256_alignr_epi8(in[i], y, 4);
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
     * (1) Sort values within each vector in ascending order using a sorting network.
     */

    for(uint32_t i = 0; i < nv; i += 8) {
        __m256 *x = &in[i];
        
        aasort256_vector_cmpswap(x, 0, 1);
        aasort256_vector_cmpswap(x, 2, 3);
        aasort256_vector_cmpswap(x, 4, 5);
        aasort256_vector_cmpswap(x, 6, 7);
        aasort256_vector_cmpswap(x, 0, 2);
        aasort256_vector_cmpswap(x, 1, 3);
        aasort256_vector_cmpswap(x, 4, 6);
        aasort256_vector_cmpswap(x, 5, 7);
        aasort256_vector_cmpswap(x, 1, 2);
        aasort256_vector_cmpswap(x, 5, 6);
        aasort256_vector_cmpswap(x, 0, 4);
        aasort256_vector_cmpswap(x, 1, 5);
        aasort256_vector_cmpswap(x, 2, 6);
        aasort256_vector_cmpswap(x, 3, 7);
        aasort256_vector_cmpswap(x, 2, 4);
        aasort256_vector_cmpswap(x, 3, 5);
        aasort256_vector_cmpswap(x, 1, 2);
        aasort256_vector_cmpswap(x, 3, 4);
        aasort256_vector_cmpswap(x, 5, 6);
        
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

void aasort256(uint32_t n, uint32_t *in, uint32_t *out)
{
    /*
     * (1) Divide all of the data to be sorted into blocks that
     * fit in the cache or the local memory of the processor.
     */

    // As stated in the paper, we use half the L2 cache as block size.
    if(!l2_cache)
        l2_cache = (uint32_t) sysconf(_SC_LEVEL2_CACHE_SIZE);
    uint32_t block_size = l2_cache / 2;
    uint32_t block_elements = block_size / 8;

    for(uint32_t i = 0; i < n; i += block_elements) {
        uint32_t k = min(n - i, block_elements);

        /*
         * (2) Sort each block with the in-core sorting algorithm.
         */

        if(!aasort256_in_core(k, (__m256*) &in[i], (__m256*) &out[i]))
            return;
    }
}

#endif /* AVX */
