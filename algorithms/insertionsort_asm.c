#include "algorithms.h"


void insertionsort_asm(uint32_t n, uint32_t *in, uint32_t *out)
{
    __asm volatile (
            // i = 0
            "\n\t xor %%rdx, %%rdx"

            // if(n == 0) goto exit; (assuming only positive n)
            "\n\t testl %%ecx, %%ecx"
            "\n\t je iasm_exit"
            
        // while(i < n)
        "\niasm_outer_loop:"
            // Store n on the stack.
            "\n\t push %%rcx"

            // Load in[i] into eax.
            "\n\t movl (%%rsi, %%rdx, 4), %%eax"

            // j = 0
            "\n\t xorl %%ecx, %%ecx"
        "\niasm_inner_loop:"
            // if (j >= i) break;
            "\n\t cmpl %%edx, %%ecx"
            "\n\t jge iasm_inner_loop_exit"

            // Load out[j] into ebx.
            "\n\t movl (%%rdi, %%rcx, 4), %%ebx"

            // if(in[i] >= out[j]) skip insertion;
            "\n\t cmpl %%ebx, %%eax"
            "\n\t jge iasm_skip_insertion"

            // MOVE & INSERT.

                // Store i on stack.
                "\n\t push %%rdx"

                // k = i - 1
                "\n\t subl $1, %%edx"
        "\niasm_move_loop:"
                // if (k < j) break;
                "\n\t cmpl %%ecx, %%edx"
                "\n\t jl iasm_move_loop_exit"

                // out[k + 1] = out[k]
                "\n\t movl (%%rdi, %%rdx, 4), %%ebx"
                "\n\t movl %%ebx, 4(%%rdi, %%rdx, 4)"

                // k--; repeat;
                "\n\t subl $1, %%edx"
                "\n\t jmp iasm_move_loop"
        "\niasm_move_loop_exit:"

                // Pop i from stack.
                "\n\t pop %%rdx"

                // out[j] = in[i]
                "\n\t movl %%eax, (%%rdi, %%rcx, 4)"

                // break;
                "\n\t jmp iasm_inner_loop_break"

        "\niasm_skip_insertion:"

            // j++; repeat;
            "\n\t addl $1, %%ecx"
            "\n\t jmp iasm_inner_loop"

        "\niasm_inner_loop_exit:"

            // End of loop reached without insertion, insert at the end!
            // out[i] = in[i]
            "\n\t movl %%eax, (%%rdi, %%rdx, 4)"

        "\niasm_inner_loop_break:"

            // i++;
            "\n\t addl $1, %%edx"
            // Retrieve n from the stack.
            "\n\t pop %%rcx"
            // if (i < n) repeat;
            "\n\t cmpl %%ecx, %%edx"
            "\n\t jl iasm_outer_loop"

        "\niasm_exit:"
        : // no output
        : "S"(in), "D"(out), "c"(n)
        : "flags", "memory", "%rdx", "%rax", "%rbx"
    );
}
