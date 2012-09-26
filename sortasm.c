#include <getopt.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ***************************************************************************
 * Prototypes.
 * ***************************************************************************/

void insertionsort(uint32_t, uint32_t*, uint32_t*);
void bubblesort(uint32_t, uint32_t*, uint32_t*);
void gnomesort(uint32_t, uint32_t*, uint32_t*);
void gnomesort_rewrite(uint32_t, uint32_t*, uint32_t*);
void combsort(uint32_t, uint32_t*, uint32_t*);
void quicksort_recursive(uint32_t, uint32_t*, uint32_t*);
void quicksort_iterative(uint32_t, uint32_t*, uint32_t*);
void heapsort(uint32_t, uint32_t*, uint32_t*);
void aasort(uint32_t, uint32_t*, uint32_t*);

void insertionsort_asm(uint32_t, uint32_t*, uint32_t*);
void bubblesort_asm(uint32_t, uint32_t*, uint32_t*);
void gnomesort_asm(uint32_t, uint32_t*, uint32_t*);
void combsort_asm(uint32_t, uint32_t*, uint32_t*);
void quicksort_iterative_asm(uint32_t, uint32_t*, uint32_t*);
void heapsort_asm(uint32_t, uint32_t*, uint32_t*);

/* ***************************************************************************
 * Algorithm registry.
 * ***************************************************************************/

typedef void (*sort_func)(uint32_t, uint32_t*, uint32_t*);

typedef struct {
    const char *name;
    sort_func func;
    uint32_t max_n; // Do not run benchmarks for n larger than this value.
} algorithm_st;

algorithm_st algorithms[] = {
    {
        "insertionsort",
        insertionsort,
        200000
    },
    {
        "insertionsort_asm",
        insertionsort_asm,
        200000
    },
    {
        "bubblesort",
        bubblesort,
        100000
    },
    {
        "bubblesort_asm",
        bubblesort_asm,
        100000
    },
    {
        "gnomesort",
        gnomesort,
        100000
    },
    {
        "gnomesort_rewrite",
        gnomesort_rewrite,
        100000
    },
    {
        "gnomesort_asm",
        gnomesort_asm,
        100000
    },
    {
        "combsort",
        combsort,
        UINT_MAX
    },
    {
        "combsort_asm",
        combsort_asm,
        UINT_MAX
    },
    {
        "quicksort_recursive",
        quicksort_recursive,
        UINT_MAX
    },
    {
        "quicksort_iterative",
        quicksort_iterative,
        UINT_MAX
    },
    {
        "quicksort_iterative_asm",
        quicksort_iterative_asm,
        UINT_MAX
    },
    {
        "heapsort",
        heapsort,
        UINT_MAX
    },
    {
        "heapsort_asm",
        heapsort_asm,
        UINT_MAX
    },
    {
        "aasort",
        aasort,
        UINT_MAX
    }
};

const uint32_t num_algorithms = sizeof(algorithms) / sizeof(algorithm_st);

/* ***************************************************************************
 * Main. Benchmarking.
 * ***************************************************************************/

void usage(const char *appname)
{
    printf("Usage:\n");
    printf("\t%s [-a <alg>] [-n <number of elements>] [number, number, number...]\n", appname);
    printf("\nAlgorithms:\n");
    for(int i = 0; i < num_algorithms; i++)
        printf("\t[%d] %s\n", i, algorithms[i].name);
    printf("\nRemember: Number of elements must be dividable by 16 for aasort to work.\n");
}

void benchmark(algorithm_st alg, uint32_t n, uint32_t *data, int print_results) 
{
    if(n > alg.max_n) {
        printf("Skipping %s, as it would probably take forever...\n", alg.name);
        return;
    }

    // Prepare data.
    uint32_t *in = valloc(4 * n);
    uint32_t *out = valloc(4 * n);
    for(uint32_t j = 0; j < n; j++) {
        in[j] = data[j];
        out[j] = data[j];        
    }

    // Benchmark runtime.
    struct timespec t1, t2;
    clock_gettime(CLOCK_REALTIME, &t1);

    alg.func(n, in, out);

    clock_gettime(CLOCK_REALTIME, &t2);

    printf("%s : %f s.\n", alg.name, 
        (t2.tv_sec - t1.tv_sec) + (float) (t2.tv_nsec - t1.tv_nsec) / 1000000000);

    // Verify results.
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

    if(print_results) {
        for(uint32_t i = 0; i < n; i++)
            printf("%u ", out[i]);       
        printf("\n");
    }

    // Cleanup.
    free(in);
    free(out); 
}

int main(int argc, char **argv) 
{
    // NOTE: n needs to be dividable by 16 for aasort.
    uint32_t n = 64000;
    int alg = -1;

    int c;
    while((c = getopt(argc, argv, "a:n:")) != -1) {
        switch(c) {
        case 'a':
           if((sscanf(optarg, "%i", &alg) != 1) 
                || (alg < 0) || (alg > num_algorithms - 1)) {
                usage(argv[0]);
                return 1;
            } 
            break;
        case 'n':
            if(sscanf(optarg, "%u", &n) != 1) {
                usage(argv[0]);
                return 1;            
            }
            break;
        default:
            usage(argv[0]);
            return 1;
        }
    }

    uint32_t *data;
    int print_results = 0;

    if(optind < argc) {
        // Number list given.
        n = argc - optind;
        print_results = 1;

        data = malloc(4 * n);
        for(int i = optind; i < argc; i++) {
            if(sscanf(argv[i], "%u", &data[i - optind]) != 1) {
                usage(argv[0]);
                return 1;
            }           
        }
    } else {
        // Randomize input.
        data = malloc(4 * n);
        srand(time(NULL));
        for(uint32_t i = 0; i < n; i++)
            data[i] = rand();
    }

    if(alg == -1) {
        for(uint32_t i = 0; i < num_algorithms; i++) {
            benchmark(algorithms[i], n, data, print_results);
        }
    } else {
        benchmark(algorithms[alg], n, data, print_results);
    }

    free(data);
    return 0;
}
