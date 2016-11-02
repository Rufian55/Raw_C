/****************************************************************************************
* forkingZombies.c demonstrates a zombie process creation and cleanup.
* Compile with "gcc forkingZombies.c -o zombie -g -Wall".
* To see the zombie process, open a second terminal and enter
* \" ps -elf | grep username \" look for the STAT "Z" entry. 
*****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
	pid_t spawnPid = -55;
	int childExitStatus = -55;
	spawnPid = fork();

	switch (spawnPid) {
	case -1:	// Setting spawnPid to fork() failed.
		perror("The fork() call has failed!\n");
		exit(1);
		break;
	case 0:		// Child process task completes.
		printf("CHILD: Terminating!\n");
		break;
	default:	// Parent process.
		printf("PARENT: making child a zombie for ten seconds;\n");
		printf("PARENT: Type \"ps -elf | grep \'username\'\" to see the defunct child\n");
		printf("PARENT: Sleeping...\n");
		fflush(stdout);
		sleep(10);	// 10 second delay before cleanup by waitpid() below.
		waitpid(spawnPid, &childExitStatus, 0);
		break;
	}
	printf("This will be executed by both of us!\n");
	exit(0);
	return 0;
}
