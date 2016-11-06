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
	int i, ch, exVal = 0;
	struct input cLine;
	while (1) {
		write(STDOUT_FILENO, ": ", 2);// Just for fun...
		fflush(stdout);
		fgets(cLine.inString, NAME_BUFFER_LEN, stdin);

		// Inhibit stdin/buffer overrun / type ahead user behaviour.
		if (!strchr(cLine.inString, '\n')) {
			// Consume rest of chars up to '\n'.
			while (((ch = getchar()) != EOF) && (ch != '\n'));
		}
		else {
			// Remove newline.
			cLine.inString[strlen(cLine.inString) - 1] = '\0';
		}

		// Constructor for populating cLine members from smallsh command line input.
		populateArguments(&cLine);

		// print arg list for debugging
//		for (i = 0; i < cLine.argCount; i++) {
//			printf("arg[%d] = %s\n", i, cLine.arguments[i]);
//		}

		// Iterate and process cLine.arguments[] array for BUILT IN commands, then OS commands.
		for (i = 0; i < cLine.argCount; i++) {
			if (strcmp(cLine.arguments[i], "exit") == 0) {
				memManager(&cLine);
				exit(0);
			}
			// BUILT IN 'cd' => [Change Dir].
			else if (strcmp(cLine.arguments[i], "cd") == 0) {
				// Check for and handle a cd argument.
				if (cLine.argCount - 1 > i) {// An argument exists after cd.
					exVal = chdir(cLine.arguments[i+1]);
				}
				else {// cd to HOME environment directory.
					exVal = chdir(getenv("HOME"));
				}
				if (exVal == -1) {
					perror("Call to cd failed: ");
					fflush(stdout);
				}
			}
			/* BUILT IN 'status' => [returns exit status or terminating
			   signal of last foreground process]. */
			else if (strcmp(cLine.arguments[i], "status") == 0) {
				printf("exit value %d\n", exVal);
				fflush(stdout);
			}
			else { // Operating SYSTEM COMMAND!
				printf("%s received\n", cLine.arguments[i]);
				fflush(stdout);
				pid_t spawnpid = -55;
				spawnpid = fork();
				printf("spawnpid = %d\n", spawnpid);
				// spawnpid = -1 (an error), 0 = successful child process created.
				switch (spawnpid) {
				case -1:
					perror("Call to fork() returned -1, signifying an error occurred!\n");
					exVal = -1;
					break;
				case 0:
					printf("Child process pid = %d\n", getpid());
					break;
				default:
					break;
				}
			}
		}

		memManager(&cLine);
	}
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
*						Functions populateArguments()							 [1]
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
		// Ignore # and /* comments, as applicable, to each line of input.
		if ( token[0] != '#' && strncmp(token, "/*", 2) != 0 ) {
			// Grow arguments array as the number of arguments increases.
			cLine->arguments = realloc(cLine->arguments, sizeof(char*) * ++spaces);
			if (cLine->arguments == NULL) {
				perror("Call to realloc() failed: ");
				exit(1);
			}
			// Note arguments is a 2D array, so malloc for each argument assigned by token.
			cLine->arguments[spaces - 1] = malloc(sizeof(char*) * strlen(token));
			if (cLine->arguments[spaces - 1] == NULL) {
				perror("Call to malloc() failed: ");
				exit(2);
			}
			// Add the token to the cLine->arguments[] array.
			strcpy(cLine->arguments[spaces - 1], token);
			token = strtok(NULL, " ");// Clear the token.
			cLine->argCount++;
		}
		else { break; }// Stop processing this line.
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