#ifndef MATRIX_H_
#define MATRIX_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct matrix {
	int m, n;
	int **data;
} matrix;

matrix allocate_matrix(int m, int n);
void free_matrix(matrix * mt);
void fill_random_matrix(matrix * mt, int seed, int a, int b);
void print_matrix(matrix *mt);

void fill_block_matrix(matrix * mt, int row_range_low, int row_range_high, 
					int col_range_low, int col_range_high, int val);

void block_matrix_multiply(matrix* out, matrix* mt1, matrix* mt2, 
		int row_range_low, int row_range_high, int col_range_low, int col_range_high
	);

#endif
// MATRIX_H_