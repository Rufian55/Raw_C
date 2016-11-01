/****************************************************************************************
* forkDemo.c demonstrates forking a process.
* compile with gcc forkDemo.c -o forker -g -Wall
*****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) { // argc = argument count, argv = argument vector.
	printf("Parent ProcessID = %d\n", getpid());

	pid_t spawnpid = -55;
	int ten = 10;
	spawnpid = fork();

	// spawnpid = -1 (an error), 0 = successful child process created.
	switch (spawnpid) {
	case -1:
		perror("fork() returned -1, signifying an error occurred!\n");
		exit(1);
		break;
	case 0:
		ten = ten + 1;
		printf("I am the child! ten = %d and pid = %d\n", ten, getpid());
		break;
	default:
		ten = ten - 1;
		printf("I am the parent! ten = %d and pid = %d\n", ten, getpid());
		break;
	}
	printf("This line is executed by both the child and parent! pid = %d\n", getpid());

	return 0;
}