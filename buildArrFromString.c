/*******************************************************************************************
* buildArrFromString.c is a demo program showing how to build an array from a string.
* compile with gcc buildArrFromString.c -o arrBld -g -Wall
*******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int main() {
	char input[] = "cat dog cow < pig > owl";
	char **arguments = NULL;
	char *token = strtok(input, " ");
	int spaces = 0, i;

	// Split string and append tokens to arguements[].
	while (token) {
		arguments = realloc(arguments, sizeof(char*) * ++spaces);
		if (arguments == NULL) {
			perror("Call to realloc() failed: ");
			exit(1);
		}
		arguments[spaces - 1] = token;
		token = strtok(NULL, " ");
	}

	// Print the resulting Array.
	for (i = 0; i < (spaces); i++) {
		printf("arguments[%d] = %s\n", i, arguments[i]);
	}

	// Manage allocated memeory.
	free(arguments);
	return 0;
}
