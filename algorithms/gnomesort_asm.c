#include "algorithms.h"

void gnomesort_asm(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    __asm volatile (
            // i = 0
            "\n\t xor %%rdx, %%rdx"

        "\ngasm_loop:"
            // i++
            "\n\t addl $1, %%edx"

            // if (i >= n) break;
            "\n\t cmpl %%ecx, %%edx"
            "\n\t jge gasm_exit"

            // Load out[i] & out[i - 1]
            "\n\t movl -4(%%rdi, %%rdx, 4), %%eax"
            "\n\t movl (%%rdi, %%rdx, 4), %%ebx"

            // if(out[i] >= out[i - 1]) skip swap.
            "\n\t cmpl %%eax, %%ebx"
            "\n\t jge gasm_loop"

            // Swap.
            "\n\t movl %%ebx, -4(%%rdi, %%rdx, 4)"
            "\n\t movl %%eax, (%%rdi, %%rdx, 4)"

            // if (i <= 1) skip decrement.
            "\n\t cmpl $1, %%edx"
            "\n\t jle gasm_loop"

            // i -= 2 (since we're going to increment it anyway at the
            // beginning of the loop)
            "\n\t subl $2, %%edx"

            // repeat
            "\n\t jmp gasm_loop"

        "\n gasm_exit:"

        : // no output
        : "D"(out), "c"(n)
        : "flags", "memory", "%rax", "%rbx", "%rdx"
    );
}
