/*********************************************************
* cLine.c is a simple command line format checking demo.
* Compile with "gcc cLine.c -o cLine -g -Wall".
* Program exits with error 1, you know what to do! 
**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define NUM_ARGS 512
#define CL_LEN 2048

// Prototypes.
void usage();

int main(int argc, char **argv) {
	int i, CL_length = 0;

	// Check for command line specifications.
	for (i = 0; i < argc; i++) {
		CL_length += strlen(argv[i]);
	}
	if (argc > NUM_ARGS || CL_length > CL_LEN) {
		usage();
	}

	printf("Chars in Command Line: %d\n", CL_length);
	printf("argc = %d\n", argc);
	for (i = 0; i < argc; i++) {
		printf("arg %d = %s\n", i, argv[i]);
	}
	usage();// Just to see the usage error message...
	return 0;
}


void usage() {
	printf("\nUsage is: cLine [arg1 arg2 ...arg512][< input_file]"
	       "[> output_file]\nNote: max args: 512, max chars: 2,048.\n");
	exit(EXIT_FAILURE);
}
