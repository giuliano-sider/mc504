#include <stdio.h>

int main() {
	/* find primes just larger than powers of 2 */
	unsigned int i, powers = 2;
	int primes[33];
	int toosmall[33];
	int toobig[33];

	for (i = 1; i < 32; i++, powers *= 2) {
		unsigned int j = (powers + 2*powers)/2; /*keep it far from powers of 2 */
		if (j % 2 == 0)
			j++;

		for ( ; j < 2*powers; j += 2) {
			unsigned int k;
			char isprime = 1;
			for (k = 3; k*k <= j; k += 2) {
				if (j % k == 0) {
					isprime = 0;
					break;
				}
			}
			if (isprime) {
				//printf("%i,\n", j);
				break; // find a prime bigger than the next power now
			}
		}
		if (j > 2*powers)
			j = -1; /* prime not found */
		primes[i] = j;
		toosmall[i] = (unsigned int) j / 4.0;
		toobig[i] = (unsigned int) j * 3.0/4.0;
	}
	for (i = 1; i < 32; i++) {
		printf("%-20i, %-20i, %-20i,\n", primes[i], toosmall[i], toobig[i]);
	}
	return 0;
}