//By Monica Moniot
#define PCG_IMPLEMENTATION
#include "pcg.h"



int main() {
	PCG rng = PCG_INITIALIZER;
	int p[10] = {0};
	for(int i = 0; i < 100000; i += 1) {
		p[pcg_random_in(&rng, 0, 9)] += 1;
	}

	for(int i = 0; i < 10; i += 1) {
		printf("%d, ", p[i]);
	}
	printf("\n");
	return 0;
}
