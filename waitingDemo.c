/****************************************************************************************
* waitingDemo.c demonstrates waiting on processes to complete.
* compile with gcc waitingkDemo.c -o waiter -g -Wall
*****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
	printf("Parent pid = %d\n", getpid());
	int exitMethod;
	pid_t spawnpid;
	spawnpid = fork();

	if (spawnpid == 0) {// Child process. 
		printf("child: I'm sleeping\n");
		sleep(5);
	}
	else if (spawnpid > 0) {// Parent process.
		printf("parent: I'm waiting for child to finish!\n");
		pid_t exitpid = wait(&exitMethod);// wait() returns the pid of child.
		printf("exitpid = %d\n", exitpid);
		printf("parent: child exited [%d]\n", exitMethod);
	}
	else {
		printf("fork failed!\n");
		perror("fork()\n");
	}

	return 0;
}