/*****************************************************************************************
* sortByFile.c demonstrates god knows what...
* Compile with "gcc sortByFile.c -o sortyBoy -g -Wall".
* Call exectuable with "sortyBoy <desiredFilename>"
******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
	int sourceFD, targetFD, result;

	if (argc != 3)	{
		printf("Usage: sortByFiles <input filename> <output filename>\n");
		exit(1);
	}
	// Open source file for reading.
	sourceFD = open(argv[1], O_RDONLY);

	// Close sourceFD on an exec() call.
	fcntl(sourceFD, F_SETFD, FD_CLOEXEC);

	if (sourceFD == -1) {
		perror("Call to open() failed: ");
		exit(1);
	}

	printf("sourceFD == %d\n", sourceFD); // Written to terminal.

	// Open destination file for writing.
	targetFD = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

	// Close targetFD on an exec() call.
	fcntl(targetFD, F_SETFD, FD_CLOEXEC);

	if (targetFD == -1) {
		perror("Call to open(argv[2]) failed: ");
		exit(1);
	}

	printf("targetFD == %d\n", targetFD); // Written to terminal.

	/* Create a copy of the sourceFD file descriptor.
	   Sets FD 0 (stdin) to point to the same place that sourceFD
	   points. So, anything in stdin (like the contents of the
	   input file) will be used by this program. */
	result = dup2(sourceFD, 0);

	if (result == -1) {
		perror("Call to dup2(sourceFD) failed: ");
		exit(2);
	}

	// Check what was returned by dup2() call.
	printf("dup2(sourcedFD, 0) returns %d\n", result);

	/* Create a copy of the targetFD file descriptor.
	   Set FD 1 (stdout) to point to the same place that
	   targetFD points.  So, anything written to stdout
	   (like the result of sort) will go to targetFD,
	   which is the passed in output filename */
	result = dup2(targetFD, 1);

	if (result == -1) {
		perror("Call to dup2(targetFD) failed: ");
		exit(2);
	}

	/* Check what was returned by dup2() call. This will be
	   written to targetFD, so we comment it out. (Returns "1"). */
	//printf("dup2(targetFD, 1) returns %d\n", result);
	
	/* execlp() now starts with stdin and stdout
	   pointing to files, which are used by sort. 
	   Syntax is execlp(c-string1, c-string2, ... NULL) where
	   the c-strings are, in this case linux OS calls to 'sort'. */
	execlp("sort", "sort", NULL);

	return(3);
}