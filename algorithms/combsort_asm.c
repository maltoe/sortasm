#include "algorithms.h"


const float combsort_asm_shrink_factor = 1.24733095F;
void combsort_asm(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    __asm volatile (
            // We check the loop cond. at the bottom.
            // Thus, do an initial check here.
            "\n\t cmpl $1, %%ecx"
            "\n\t jle casm_exit"
        
            // uint32_t gap = n;
            "\n\t movl %%ecx, %%edx"
            // Load static shrink_factor into xmm2.
            "\n\t movss combsort_asm_shrink_factor, %%xmm2"
        
        "\n casm_outer_loop:"
        
            // if (gap <= 1) skip shrink.
            "\n\t cmpl $1, %%edx"
            "\n\t jle casm_no_gap_shrink"
            
            // Shrink gap.
            "\n\t cvtsi2ss %%edx, %%xmm1"
            "\n\t divss %%xmm2, %%xmm1"
            "\n\t cvttss2si %%xmm1, %%edx"
                       
        "\n casm_no_gap_shrink:"
        
            // swapped = 0;
            "\n\t xorps %%xmm0, %%xmm0"
            // uint32_t i = 0;
            "\n\t xor %%rsi, %%rsi"
            
        "\n casm_inner_loop:"
        
            // Temporarily set gap += i.
            "\n\t addl %%esi, %%edx"
        
            // if (gap + i >= n) break;
            "\n\t cmpl %%ecx, %%edx"
            "\n\t jge casm_inner_loop_exit"
            
            // Load out[i], out[i + gap]
            "\n\t movl (%%rdi, %%rsi, 4), %%eax"
            "\n\t movl (%%rdi, %%rdx, 4), %%ebx"
            
            // if(out[i] <= out[i + gap]) skip swap.
            "\n\t cmpl %%ebx, %%eax"
            "\n\t jle casm_no_swap"
            
            // Swap.
            "\n\t movl %%ebx, (%%rdi, %%rsi, 4)"
            "\n\t movl %%eax, (%%rdi, %%rdx, 4)"
            
            // Set swapped = true;
            "\n\t cmpeqps %%xmm0, %%xmm0"
            
        "\n casm_no_swap:"    
            
            // Reset edx = gap.
            "\n\t subl %%esi, %%edx"
            
            // i++; repeat;
            "\n\t addl $1, %%esi"
            "\n\t jmp casm_inner_loop"
            
        "\n casm_inner_loop_exit:"
        
            // Reset edx = gap a last time.
            "\n\t subl %%esi, %%edx"
            
            // First cond: if (gap > 1) repeat;
            "\n\t cmpl $1, %%edx"
            "\n\t jg casm_outer_loop"
            
            // Second cond: if (swapped) repeat;
            "\n\t ptest %%xmm0, %%xmm0"
            "\n\t jne casm_outer_loop"            
                    
        "\n casm_exit:"
        
        : // no output
        : "D"(out), "c"(n)
        : "flags", "memory", "rax", "rbx", "rdx", "rsi", "xmm0", "xmm1", "xmm2"
    );
}
