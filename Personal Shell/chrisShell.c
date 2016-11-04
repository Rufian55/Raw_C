#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define NAME_BUFFER_LEN 2048

struct input {
	char inString[NAME_BUFFER_LEN];
	int argCount;
	char **arguments;
};

// Prototypes.
void usage();
void populateArguments(struct input *);
void memManager(struct input *);

int main() {
	int ch;
	struct input cLine;
	printf(": ");
	fflush(stdout);
	fgets(cLine.inString, NAME_BUFFER_LEN, stdin);

	// Inhibit stdin/buffer overrun type ahead user behaviour.
	if (!strchr(cLine.inString, '\n')) {
		// Consume rest of chars up to '\n'.
		while (((ch = getchar()) != EOF) && (ch != '\n'));// [14]
	}
	else {
		// Remove newline.
		cLine.inString[strlen(cLine.inString) - 1] = '\0';
	}

	// Constructor for populating cLine members from smallsh command line input.
	populateArguments(&cLine);

	memManager(&cLine);

	return 0;
}

/********************************************************************************
*						Function usage()
* Called from main() on an out of range command line call. argc!>512, chars!>2048 
*********************************************************************************/
void usage() {
	printf("Usage is: [arg1 arg2 ...arg512][< input_file][> output_file][&]\n"
		  "Note: max args: 512, max chars: 2,048.\n");
	exit(EXIT_FAILURE);
}


/*********************************************************************************************
*						Functions populateArguments()
* The constuctor used to populate struct input cLine with the command line argument
* string, breaking it apart for appropriate processing.
* Param:	 struct input *cLine (passed by reference).
* Result: populated cLine->arguments array with all args seperated by index.
**********************************************************************************************/
void populateArguments(struct input *cLine) {
	cLine->arguments = NULL;
	cLine->argCount = 0;
	char *token = strtok(cLine->inString, " ");
	int spaces = 0;

	// Split string and append tokens to cLine->arguments[].
	while (token) {
		// Grow arguments array as the number of arguments increases.
		cLine->arguments = realloc(cLine->arguments, sizeof(char*) * ++spaces);
		if (cLine->arguments == NULL) {
			perror("Call to realloc() failed: ");
			exit(1);
		}
		// Note arguments is a 2D array, so malloc for each argument assigned by token.
		cLine->arguments[spaces-1] = malloc(sizeof(char*) * strlen(token));
		if (cLine->arguments[spaces-1] == NULL) {
			perror("Call to malloc() failed: ");
			exit(2);
		}

		strcpy(cLine->arguments[spaces -1], token);
		token = strtok(NULL, " ");
		cLine->argCount++;
	}

	/*// Print the resulting Array (for debugging).
	int i;
	for (i = 0; i < cLine->argCount; i++) {
		printf("arguments[%d] = %s\n", i, cLine->arguments[i]);
	}*/
}


/***************************************************************
*				Function memManager()
* The general purpose clean up method for the realloc calls
* for the cLine->arguemnts array and each tokenized argument
* cLine->arguments[argCount].
***************************************************************/
void memManager(struct input *cLine) {
	int i;
	for (i = 0; i < cLine->argCount; i++) {
		free(cLine->arguments[i]);
	}
	free(cLine->arguments);
}


/* CITATIONS:
[1] Adapted from: http://stackoverflow.com/questions/11198604/c-split-string-into-an-array-of-strings

*/