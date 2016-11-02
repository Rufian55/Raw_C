/****************************************************************************************
* signalsDemo.c demonstrates use of signals with inter process communications piping.
* Compile with "gcc signalsDemo.c -o signal -g -Wall".
*****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char **argv) {
	int r;
	int pipeFDs[2];	// pipeFDs[0] is read end of pipe, pipeFDs[1] is write end of pipe.
	char message[512];
	pid_t spawnpid;

	if (pipe(pipeFDs) == -1) { 
		perror("An error has occurred with the call to pipe(): ");
	}

	spawnpid = fork();
	switch (spawnpid) {
		case 0:					// Child.
			close(pipeFDs[0]);		// Close input file descriptor.
			write(pipeFDs[1], "Hello Parent, this is the Child!", 33);
			exit(0);
		default:				// Parent.
			close(pipeFDs[1]);		// Close output file descriptor.
			r = read(pipeFDs[0], message, sizeof(message));
			if (r > 0) {
				printf("Message received from child: %s\n", message);
			}
			exit(0);
	}
	return 0;
}
