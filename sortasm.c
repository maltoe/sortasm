#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ***************************************************************************
 * Prototypes & Algorithm registry.
 * ***************************************************************************/

void insertionsort(int, int*, int*);
void insertionsort_asm(int, int*, int*);
void bubblesort(int, int*, int*);
void bubblesort_asm(int, int*, int*);
void gnomesort(int, int*, int*);
void gnomesort_rewrite(int, int*, int*);
void gnomesort_asm(int, int*, int*);
void combsort(int, int*, int*);
void combsort_asm(int, int*, int*);
void quicksort_recursive(int, int*, int*);
void quicksort_iterative(int, int*, int*);
void quicksort_iterative_asm(int, int*, int*);
void heapsort(int, int*, int*);

typedef void (*sort_func)(int, int*, int*);

const int num_sort_funcs = 13;
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
    heapsort
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
    "heapsort"
};

/* ***************************************************************************
 * Main. Benchmarks.
 * ***************************************************************************/

void tim(sort_func sf, const char *name, int n, int *in, int *out) {
    struct timespec t1, t2;
    clock_gettime(CLOCK_REALTIME, &t1);
    sf(n, in, out);
    clock_gettime(CLOCK_REALTIME, &t2);
    printf("%s : %f s.\n", name, 
        (t2.tv_sec - t1.tv_sec) + 
        (float) (t2.tv_nsec - t1.tv_nsec) / 1000000000);
}

void usage()
{
    printf("Usage:\n");
    printf("\t./test <alg> <num1> [num2, num3, ...]\n");
    printf("\nAlgorithms:\n");
    for(int i = 0; i < num_sort_funcs; i++)
        printf("\t[%d] %s\n", i, sort_func_names[i]);
    printf("No parameters for benchmark.\n");
}

void main(int argc, char **argv) 
{
    if(argc == 1) {
        // No args -> benchmark.
        
        const int n = 100000;
        int list[n];
        srand(time(NULL));
        for(int i = 0; i < n; i++)
            list[i] = rand();

        int in[n];
        int out[n];
        
        for(int alg = 0; alg < num_sort_funcs; alg++) {
            for(int i = 0; i < n; i++) {
                in[i] = list[i];
                out[i] = list[i];        
            }
            tim(sort_funcs[alg], sort_func_names[alg], n, in, out);
        }
        
        return;
    }
    
    if(argc < 3) {
        usage();
        return;
    }
    
    int alg;
    if((sscanf(argv[1], "%d", &alg) != 1) || (alg < 0) || (alg > num_sort_funcs - 1)) {
        usage();
        return;
    }
    
    int n = argc - 2;    
    int in[n], out[n];
    for(int i = 0; i < n; i++) {
        if(sscanf(argv[i + 2], "%d", &in[i]) != 1) {
            usage();
            return;
        }
        out[i] = in[i];
    }
    
    sort_funcs[alg](n, in, out);
    
    for(int i = 0; i < n; i++)
        printf("%d ", out[i]);
    printf("\n");
}

/* ***************************************************************************
 * General helper functions.
 * ***************************************************************************/

#ifndef DEBUG
inline
#endif
void swap(int *arr, int i, int j)
{
    int tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
}

/* ***************************************************************************
 * Sorting algorithms.
 * ***************************************************************************/

