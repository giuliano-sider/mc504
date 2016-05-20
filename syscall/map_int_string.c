/* 
 * Teste da nova chamada de sistema
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <unistd.h>

#define SYS_GETKEY 379
#define SYS_SETKEY 380

#define TEST_SIZE 150000

int main() {
	/*
	 * Vamos adicionar pares chaves/valor no nosso mapa 
	 * (localizado no kernel space, ele guarda ponteiros para nossas strings)
	 */
	
	int i;

	/* old test #1
	char *strings[] = {
		"string123456789",
		"string123456798",
		"string123456879",
		"string123456897",
		"string123456978",
		"string123456987",
		"string123457689",
		"string123457698",
		"string123457869",
		"string123457896",
		"string123457968",
		"string123457986",
		"string123458679"
	};
	puts("beginning k hash map test 1: \n");
	hash_table = construct_map(NULL); use default hash function

	put_mapping(hash_table, 1 << 31, strings[12]);	
	printf("this is the mapping for key %i in the table: %s\n",
		1 << 31, get_mapping(hash_table, 1 << 31)
	);
	printf("this is the mapping (that we have just removed) for key %i in the table: %s\n",
		1 << 31, remove_mapping(hash_table, 1 << 31)
	);

	destroy_map(hash_table);
	*/



	puts("\n\n\nbeginning k hash map test 2:\n");

	/*system("cat thisfile.c | tail -796 | head -795 > strings.neat.txt");*/
	char *filename = "strings_neat.txt";
	FILE *fp = fopen(filename, "r");
	const int STRING_FROM_FILE = 795;
	char **stringsfromfile = malloc(STRING_FROM_FILE * sizeof(char*));
	int arraysize = 0;
	while (arraysize < STRING_FROM_FILE) {
		char buffer[20];
		if (fscanf(fp, " %19s ", buffer) != 1)
			break;
		stringsfromfile[arraysize] = malloc(20*sizeof(char));
		strcpy(stringsfromfile[arraysize], buffer);
		arraysize += 1;
	}
	/*assert(arraysize == STRING_FROM_FILE);*/

	/*hash_table2 = construct_map(NULL);*/

	srand(time(NULL));
	const int test_size = TEST_SIZE;
	int *keys_used  = malloc(sizeof(int) * test_size);
	int *strings_used = malloc(sizeof(int) * test_size); 
	/* we need to keep the index of the strings,
	 * since we want to test setkey's removal feature, which doesn't return the mapped string
	 */
	for (i = 0; i < test_size; i++) {
		keys_used[i] = rand();
		strings_used[i] = rand() % arraysize;
		if (syscall(SYS_SETKEY, keys_used[i], stringsfromfile[strings_used[i]]) == -1)
			perror("SYS_SETKEY call failed at line 83");
	}

	puts("this is the hash map after a big number of insertions:\n");
	/*print_hash_map(hash_table2);*/
	printf("{\n");
	for (i = 0; i < test_size; i++) {
		char *s = NULL;
		if (syscall(SYS_GETKEY, keys_used[i], &s) == -1) {
			perror("SYS_GETKEY call failed at line 92");
			exit(-1);	
		}
		printf("\"%i\": \"%s\"\n", keys_used[i], s);
	}
	printf("}\n");

	for (i = 0; i < test_size; i++) {
		if (rand() % 2 == 0) { /* 50% chance of deleting the key */
			/*char *s = remove_mapping(hash_table2, keys_used[i]);*/
			if (syscall(SYS_SETKEY, keys_used[i], NULL) == -1) /* removes the mapping */
				perror("SYS_SETKEY call failed at line 100");

			printf("the following key, %i, with value %s, was just removed from"
				"the map\n", keys_used[i], stringsfromfile[strings_used[i]]);
		}
	}

	puts("this is the hash map after a big number of deletions:\n");
	/*print_hash_map(hash_table2);*/
	printf("{\n");
	for (i = 0; i < test_size; i++) {
		char *s = NULL;
		if (syscall(SYS_GETKEY, keys_used[i], &s) == -1) {
			perror("SYS_GETKEY call failed at line 113");
			exit(-1);	
		}
		if (s != NULL)
			printf("\"%i\": \"%s\"\n", keys_used[i], s);
	}
	printf("}\n");

	free(keys_used);
	free(strings_used);
	
	/*destroy_map(hash_table2);*/
	for(i = 0; i < arraysize; i++)
		free(stringsfromfile[i]);
	free(stringsfromfile);
	fclose(fp);

	return 0;

	
	
	
}
