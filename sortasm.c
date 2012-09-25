#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Includes for aasort.
#include <string.h>
#include <unistd.h>
#include <smmintrin.h>

/* ***************************************************************************
 * Prototypes & Algorithm registry.
 * ***************************************************************************/
void insertionsort(uint32_t, uint32_t*, uint32_t*);
void insertionsort_asm(uint32_t, uint32_t*, uint32_t*);
void bubblesort(uint32_t, uint32_t*, uint32_t*);
void bubblesort_asm(uint32_t, uint32_t*, uint32_t*);
void gnomesort(uint32_t, uint32_t*, uint32_t*);
void gnomesort_rewrite(uint32_t, uint32_t*, uint32_t*);
void gnomesort_asm(uint32_t, uint32_t*, uint32_t*);
void combsort(uint32_t, uint32_t*, uint32_t*);
void combsort_asm(uint32_t, uint32_t*, uint32_t*);
void quicksort_recursive(uint32_t, uint32_t*, uint32_t*);
void quicksort_iterative(uint32_t, uint32_t*, uint32_t*);
void quicksort_iterative_asm(uint32_t, uint32_t*, uint32_t*);
void heapsort(uint32_t, uint32_t*, uint32_t*);
void heapsort_asm(uint32_t, uint32_t*, uint32_t*);
void aasort(uint32_t, uint32_t*, uint32_t*);

typedef void (*sort_func)(uint32_t, uint32_t*, uint32_t*);

const uint32_t num_sort_funcs = 15;
const sort_func sort_funcs[]= {
    insertionsort,
    insertionsort_asm,
    bubblesort,
    bubblesort_asm,
    gnomesort,
    gnomesort_rewrite,
    gnomesort_asm,
    combsort,
    combsort_asm,
    quicksort_recursive,
    quicksort_iterative,
    quicksort_iterative_asm,
    heapsort,
    heapsort_asm,
    aasort
};
const char* sort_func_names[] = {
    "insertionsort",
    "insertionsort_asm",
    "bubblesort",
    "bubblesort_asm",
    "gnomesort",
    "gnomesort_rewrite",
    "gnomesort_asm",
    "combsort",
    "combsort_asm",
    "quicksort_recursive",
    "quicksort_iterative",
    "quicksort_iterative_asm",
    "heapsort",
    "heapsort_asm",
    "aasort"
};

/* ***************************************************************************
 * Main. Benchmarking.
 * ***************************************************************************/

void usage()
{
    printf("Usage:\n");
    printf("\t./sortasm <alg> [num1, num2, num3, ...]\n");
    printf("\nAlgorithms:\n");
    for(int i = 0; i < num_sort_funcs; i++)
        printf("\t[%d] %s\n", i, sort_func_names[i]);
    printf("No numbers for benchmark.\n");
    printf("No parameters for benchmarking all algorithms.\n");
}

void print_arr(uint32_t n, uint32_t *arr)
{
    for(uint32_t i = 0; i < n; i++)
        printf("%u ", arr[i]);       
    printf("\n");
}

void tim(sort_func sf, const char *name, uint32_t n, uint32_t *in, uint32_t *out) {
    struct timespec t1, t2;
    clock_gettime(CLOCK_REALTIME, &t1);
    sf(n, in, out);
    clock_gettime(CLOCK_REALTIME, &t2);
    printf("%s : %f s.\n", name, 
        (t2.tv_sec - t1.tv_sec) + 
        (float) (t2.tv_nsec - t1.tv_nsec) / 1000000000);
}

void verify_results(uint32_t n, uint32_t *out)
{
    for(uint32_t i = 0; i < n - 1; i++) {
        if(out[i] > out[i + 1]) {
            printf("Resulting array is not sorted (position %u/%u): ", i, n);

            printf("... ");
            uint32_t from = i - 2 >= 0 ? i - 2 : 0;
            uint32_t to = i + 4 <= n ? i + 4 : n;
            for(int j = from; j < to; j++)
                printf("%u ", out[j]);
            printf("...\n");

            break;
        }
    }    
}

void benchmark(uint32_t algn, uint32_t *algs)
{
    // NOTE: n needs to be dividable by 16 for aasort.
    const uint32_t n = 6400000 - 108544;

    uint32_t *list = valloc(4 * n);
    srand(time(NULL));
    for(uint32_t i = 0; i < n; i++)
        list[i] = rand();

    uint32_t *in = valloc(4 * n);
    uint32_t *out = valloc(4 * n);
    for(uint32_t i = 0; i < algn; i++) {
        uint32_t alg = algs[i];
        
        for(uint32_t j = 0; j < n; j++) {
            in[j] = list[j];
            out[j] = list[j];        
        }
        
        tim(sort_funcs[alg], sort_func_names[alg], n, in, out);
        verify_results(n, out);
    }

    free(list);
    free(in);
    free(out);
}

