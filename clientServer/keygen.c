/******************************************************************************
* keygen.c is a simple cipher text key generator using rand(). Usage is:
* keygen {int} > {textFileName} where int is an int from 1 to INT_MAX-1
* inclusive as the otp_* programs require a keygen generated text file of the
* key string. Compile with the "compileall" bash script or individually: 
* gcc keygen.c -o keygen -g -Wall
******************************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<time.h>

// Define maximum key length as MAX_INT - 1 (for the '/n').
#define MAX_KEY_LENGTH 100000

void usage();

int main(int argc, char **argv) {
	int i, length, temp;
	// Check arg count - must be 2.
	if (argc != 2) {
		usage();
		exit(1);
	}
	// Capture argv[1] and convert to int.
	length = atoi(argv[1]);
	// Check for proper range (arbitraily set by author).
	if (length < 1 || length > MAX_KEY_LENGTH) {
		usage();
		exit(1);
	}

	// Seed for rand() with the current time.
	srand(time(0));

	// Print "length" random chars that represent the key.
	for (i = 0; i < length; i++) {
		temp = rand() % 27;			// Yields random int from 0 - 26.
		if (temp == 26) {
			temp = ' ';			// Add ' ' to our 'ALPHA' char range.
		}
		else {
			temp += 65;			/* Convert "ASCII int" from ??? to uppercase.
								   Char 'A' ASCII to int is '65'.     [1] */
		}
		printf("%c", (char)temp);	// Print while casting "ASCII int" to chars. 
	}
	printf("\n");					// Add the new line to the end of the string.

	return 0;
}

void usage() {// [2]
	fprintf(stderr, "Error: syntax: keygen {int 1 - 100,000 inclusive}"
				 " > {yourTextFileName}\n");
}

/* CITATIONS: Adapted from the following:
[1] http://www.asciitable.com/
[2] http://forums.devshed.com/programming-42/printf-vs-fprintf-stderr-errors-385959.html
*/
