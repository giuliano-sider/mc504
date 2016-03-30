/**
MATRIX_THREAD_BUNDLE = {
	"description": "created test and profile comparison for basic multithreaded operations with an integer matrix",
	"creator": "Giuliano Sider",
	"creation_date": "21/03/2016",
	"last_modified": "23/03/2016",
	"message": "see description",
	"license": "GPL v2",
	"version": 0.9,
	"issues": [
		"move the threaded multiplication to the matrix library, with different possible schemes for allocating work to threads",
		"avoid the code duplication (line 199-235, 236-277) where we check for the number of rows in mt1 and columns in mt2",		
		"add some new matrix operations like addition, that invoke the thread allocator separately",
		"move the time function into some utility library for profiling different code. make it non-static so that different threads can use it?"
	]
}
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "matrix.h"
#include "utility.h"
//#include <sys/time.h>

/*
#define PROFILE

#ifdef PROFILE

#define tic(MESSAGE) toc(MESSAGE)

#elif

#define tic(MESSAGE) 

#endif 
// PROFILE
*/

#define MAX(A,B) ( (A) >= (B) ? (A) : (B) )

#define tic(MESSAGE) (profile == true ? toc(MESSAGE) : (void) 0 )





typedef struct mat_thread_info {
	matrix* out; matrix* a; matrix* b; int rows_from; int rows_to; int cols_from; int cols_to;
} mat_thread_info; // used to pass data to threads

void* forward_matrix_mul(void *args) { 
// starting point of our auxiliary threads. it forwards the arguments to the matrix multiplier.
// since this is somewhat internal to the matrix library, we should probably move it there (passing suggested number of threads/thread allocation strategy as a parameter)
	struct mat_thread_info *info = (mat_thread_info*) args;
	block_matrix_multiply(info->out, info->a, info->b, 
		info->rows_from, info->rows_to, info->cols_from, info->cols_to 
	);
	
	return NULL;
}