int main(int argc, char **argv) 
{
    if(argc == 1) {
        // Benchmark all algorithms.
        uint32_t algs[num_sort_funcs];
        for(uint32_t i = 0; i < num_sort_funcs; i++)
            algs[i] = i;
        benchmark(num_sort_funcs, algs);
        return 0;
    }
    
    uint32_t alg;
    if((sscanf(argv[1], "%i", &alg) != 1) || (alg < 0) || (alg > num_sort_funcs - 1)) {
        usage();
        return 1;
    }
    
    if(argc == 2) {
        // Benchmark single algorithm.
        uint32_t algs[] = { alg };
        benchmark(1, algs);
        return 0;
    }
    
    // Otherwise: Run single algorithm with given data.
    
    uint32_t n = argc - 2;    
    uint32_t in[n], out[n];
    for(uint32_t i = 0; i < n; i++) {
        if(sscanf(argv[i + 2], "%u", &in[i]) != 1) {
            usage();
            return 1;
        }
        out[i] = in[i];
    }
    
    sort_funcs[alg](n, in, out);
    verify_results(n, out);
    print_arr(n, out);

    return 0;
}

/* ***************************************************************************
 * General helper functions.
 * ***************************************************************************/

#ifndef DEBUG
inline
#endif
void swap(uint32_t *arr, uint32_t i, uint32_t j)
{
    uint32_t tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
}

#ifndef DEBUG
inline
#endif
uint32_t min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

/* ***************************************************************************
 * Sorting algorithms.
 * ***************************************************************************/

void insertionsort(uint32_t n, uint32_t *in, uint32_t *out)
{
    for(int i = 0; i < n; i++) {
        int inserted = 0;
        for(int j = 0; j < i; j++) {
            if(in[i] < out[j]) {
                // Move & insert.
                for(int k = (int) i - 1; k >= j; k--)
                    out[k + 1] = out[k];
                out[j] = in[i];
                inserted = 1;
                break;
            }
        }
        if(!inserted)
            out[i] = in[i];
    }
}

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

