#include "algorithms.h"

void bubblesort_asm(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    __asm volatile (
            // Givens: ecx holds n, rdi holds out.
            // Initialization.

            // if (i <= 1) goto exit;
            "\n\t cmpl $1, %%ecx"
            "\n\t jle basm_exit"

            // i--;
            "\n\t subl $1,%%ecx"

        // while(i > 1) (always executed once!)
        "\n basm_outer_loop:"
            
            // j = 0
            "\n\t xorl %%edx, %%edx"     // j = 0 

        // while(j < i - 1) (always executed once!)
        "\n basm_inner_loop:"
            "\n\t movl (%%rdi, %%rdx, 4), %%eax"       // out[j]
            "\n\t movl 4(%%rdi, %%rdx, 4), %%ebx"      // out[j + 1]
            "\n\t cmpl %%ebx, %%eax"
            "\n\t jle basm_no_swap"                    // out[j] <= out[j + 1] ?
            "\n\t movl %%ebx, (%%rdi, %%rdx, 4)"       // out[j] = out[j + 1]
            "\n\t movl %%eax, 4(%%rdi, %%rdx, 4)"      // out[j +1 ] = out[j]
        "\n basm_no_swap:"
            
            // j++
            "\n\t addl $1, %%edx"

            // if(i - 1 > j) repeat;
            "\n\t cmpl %%edx, %%ecx" 
            "\n\t jg basm_inner_loop"

        "\n basm_inner_loop_exit:"

            // i--;
            // if(i != 1) repeat;
            "\n\t subl $1, %%ecx"
            "\n\t jne basm_outer_loop"

        "\n basm_exit:"
        :
        : "D"(out), "c"(n) 
        : "flags", "memory", "%rax", "%rbx" , "%rdx"
    );
}

