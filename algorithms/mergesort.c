#include "algorithms.h"

void mergesort(uint32_t n, uint32_t *in, uint32_t *out)
{
	if(n <= 1) {
		if(n == 1)
			*out = *in;
		return;
	}

	uint32_t nl = n / 2;
	uint32_t nu = n - nl;
	uint32_t *inl = out;
	uint32_t *inu = &out[nl];
	uint32_t *outl = in;
	uint32_t *outu = &in[nl];

	mergesort(nl, inl, outl);
	mergesort(nu, inu, outu);

	uint32_t pos = 0;
	uint32_t i = 0;
	uint32_t j = 0;
	while(pos < n) {
		if(j >= nu || (i < nl && outl[i] <= outu[j]))
			out[pos++] = outl[i++];
		else
			out[pos++] = outu[j++];
	}
}
