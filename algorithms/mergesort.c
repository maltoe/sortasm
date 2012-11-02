#include <pthread.h>
#include "algorithms.h"

#ifndef DEBUG
inline
#endif
void merge(uint32_t n, uint32_t nl, uint32_t *inl, uint32_t nu, uint32_t *inu, uint32_t *out)
{
	uint32_t pos = 0;
	uint32_t i = 0;
	uint32_t j = 0;
	while(pos < n) {
		if(j >= nu || (i < nl && inl[i] <= inu[j]))
			out[pos++] = inl[i++];
		else
			out[pos++] = inu[j++];
	}	
}

void mergesort(uint32_t n, uint32_t *in, uint32_t *out)
{
	if(n == 1) {
		*out = *in;
		return;
	}

	uint32_t nl = n / 2;
	uint32_t nu = n - nl;

	mergesort(nl, out, in);
	mergesort(nu, &out[nl], &in[nl]);

	merge(n, nl, in, nu, &in[nl], out);
}

const uint32_t thread_depth = 3;
typedef struct {
		uint32_t td;
    uint32_t n;
    uint32_t *in;
    uint32_t *out;
} algorithm_params_st;

void* mergesort_parallel_run(void *data)
{
	algorithm_params_st *params = (algorithm_params_st*) data;

	if(params->n <= 1) {
		*(params->out) = *(params->in);
	} else if(params->td == 0) {
		mergesort(params->n, params->in, params->out);
	}	else {
		uint32_t nl = params->n / 2;
		uint32_t nu = params->n - nl;

		pthread_t t1, t2;
		algorithm_params_st p1, p2;

		p1.td = params->td - 1;
		p1.n = nl;
		p1.in = params->out;
		p1.out = params->in;

		pthread_create(&t1, NULL, mergesort_parallel_run, &p1);

		p2.td = params->td - 1;
		p2.n = nu;
		p2.in = &(params->out[nl]);
		p2.out = &(params->in[nl]);

		pthread_create(&t2, NULL, mergesort_parallel_run, &p2);

		pthread_join(t1, NULL);
		pthread_join(t2, NULL);

		merge(params->n, nl, params->in, nu, &(params->in[nl]), params->out);
	}

	return NULL;
}


void mergesort_parallel(uint32_t n, uint32_t *in, uint32_t *out)
{
	algorithm_params_st s;
	s.td = thread_depth;
	s.n = n;
	s.in = in;
	s.out = out;
	mergesort_parallel_run(&s);
}