#include "algorithms.h"


void heapsort_asm(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    __asm volatile (
            // end = n - 1.
            "\n\t subl $1, %%ecx"

            // #######################
            // First step: Heapify.

            // start = (n - 2) / 2
            "\n\t movl %%ecx, %%esi"
            "\n\t subl $1, %%esi"
            "\n\t shr %%esi"

            // jmp address 
            "\n\t lea hasm_heapify_siftdown_return, %%r10"

        "\n hasm_heapify_loop:"

            // exit condition: start < 0
            "\n\t testl %%esi, %%esi"
            "\n\t js hasm_heapify_exit"

            // Call siftdown_helper.
            "\n\t mov %%rsi, %%r8"
            "\n\t jmp hasm_siftdown_loop"

        "\n hasm_heapify_siftdown_return:"

            "\n\t subl $1, %%esi"
            "\n\t jmp hasm_heapify_loop"

        "\n hasm_heapify_exit:"

            // End of heapify.
            // #######################

            // #######################
            // Main section.

            // jmp address
            "\n\t lea hasm_main_siftdown_return, %%r10"

        "\n hasm_main_loop:"

            // Swap first element and end.
            "\n\t movl (%%rdi), %%eax"
            "\n\t movl (%%rdi, %%rcx, 4), %%ebx"
            "\n\t movl %%eax, (%%rdi, %%rcx, 4)"
            "\n\t movl %%ebx, (%%rdi)"

            // Decrement end.
            "\n\t subl $1, %%ecx"

            // Call siftdown_helper.
            "\n\t xor %%r8, %%r8"
            "\n\t jmp hasm_siftdown_loop"

        "\n hasm_main_siftdown_return:"

            // Exit condition: end == 0
            "\n\t testl %%ecx, %%ecx"
            "\n\t je hasm_exit"

            "\n\t jmp hasm_main_loop"

            // End of main section.
            // #######################

            // #######################
            // siftdown_helper.
            // We expect start in r8, end in rcx, return address in r10.

        "\n hasm_siftdown_loop:"

            // child = root * 2 + 1
            "\n\t mov %%r8, %%r11"
            "\n\t shl %%r11"
            "\n\t add $1, %%r11"

            // exit condition: child > end
            "\n\t cmp %%rcx, %%r11"
            "\n\t jg hasm_siftdown_loop_exit"

            // s = root
            "\n\t mov %%r8, %%r12"

            // Load out[child] and out[s]
            "\n\t movl (%%rdi, %%r11, 4), %%eax"
            "\n\t movl (%%rdi, %%r12, 4), %%ebx"

            // if out[s] < out[child]
            "\n\t cmpl %%eax, %%ebx"
            "\n\t jge hasm_siftdown_left_child_skip"

            // s = child, out[s] = out[child] (only in register!)
            "\n\t mov %%r11, %%r12"
            "\n\t movl %%eax, %%ebx"

        "\n hasm_siftdown_left_child_skip:"

            // right child = child + 1
            "\n\t add $1, %%r11"

            // Does the right child even exist?
            "\n\t cmp %%rcx, %%r11"
            "\n\t jg hasm_siftdown_right_child_skip"

            // Load it.
            "\n\t movl (%%rdi, %%r11, 4), %%eax"

            // Is it larger than the current swap item?
            "\n\t cmpl %%eax, %%ebx"
            "\n\t jge hasm_siftdown_right_child_skip"

            // s = right child, out[s] = out[right child] (only in register!)
            "\n\t mov %%r11, %%r12"
            "\n\t movl %%eax, %%ebx"

        "\n hasm_siftdown_right_child_skip:"

            // Do we need to swap at all?
            "\n\t cmp %%r8, %%r12"
            "\n\t je hasm_siftdown_loop_exit" // Break.

            // Swap.
            "\n\t movl (%%rdi, %%r8, 4), %%eax"
            "\n\t movl %%eax, (%%rdi, %%r12, 4)"
            "\n\t movl %%ebx, (%%rdi, %%r8, 4)"
            "\n\t mov %%r12, %%r8"

            "\n\t jmp hasm_siftdown_loop"

        "\n hasm_siftdown_loop_exit:"

            "\n\t jmp *%%r10"

            // end of siftdown_helper
            // #######################

        "\n hasm_exit:"
        : // no output
        : "D"(out), "c"(n)
        : "flags", "memory", "rax", "rbx", "rdx", "rsi", "r8", "r10", "r11", "r12"
    );
}
