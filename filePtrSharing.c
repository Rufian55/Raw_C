/*****************************************************************************************
* filePtrSharing.c demonstrates managing file pointers in multipath programming.  Program
* reads and writes via child and parent process to an single "shared" open file. 
* Compile with "gcc filePtrSharing.c -o fpShareDemo -g -Wall".
******************************************************************************************/
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int main() {
	pid_t forkPID;
	int childExitMethod;
	int fileDescriptor;
	char *newFilePath = "./newFile.txt";
	char readBuffer[8];
	memset(readBuffer, '\0', sizeof(readBuffer));

	printf("PARENT: Opening file.\n");
	fileDescriptor = open(newFilePath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fileDescriptor == -1) {
		printf("open() failed on \"%s\"\n", newFilePath);
		exit(1);
	}
	printf("PARENT: Writing 01234 to file.\n");
	write(fileDescriptor, "01234", 5);
	printf("PARENT: New FP position (all FP positions are zero-indexed): %d\n", lseek(fileDescriptor, 0, SEEK_CUR));
	fflush(stdout);

	printf("PARENT: Spawning child.\n");
	forkPID = fork();// Child will have access to fileDescriptor.

	switch (forkPID) {
	case -1: // Error.
		perror("Error on fork() call: ");
		exit(1);
		break;
	case 0: // Child.
		printf("CHILD: Writing C to file.\n");
		fflush(stdout);
		write(fileDescriptor, "C", 1);
		printf("CHILD: After write, new FP position: %d\n", lseek(fileDescriptor, 0, SEEK_CUR));
		fflush(stdout);
		printf("CHILD: lseek back 3 chars.\n");
		fflush(stdout);
		printf("CHILD: New FP position: %d\n", lseek(fileDescriptor, -3, SEEK_CUR));
		fflush(stdout);
		printf("CHILD: Reading char.\n");
		fflush(stdout);
		read(fileDescriptor, &readBuffer, 1);
		printf("CHILD: After read, new FP position: %d, char read was: %c\n", lseek(fileDescriptor, 0, SEEK_CUR), readBuffer[0]);
		fflush(stdout);
		break;
	default: // Parent.
		printf("PARENT: Writing P to file.\n");
		fflush(stdout);
		write(fileDescriptor, "P", 1);
		printf("PARENT: After write, new FP position: %d\n", lseek(fileDescriptor, 0, SEEK_CUR));
		fflush(stdout);
		printf("PARENT: lseek back 3 chars.\n");
		fflush(stdout);
		printf("PARENT: New FP position: %d\n", lseek(fileDescriptor, -3, SEEK_CUR));
		fflush(stdout);
		printf("PARENT: Reading char.\n");
		fflush(stdout);
		read(fileDescriptor, &readBuffer, 1);
		printf("PARENT: After read, new FP position: %d, char read was: %c\n", lseek(fileDescriptor, 0, SEEK_CUR), readBuffer[0]);
		fflush(stdout);
		// Wait for child process to finish.
		waitpid(forkPID, &childExitMethod, 0);
		lseek(fileDescriptor, 0, SEEK_SET);
		read(fileDescriptor, &readBuffer, 7);
		printf("PARENT: child terminated; file contents: %s\n", readBuffer);
		fflush(stdout);
		break;
	}
}