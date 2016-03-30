


#include "utility.h"

void toc(const char *event_label) { 
	static time_t seconds = 0; // static members: not thread safe!! but we only use it in the main thread
	static long nanoseconds = 0;
	static struct timespec time_keeper;

	// clocks:
	// CLOCK_THREAD_CPUTIME_ID
	// CLOCK_PROCESS_CPUTIME_ID
	// CLOCK_MONOTONIC
	// CLOCK_REALTIME	

	if(clock_gettime(CLOCK_REALTIME, &time_keeper) == -1)
		perror("function toc() on line 45 called clock_gettime() and got an error condition");

	if(event_label != NULL) { // if passed NULL, it merely sets up the counter
		
		// calculate time difference between fetched and stored times
		if(nanoseconds > time_keeper.tv_nsec) {
			nanoseconds = 1000000000 - nanoseconds + time_keeper.tv_nsec;
			seconds = time_keeper.tv_sec - seconds - 1;
		}
		else {
			nanoseconds = time_keeper.tv_nsec - nanoseconds;
			seconds = time_keeper.tv_sec - seconds;
		}

		fprintf(stderr, "%s: %li seconds, %li nanoseconds\n", 
			event_label, seconds, nanoseconds
		);

	}
	
	seconds = time_keeper.tv_sec;
	nanoseconds = time_keeper.tv_nsec; // now keep the count

}
