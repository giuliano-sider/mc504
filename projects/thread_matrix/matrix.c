#include "matrix.h"

matrix allocate_matrix(int m, int n) {
	matrix mt = { m, n, NULL };
	mt.data = (int **) malloc(sizeof(int*) * m);
	int i;
	for(i = 0; i < m; i++)
		mt.data[i] = (int*) malloc(sizeof(int) * n);
	return mt;
}

void free_matrix(matrix * mt) {
	int i;
	for(i = 0; i < mt->m; i++)
		free(mt->data[i]);
	free(mt->data);
	mt->data = NULL;
}

void fill_block_matrix(matrix * mt, int row_range_low, int row_range_high, 
					int col_range_low, int col_range_high, int val) {
	int i, j;
	for(i = row_range_low; i < row_range_high; i++) {
		for(j = col_range_low; j < col_range_high; j++) {
			mt->data[i][j] = val;
		}
	}
}

void fill_random_matrix(matrix * mt, int seed, int a, int b) {
	int i, j;
	srand(seed);
	for(i = 0; i < mt->m; i++) {
		for(j = 0; j < mt->n; j++) {
			mt->data[i][j] = a + rand() % (b - a + 1);
		}
	}
}

void print_matrix(matrix *mt) {
	int i, j;
	printf("%i %i\n", mt->m, mt->m);
	for(i = 0; i < mt->m; i++) {
		for(j = 0; j < mt->n; j++, putchar(' ') ) {
			printf("%i", mt->data[i][j]);
		}
		putchar('\n');
	}
}

void block_matrix_multiply(matrix* out, matrix* mt1, matrix* mt2, 
		int row_range_low, int row_range_high, int col_range_low, int col_range_high
	) {

	int i, j, k;

	fill_block_matrix(out, row_range_low, row_range_high, col_range_low, col_range_high, 0);

	for(i = row_range_low; i < row_range_high; i++) {
		for(j = col_range_low; j < col_range_high; j++) {
			for(k = 0; k < mt1->n; k++) {
				out->data[i][j] += mt1->data[i][k] * mt2->data[k][j];
			}
		}
	}


}