int main(int argc, char **argv) {
	
	enum Truth { false, true }; 
	enum Truth threaded = false, profile = false;
	int threads = 42; // default number of threads spun, if applicable

	const char *usage_msg = "USAGE: threaded_matrix_multiply [--threaded [num_threads]] [--profile]\n"
							"\n\tTakes two matrices from stdin"
				 			" and multiplies them. \n\tFirst line is 2 integers: m1 n1"
				 			"\n\tFollowed by m1 lines with n1 integers"
				 			"\n\tAnd then a line with 2 integers: m2 n2"
				 			"\n\tFollowed by m2 lines with n2 integers."
				 			"\n\tSpecify '--threaded' to parallelize the" 
				 			" multiplication, with an optional suggested number of threads."
				 			"\n\tSpecify '--profile' to measure time taken for key operations."
				 			"\n\tExample: threaded_matrix_multiply < my_mats --threaded 41 --profile > my_output 2> my_profile\n";
	int i, j, m1, n1, m2, n2;


	for(i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "--threaded")) {
			threaded = true;
			if(i + 1 < argc) { // check to see if num_threads was passed
				if(sscanf(argv[i+1],"%i", &threads) == 1) // if we have a successful match for an integer argument
					i++; // then absorb this argument and move on the next
			}
		}
		else if(!strcmp(argv[i], "--profile")) {
			profile = true;
		}
		else if(strstr(argv[i], "help") != NULL) {
			fprintf(stderr, "%s", usage_msg);
			exit(0);
		}
		else {
			fprintf(stderr, "%s", usage_msg);
			exit(-1);
		}
	}				 		

	if( scanf("%i %i", &m1, &n1) != 2 ) {
		fprintf(stderr, "error reading the first line of first matrix\n%s", usage_msg);
		exit(-1);
	}
		

	//time 0: begin
	tic(NULL);

	matrix mt1 = allocate_matrix(m1, n1);
	tic("first matrix allocation");

	
	for(i = 0; i < m1; i++) {
		for(j = 0; j < n1; j++) {
			if( scanf("%i", &mt1.data[i][j] ) != 1) {
				fprintf(stderr, "error reading the first matrix\n%s", usage_msg);
				exit(-1);
			}
		}
	}
	tic("first matrix read");

	if( scanf("%i %i", &m2, &n2) != 2 ) {
		fprintf(stderr, "error reading the first line of second matrix\n%s", usage_msg);
		exit(-1);
	}

	matrix mt2 = allocate_matrix(m2, n2);
	tic("second matrix allocation");

	for(i = 0; i < m1; i++) {
		for(j = 0; j < n1; j++) {
			if( scanf("%i", &mt2.data[i][j] ) != 1) {
				fprintf(stderr, "error reading the second matrix\n%s", usage_msg);
				exit(-1);
			}
		}
	}
	tic("second matrix read");

	if(n1 != m2) {
		fprintf(stderr, "The matrices have incompatible sizes for multiplication: the first has %i columns"
					  " and the second has %i rows\n%s", n1, m1, usage_msg
			);
		exit(-1);
	}

	matrix output_mt = allocate_matrix(m1, n2);
	tic("output matrix allocation");

	// not necessary. multiply zeroes it out
	//fill_block_matrix(&output_mt, 0, output_mt.m, 0, output_mt.n, 0);
	//tic("output matrix initialization");

	if(threaded == false) {

		block_matrix_multiply(&output_mt, &mt1, &mt2, 0, mt1.m, 0, mt2.n);

		tic("sequential (non-threaded) matrix multiplication");
	}
	else { // ISSUE 2: code duplication


		if(mt1.m > mt2.n) { // split work along rows since there are more rows in the output matrix than there are columns

			int rows_per_thread = mt1.m / threads;
		
			if(rows_per_thread == 0) {
				rows_per_thread = 1;
				threads = mt1.m;
			}
			pthread_t *thr = (pthread_t *) malloc(sizeof(pthread_t) * threads);
			mat_thread_info *mat_threads = (mat_thread_info*) malloc(sizeof(mat_thread_info) * threads);
			for(i = 0; i < threads; i++) {
				mat_threads[i].out = &output_mt;
				mat_threads[i].a = &mt1;
				mat_threads[i].b = &mt2;
				mat_threads[i].rows_from = i*rows_per_thread;
				mat_threads[i].rows_to = (i+1)*rows_per_thread;
				mat_threads[i].cols_from = 0;
				mat_threads[i].cols_to = mt2.n;
				if( pthread_create(&thr[i], NULL, forward_matrix_mul, (void*) &mat_threads[i] ) != 0) {
					perror("error calling pthread_create at line 177 of main");
					exit(-1);
				}
			}

			block_matrix_multiply(&output_mt, &mt1, &mt2, threads*rows_per_thread, mt1.m, 0, mt2.n);

			for(i = 0; i < threads; i++) {
				if(pthread_join(thr[i], NULL)) {
					perror("pthread_join failed at main");
					exit(-1);
				}
			}
			free(thr);
			free(mat_threads);
			// main does the leftover work

		}
		else { // split work along columns, since there are more columns than rows in the output matrix

			int columns_per_thread = mt2.n / threads;
		
			if(columns_per_thread == 0) {
				columns_per_thread = 1;
				threads = mt2.n;
			}
			pthread_t *thr = (pthread_t *) malloc(sizeof(pthread_t) * threads);
			mat_thread_info *mat_threads = (mat_thread_info*) malloc(sizeof(mat_thread_info) * threads);
			for(i = 0; i < threads; i++) {
				mat_threads[i].out = &output_mt;
				mat_threads[i].a = &mt1;
				mat_threads[i].b = &mt2;
				mat_threads[i].rows_from = 0;
				mat_threads[i].rows_to = mt1.m;
				mat_threads[i].cols_from = i*columns_per_thread;
				mat_threads[i].cols_to = (i+1)*columns_per_thread;
				if( pthread_create(&thr[i], NULL, forward_matrix_mul, (void*) &mat_threads[i] ) != 0) {
					perror("error calling pthread_create at line 177 of main");
					exit(-1);
				}
			}

			block_matrix_multiply(&output_mt, &mt1, &mt2, 0, mt1.m, threads*columns_per_thread, mt2.n);

			for(i = 0; i < threads; i++) {
				if(pthread_join(thr[i], NULL)) {
					perror("pthread_join failed at main");
					exit(-1);
				}
			}
			free(thr);
			free(mat_threads);
			// main does the leftover work

		}


		tic("parallel (threaded) matrix multiplication");
	}

	print_matrix(&output_mt);
	tic("output matrix printout");



	free_matrix(&mt1);
	free_matrix(&mt2);
	free_matrix(&output_mt);
	tic("matrix deallocations");

	return 0;
}