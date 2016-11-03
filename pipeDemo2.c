/****************************************************************************************
* pipeDemo2.c demonstrates use of pipes with inter process communications and how to
* read data from a pipe in chunks via a small buffer with some pointer arithmetic.
* Compile with "gcc pipeDemo2.c -o piper2 -g -Wall".
*****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


int  main() {
	int r, pipeFDs[2];
	char completeMessage[512], readBuffer[10];
	pid_t spawnpid;

	// Create the pipe with error check.
	if (pipe(pipeFDs) == -1) {
		perror("Call to pipe(pipeFDs) failed: ");
		exit(1);
	}

	// Fork the child, which will write into the pipe.
	spawnpid = fork();

	switch (spawnpid) {
	case 0: // Child.
		close(pipeFDs[0]);				// Close the input file descriptor.
		write(pipeFDs[1], "CHILD: Hi parent!@@", 19);	// Write the entire string into the pipe.
		exit(0);					// Terminate Child.
		break;
	default: // Parent.
		close(pipeFDs[1]);							// Close output file descriptor.
		memset(completeMessage, '\0', sizeof(completeMessage));			// Clear the buffer.
		while (strstr(completeMessage, "@@") == NULL) {				// As long as we haven't found the terminal...
			memset(readBuffer, '\0', sizeof(readBuffer));			// Clear the buffer.
			r = read(pipeFDs[0], readBuffer, sizeof(readBuffer) - 1);	// Get the next chunk.
			strcat(completeMessage, readBuffer);				// Add that chunk to what we have so far.
			printf("PARENT: Message received from child: \"%s\", total: \"%s\"\n", readBuffer, completeMessage);
			if (r == -1) {
				printf("r == -1\n");
				break;
			}
			// Check for errors
			if (r == 0) {
				printf("r == 0\n");
				break;
			}
		}

		/* Pointer arithmetic: terminalLocation = 17.  This is calculated by "strstr(completeMessage, "@@")"
		   returning its memory address. Remember, @@ looks odd, but it is still a c-string array, the base
		   of which is at a memory address, say 1017. Then, subtracting from that completeMessage, whose 
		   memory address would be 1000 since its 17 chars long, yields 17, thus yeilding the index of @@'s
		   starting point which we can then chop off.  Note the sneaky casts to long int and int. */
		int temp1 = (long)strstr(completeMessage, "@@");
		printf("Memory Address of @@ = %d\n", temp1);
		int temp2 = (long)completeMessage;
		printf("Memory Address of completeMessage = %d\n", temp2);
		printf("So, subtracting temp2 from temp1 = %d\n", (temp1 - temp2));

		// Note that the actual pointer arithmetic here does not need a cast to long int...
		int terminalLocation = strstr(completeMessage, "@@") - completeMessage; // Where is the terminal?
		completeMessage[terminalLocation] = '\0';				// End the string early to wipe out the terminal.
		printf("PARENT: Complete string: \"%s\"\n", completeMessage);
		break;
	}
	return 0;
}
