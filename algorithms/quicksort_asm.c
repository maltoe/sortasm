#include "algorithms.h"


void quicksort_iterative_asm(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    __asm volatile (
            // uint32_t stack_count = 0;
            "\n\t xor %%rdx, %%rdx"
                        
        "\n qiasm_loop:"
            
            // ecx contains cur_n, rdi contains cur_out.
        
            // if (cur_n <= 1) continue;
            "\n\t cmpl $1, %%ecx"
            "\n\t jle qiasm_load_remaining_from_stack"
            
            // Save cur_n, cur_out, stack count in unused registers.
            "\n\t mov %%rdx, %%r11"
            "\n\t mov %%rcx, %%r12"
            "\n\t mov %%rdi, %%r13"
            
            // ###########################
            // The following code is the partition helper.
        
            // Decrement n as we only need it in (n - 1) fashion.
            "\n\t subl $1, %%ecx"
            // uint32_t pivot = out[n - 1] --> eax.
            "\n\t movl (%%rdi, %%rcx, 4), %%eax"
            // uint32_t i = 0 --> edx
            "\n\t xor %%rdx, %%rdx"
            // uint32_t store_idx = 0 --> esi
            "\n\t xor %%rsi, %%rsi"
            
            // Note that we assume that we never get called with n == 1,
            // thus put the loop cond. at the end.
        "\n qphasm_loop:"
            // Load out[i].
            "\n\t movl (%%rdi, %%rdx, 4), %%ebx"
            
            // if (out[i] >= pivot) skip swap.
            "\n\t cmpl %%eax, %%ebx"
            "\n\t jge qphasm_no_swap"
            
            // Swap.
            "\n\t mov %%rbx, %%r14"
            "\n\t movl (%%rdi, %%rsi, 4), %%ebx"
            "\n\t movl %%ebx, (%%rdi, %%rdx, 4)"
            "\n\t mov %%r14, %%rbx"
            "\n\t movl %%ebx, (%%rdi, %%rsi, 4)"
            
            // store_idx++
            "\n\t addl $1, %%esi"
            
        "\n qphasm_no_swap:"
            
            // i++.
            "\n\t addl $1, %%edx"
            // if (i < n - 1) repeat;
            "\n\t cmpl %%ecx, %%edx"
            "\n\t jl qphasm_loop"
        
            // End of loop.
            
            // out[n - 1] = out[store_idx]; out[store_idx] = pivot;
            "\n\t movl (%%rdi, %%rsi, 4), %%ebx"
            "\n\t movl %%eax, (%%rdi, %%rsi, 4)"
            "\n\t movl %%ebx, (%%rdi, %%rcx, 4)"            
            
            // End of partition helper.
            // ###########################

            // Now esi contains store_idx.
            // Load old cur_n, cur_out and stack_count.
            "\n\t mov %%r13, %%rdi"
            "\n\t mov %%r12, %%rcx"
            "\n\t mov %%r11, %%rdx"
            
            // Save lower list part on stack.
            "\n\t push %%rsi"
            "\n\t push %%rdi"
            "\n\t addl $1, %%edx"
            
            // Continue with upper part.
            
            // cur_n -= (store_idx + 1)
            "\n\t addl $1, %%esi"
            "\n\t subl %%esi, %%ecx"
            
            // cur_out = &cur_out[store_idx + 1])
            "\n\t lea (%%rdi, %%rsi, 4), %%rdi"
            
            "\n\t jmp qiasm_loop"
                    
        "\n qiasm_load_remaining_from_stack:"
            
            // if (stack_count == 0) goto exit;
            "\n\t testl %%edx, %%edx"
            "\n\t je qiasm_exit"
        
            // Pop cur_n & cur_out from stack.
            "\n\t pop %%rdi"
            "\n\t pop %%rcx"
            "\n\t subl $1, %%edx"
            "\n\t jmp qiasm_loop"
            
        "\n qiasm_exit:"
        : // no output
        : "D"(out), "c"(n)
        : "flags", "memory", "rax", "rbx", "rdx", "rsi", "r11", "r12", "r13", "r14"
    );
}
