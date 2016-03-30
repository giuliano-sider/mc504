CC = gcc
CFLAGS = -Wall -g
USERBIN = ~/bin/
LOCALBIN = ./bin/
CPLUSPLUS = g++
CPPFLAGS = -std=c++11 -Wall -g

PROGRAMS = threaded_matrix_multiply \
           random_matrix

INTERMEDIATES = matrix.o utility.o     			

# executables:

all: $(PROGRAMS)

threaded_matrix_multiply: threaded_matrix_multiply.c matrix.o utility.o
	$(CC) $(CFLAGS) -o $@ $^ -pthread

random_matrix: random_matrix.c matrix.o
	$(CC) $(CFLAGS) -o $@ $^



# libraries:

matrix.o: matrix.c
	$(CC) $(CFLAGS) -o $@ -c $^

utility.o: utility.c
	$(CC) $(CFLAGS) -o $@ -c $^

# activities

run:
	./run_instances.sh

clean:
	rm -f $(PROGRAMS) $(INTERMEDIATES)
