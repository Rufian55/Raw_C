/****************************************************************************************
* exitDemo.c demonstrates checking exit status of processes.
* compile with gcc exitDemo.c -o exiter -g -Wall
*****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
	printf("Parent pid = %d\n", getpid());

	int ten, status;

	if (fork() == 0) {// fork successful.
		ten = 10;
		printf("Child's pid = %d: sleeping... and then exiting with number %d\n", getpid(), ten);
		sleep(2);
		exit(ten);
	}
	else {
		printf("Parent: Waiting for Child to finish, Parent's pid = %d\n", getpid());
		wait(&status);
		if (WIFEXITED(status)) {
			printf("Child's exit status was %d\n", WEXITSTATUS(status));
		} else {
			printf("Child terminated with a signal.\n");
		}
	}

	return 0;
}