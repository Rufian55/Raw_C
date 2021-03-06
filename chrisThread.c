/******************************************************************************
* FILE: chrisThread.c	Date: 21 Oct 2016
* Pthreads and Mutex demo program.
******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define NUM_THREADS	5

void* perform_work(void* arguments) {
	int passed_in_value;
	passed_in_value = *((int *)arguments);
	printf("Hello World! It's me, thread with argument %d!\n", passed_in_value);
	return NULL;
}

int main(void) {
	pthread_t threads[NUM_THREADS];
	int thread_args[NUM_THREADS];
	int result_code, index;

	for (index = 0; index < NUM_THREADS; ++index) {
		// Create all threads one by one.
		thread_args[index] = index;
		printf("In main: creating thread %d\n", index);
		result_code = pthread_create(&threads[index], NULL, perform_work, (void *)&thread_args[index]);
		assert(0 == result_code);
	}
	// Wait for each thread to complete.
	for (index = 0; index < NUM_THREADS; ++index) {
		result_code = pthread_join(threads[index], NULL);
		printf("In main: thread %d has completed\n", index);
		assert(0 == result_code);
	}
	printf("In main: All threads have completed sucessfully!\n");
	exit(EXIT_SUCCESS);
}
