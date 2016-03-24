#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "matrix.h"

int main(int argc, char **argv) {
	
	enum Truth { false, true };
	enum Truth seeded = false, interval = false;
	int i;
	int seed, a, b, m, n;

	const char *usage_msg = "USAGE: random_matrix m n [--seed seed_num] [--interval a b]\n"
							"\tA random matrix of m rows and n columns will"
							" be produced for standard output. \n\tA seed may be specified; otherwise,"
							" system time will be used. \n\tAn interval of integers [a, b] may be specified\n";

	if(argc != 3 && argc != 5 && argc != 6 && argc != 8) {
		fputs(usage_msg, stderr);
		exit(-1);
	}

	if(sscanf(argv[1], "%i", &m) != 1) {
		fputs(usage_msg, stderr);
		exit(-1);
	}
	if(sscanf(argv[2], "%i", &n) != 1) {
		fputs(usage_msg, stderr);
		exit(-1);
	}
	for(i = 3; i < argc; i++) {
		if(!strcmp(argv[i], "--seed")) {
			seeded = true;
			if(i + 1 == argc) {
				fputs(usage_msg, stderr);
				exit(-1);
			}
			if(sscanf(argv[i + 1], "%i", &seed) != 1) {
				fputs(usage_msg, stderr);
				exit(-1);
			}
			i++;
		}
		else if(!strcmp(argv[i], "--interval")) {
			interval = true;
			if(i + 2 >= argc) {
				fputs(usage_msg, stderr);
				exit(-1);
			}
			if(sscanf(argv[i+1], "%i", &a) != 1) {
				fputs(usage_msg, stderr);
				exit(-1);
			}
			if(sscanf(argv[i+2], "%i", &b) != 1) {
				fputs(usage_msg, stderr);
				exit(-1);
			}
			i += 2;
		}
		else {
			fputs(usage_msg, stderr);
			exit(-1);
		}
	}


	matrix mt = allocate_matrix(m, n);

	if(interval == false) {
		a = 0;
		b = RAND_MAX;
	}
	
	fill_random_matrix(&mt, seeded == true ? seed : time(NULL), a, b);

	print_matrix(&mt);

	free_matrix(&mt);

	return 0;
}