void insertionsort(int n, int *in, int *out)
{
    outerLoop:
    for(int i = 0; i < n; i++) {
        int inserted = 0;
        for(int j = 0; j < i; j++) {
            if(in[i] < out[j]) {
                // Move & insert.
                for(int k = i - 1; k >= j; k--)
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

void insertionsort_asm(int n, int *in, int *out)
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

void bubblesort(int n, int *in, int *out)
{
    for(int i = n; i > 1; i--) {
        for(int j = 0; j < i - 1; j++) {
            if(out[j] > out[j + 1])
                swap(out, j, j + 1);
        }
    }
}

void bubblesort_asm(int n, int *in, int *out)
{
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

void gnomesort(int n, int *in, int *out)
{
    int i = 1;
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

void gnomesort_rewrite(int n, int *in, int *out)
{
    int i = 0;
    while(i < n) {
        i++;
        if(out[i] < out[i - 1]) {
            swap(out, i, i - 1);

            if(i > 1)
                i -= 2;
        }
    }
}

void gnomesort_asm(int n, int *in, int *out)
{
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
void combsort(int n, int *in, int *out)
{
    int gap = n;
    int swapped = 0;

    while((gap > 1) || swapped) {
        if(gap > 1)
            gap = (int) ((double) gap / combsort_shrink_factor);
         
        swapped = 0;
        for (int i = 0; gap + i < n; i++) {
            if (out[i] > out[i + gap]) {
                swap(out, i, i + gap);
                swapped = 1;
            }
        }
    }
}

void combsort_asm(int n, int *in, int *out)
{
    __asm volatile (
            // We check the loop cond. at the bottom.
            // Thus, do an initial check here.
            "\n\t cmpl $1, %%ecx"
            "\n\t jle casm_exit"
        
            // int gap = n;
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
            // int i = 0;
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
int quicksort_partition_helper(int n, int *out)
{
    int pivot = out[n - 1];
    int store_idx = 0;
    for(int i = 0; i <  n - 1; i++) {
        if(out[i] < pivot) {
            swap(out, i, store_idx);
            store_idx++;
        }
    }
    out[n - 1] = out[store_idx];
    out[store_idx] = pivot;
    
    return store_idx;
}

void quicksort_recursive(int n, int *in, int *out)
{
    if(n <= 1)
        return;
    
    int store_idx = quicksort_partition_helper(n, out);
    
    quicksort_recursive(store_idx, NULL, out);
    quicksort_recursive(n - store_idx - 1, NULL, &out[store_idx + 1]);
}

void quicksort_iterative(int n, int *in, int *out)
{
    int stack_count;
    int stack_n[n];
    int *stack_out[n];
    
    stack_count = 1;
    stack_n[0] = n;
    stack_out[0] = out;
    
    while(stack_count > 0) {
        int cur_n = stack_n[stack_count - 1];
        int *cur_out = stack_out[stack_count - 1];
        stack_count--;
        
        if(cur_n <= 1)
            continue;
        
        int store_idx = quicksort_partition_helper(cur_n, cur_out);
        
        stack_n[stack_count] = store_idx;
        stack_out[stack_count] = cur_out;
        stack_count++;
        stack_n[stack_count] = cur_n - store_idx - 1;
        stack_out[stack_count] = &cur_out[store_idx + 1];
        stack_count++;
    }
}

void quicksort_iterative_asm(int n, int *in, int *out)
{
    __asm volatile (
            // int stack_count = 0;
            "\n\t xor %%rdx, %%rdx"
                        
        "\n qiasm_loop:"
            
            // ecx contains cur_n, rdi contains cur_out.
        
            // if (cur_n <= 1) continue;
            "\n\t cmpl $1, %%ecx"
            "\n\t jle qiasm_load_remaining_from_stack"
            
            // Save cur_n, cur_out, stack count.
            "\n\t push %%rdx"
            "\n\t push %%rcx"
            "\n\t push %%rdi"
            
            // ###########################
            // The following code is the partition helper.
        
            // Decrement n as we only need it in (n - 1) fashion.
            "\n\t subl $1, %%ecx"
            // int pivot = out[n - 1] --> eax.
            "\n\t movl (%%rdi, %%rcx, 4), %%eax"
            // int i = 0 --> edx
            "\n\t xor %%rdx, %%rdx"
            // int store_idx = 0 --> esi
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
            "\n\t push %%rbx"
            "\n\t movl (%%rdi, %%rsi, 4), %%ebx"
            "\n\t movl %%ebx, (%%rdi, %%rdx, 4)"
            "\n\t pop %%rbx"
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
            // Load old cur_n, cur_out and stack_count from stack.
            "\n\t pop %%rdi"
            "\n\t pop %%rcx"
            "\n\t pop %%rdx"
            
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
        : "flags", "memory", "%rax", "%rbx", "%rdx", "rsi"
    );
}

#ifndef DEBUG
inline
#endif
void heapsort_siftdown_helper(int start, int end, int *out)
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
void heapsort_heapify_helper(int n, int *out)
{
    // Start is assigned the index in a of the last parent node.   
    for(int start = (n - 2) / 2; start >= 0; start--) {
        
        // Sift down the node at index start to the proper place such that all nodes below
        // the start index are in heap order.
        heapsort_siftdown_helper(start, n - 1, out);

    }
}

void heapsort(int n, int *in, int *out)
{
    // First place out in max-heap order.
    heapsort_heapify_helper(n, out);
    
    int end = n - 1;
    while(end > 0) {
        // Swap the root(maximum value) of the heap with the last element of the heap.
        swap(out, 0, end);
        end--;
        heapsort_siftdown_helper(0, end, out);
    }
}