void bubblesort(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    for(uint32_t i = n; i > 1; i--) {
        for(uint32_t j = 0; j < i - 1; j++) {
            if(out[j] > out[j + 1])
                swap(out, j, j + 1);
        }
    }
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

void gnomesort(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    uint32_t i = 1;
    while(i < n) {
        if(out[i] >= out[i - 1])
            i++;
        else {
            swap(out, i, i - 1);
            
            if(i > 1)
                i--;
            else
                i++;
        }
    }
}

void gnomesort_rewrite(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    uint32_t i = 0;
    while(i < n) {
        i++;
        if(out[i] < out[i - 1]) {
            swap(out, i, i - 1);

            if(i > 1)
                i -= 2;
        }
    }
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

const float combsort_shrink_factor = 1.24733095F;
void combsort(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    uint32_t gap = n;
    uint32_t swapped = 0;

    while((gap > 1) || swapped) {
        if(gap > 1)
            gap = (uint32_t) ((double) gap / combsort_shrink_factor);
         
        swapped = 0;
        for (uint32_t i = 0; gap + i < n; i++) {
            if (out[i] > out[i + gap]) {
                swap(out, i, i + gap);
                swapped = 1;
            }
        }
    }
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

#ifndef DEBUG
inline 
#endif
uint32_t quicksort_partition_helper(uint32_t n, uint32_t *out)
{
    uint32_t pivot = out[n - 1];
    uint32_t store_idx = 0;
    for(uint32_t i = 0; i <  n - 1; i++) {
        if(out[i] < pivot) {
            swap(out, i, store_idx);
            store_idx++;
        }
    }
    out[n - 1] = out[store_idx];
    out[store_idx] = pivot;
    
    return store_idx;
}

void quicksort_recursive(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    if(n <= 1)
        return;
    
    uint32_t store_idx = quicksort_partition_helper(n, out);
    
    quicksort_recursive(store_idx, NULL, out);
    quicksort_recursive(n - store_idx - 1, NULL, &out[store_idx + 1]);
}

void quicksort_iterative(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    const uint32_t max_stack_size = ((int) log(n)/log(2)) + 1;
    uint32_t stack_count;
    uint32_t *stack_n = malloc(4 * max_stack_size);
    uint32_t **stack_out = malloc(sizeof(uint32_t*) * max_stack_size);
    
    stack_count = 1;
    stack_n[0] = n;
    stack_out[0] = out;
    
    while(stack_count > 0) {
        uint32_t cur_n = stack_n[stack_count - 1];
        uint32_t *cur_out = stack_out[stack_count - 1];
        stack_count--;
        
        if(cur_n <= 1)
            continue;
        
        uint32_t store_idx = quicksort_partition_helper(cur_n, cur_out);
        
        // Continue with the smaller part (i.e., push it onto stack last).
        // This way we can ensure that the stack size will not exceed log2(n).
        // See: http://stackoverflow.com/questions/6709055/quicksort-stack-size
        if(store_idx <= cur_n / 2) {
            stack_n[stack_count] = cur_n - store_idx - 1;
            stack_out[stack_count] = &cur_out[store_idx + 1];
            stack_count++;
            stack_n[stack_count] = store_idx;
            stack_out[stack_count] = cur_out;
            stack_count++;
        } else {
            stack_n[stack_count] = store_idx;
            stack_out[stack_count] = cur_out;
            stack_count++;
            stack_n[stack_count] = cur_n - store_idx - 1;
            stack_out[stack_count] = &cur_out[store_idx + 1];
            stack_count++;
        }
    }

    free(stack_n);
    free(stack_out);
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
        : "flags", "memory", "rax", "rbx", "rdx", "rsi"
    );
}

#ifndef DEBUG
inline
#endif
void heapsort_siftdown_helper(int start, int end, uint32_t *out)
{
    int root = start;
    
    // While the root has at least one child.
    while(root * 2 + 1 <= end) {
        int child = root * 2 + 1;
        int s = root;
        
        // Check if root is smaller than child.
        if(out[s] < out[child])
            s = child;
        
        // Check if right child exists, and if it's bigger than what we're currently swapping with.
        if(child + 1 <= end && out[s] < out[child + 1])
            s = child + 1;
        
        // Check if we need to swap at all.
        if(s != root) {
            swap(out, root, s);
            root = s;
        } else
            // No swap? Done sifting down.
            break;
    }
}

#ifndef DEBUG
inline
#endif
void heapsort_heapify_helper(uint32_t n, uint32_t *out)
{
    // Start is assigned the index in a of the last parent node.   
    for(int start = (int) (n - 2) / 2; start >= 0; start--) {
        
        // Sift down the node at index start to the proper place such that all nodes below
        // the start index are in heap order.
        heapsort_siftdown_helper(start, (int) n - 1, out);

    }
}

void heapsort(uint32_t n, uint32_t *in, uint32_t *out)
{
    (void) in;

    // First place out in max-heap order.
    heapsort_heapify_helper(n, out);
    
    int end = (int) n - 1;
    while(end > 0) {
        // Swap the root(maximum value) of the heap with the last element of the heap.
        swap(out, 0, end);
        end--;
        heapsort_siftdown_helper(0, end, out);
    }
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

typedef union {
    __m128i v;
    uint32_t i[4];
} m128i_u;

void aasort_vector_merge(m128i_u *a, m128i_u *b)
{
    uint32_t o[8];
    uint32_t ap = 0, bp = 0, op = 0;
    while(ap < 4 && bp < 4) {
        if((*a).i[ap] < (*b).i[bp])
            o[op++] = (*a).i[ap++];
        else 
            o[op++] = (*b).i[bp++];
    }

    while(ap < 4)
        o[op++] = (*a).i[ap++];

    while(bp < 4)
        o[op++] = (*b).i[bp++];

    (*a).v = _mm_load_si128((__m128i*) &o[0]);
    (*b).v = _mm_load_si128((__m128i*) &o[4]);
}

void aasort_out_of_core(uint32_t an, __m128i *a, uint32_t bn, __m128i *b, __m128i *out)
{
    uint32_t ap = 0, bp = 0, op = 0;
    
    __m128i vmin = a[ap++];
    __m128i vmax = b[bp++];
    while(ap < (an / 4) && bp < (bn / 4)) { 
        aasort_vector_merge((m128i_u*) &vmin, (m128i_u*) &vmax); 
        out[op++] = vmin;
        if(_mm_cvtsi128_si32(a[ap]) <= _mm_cvtsi128_si32(b[bp]))
            vmin = a[ap++];
        else
            vmin = b[bp++];
    }

    if(ap < (an / 4)) {
        aasort_vector_merge((m128i_u*) &vmin, (m128i_u*) &vmax);
        out[op++] = vmin;
        while(ap < (an / 4)) {
            vmin = a[ap++];
            aasort_vector_merge((m128i_u*) &vmin, (m128i_u*) &vmax);
            out[op++] = vmin;
        }
    } else if(bp < (bn / 4)) {
        aasort_vector_merge((m128i_u*) &vmin, (m128i_u*) &vmax);
        out[op++] = vmin;
        while(bp < (bn / 4)) {
            vmin = b[bp++];
            aasort_vector_merge((m128i_u*) &vmin, (m128i_u*) &vmax);
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
    uint32_t l2_cache = (uint32_t) sysconf(_SC_LEVEL2_CACHE_SIZE);
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
                // Last block? Copy.
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
