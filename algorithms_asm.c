#include <stdint.h>

/* ***************************************************************************
 * Sorting algorithms.
 * ***************************************************************************/

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
            "\n\t movss combsort_shrink_factor, %%xmm2"
        
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
