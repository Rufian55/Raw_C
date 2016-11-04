/****************************************************************************************
* pipeDemo3.c demonstrates use of FIFO pipes with inter process communications and how to
* read data from a FIFO pipe in chunks via a small buffer with some pointer arithmetic.
* FIFO's are treated in similar fashion to files, syntax is just a bit different.
* Compile with "gcc pipeDemo3.c -o piper3 -g -Wall".
*****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>


int  main() {
	int r, newfifo, fd;
	char completeMessage[512], readBuffer[10];
	char stringToWrite[20] = "CHILD: Hi Parent!@@";
	pid_t spawnpid;
	char* FIFOfilename = "myNewFifo";
	newfifo = mkfifo(FIFOfilename, 0644);				// Declare the persistant pipe (FIFO).
	spawnpid = fork();								// Fork the child, which will write into the pipe.

	switch (spawnpid) {
	case 0: // Child.
		fd = open(FIFOfilename, O_WRONLY);				// Open the FIFO for writing.
		if (fd == -1) {
			perror("CHILD: Call to open() failed: ");
			exit(1);
			break;
		}
		write(fd, stringToWrite, strlen(stringToWrite));	// Write the entire string to the pipe.
		exit(0);									// Terminate Child.
		break;
	default: // Parent.
		fd = open(FIFOfilename, O_RDONLY);							// Open the FIFO for reading.
		memset(completeMessage, '\0', sizeof(completeMessage));		// Clear the buffer.
		while (strstr(completeMessage, "@@") == NULL) {				// As long as we haven't found the terminal...
			memset(readBuffer, '\0', sizeof(readBuffer));			// Also clear this buffer.
			r = read(fd, readBuffer, sizeof(readBuffer) - 1);			// Get the next chunk.
			strcat(completeMessage, readBuffer);					// Add that chunk to what we have so far.
			printf("PARENT: Message received from child: \"%s\", total: \"%s\"\n", readBuffer, completeMessage);
			// Error checks.
			if (r == -1) {										
				printf("Parent: r == -1, exiting!\n");
				break;
			}
			if (r == 0) {
				printf("Parent: r == 0, exiting!\n");
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
		completeMessage[terminalLocation] = '\0';						  // End the string early to wipe out the terminal.
		printf("PARENT: Complete string: \"%s\"\n", completeMessage);
		remove(FIFOfilename);										  // Delete the FIFO.
		break;
	}
	return 0;
}
