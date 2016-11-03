/*****************************************************************************************
* redirect2file.c demonstrates redirecting input to file vs. default stdout.
* Compile with "gcc redirect2file.c -o redirect -g -Wall".
* Call resulting executable with "redirect <desiredFilename>"
******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
	if (argc == 1 || argc > 2) {
		printf("Usage: redirect2File <filename to redirect stdout to>\n");
		exit(1);
	}
	/* Open the input file (argv[1]) for write only. On success,
		open returns an int val, -1 otherwise. */
	int targetFD = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);

	// Error on open() fail.
	if (targetFD == -1) {
		perror("Call to open() failed: ");
		exit(1);
	}

	printf("targetFD == %d\n", targetFD); // Written to terminal

	/* dup2 args are input file(argv[1]) and 1(stdout)
		File descriptor 0 is stdin.
		File descriptor 1 is stdout.
		File descriptor 2 is stderr. */
	int result = dup2(targetFD, 1);

	if (result == -1) {
		perror("Call to dup2() failed: ");
		exit(2);
	}

	// printf call now redirects to to targetFD(argv[1]) and not stdout(terminal).
	printf("targetFD == %d\nresult == %d\n", targetFD, result); // Written to file

	return(0);
}
