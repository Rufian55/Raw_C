/*******************************************************************
* getDir.c returns a directory based upon userID and <pid> for 
* general usage wherever needed...
* Compile with "gcc getDir.c -o getDir -g -Wall"
*******************************************************************/
#include <assert.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

// Prototype.
char* getDirName();

int main() {
	printf("Directory name is: %s\n", getDirName() );
	return 0;
}

// Returns a directory name based upon userID and processID <pid>
char* getDirName() {
	// Get the current process id.
	pid_t pid = getpid();

	// Get "real" userId. and their entry in /etc/passwd.
	uid_t uid = getuid();

	// Get user entry in /etc/passwd. (int to c-string)
	struct passwd *user = getpwuid(uid);

	/* Define the maximum length for the directory name.
	   Assumes pid is not larger than 1 Billion... */
	unsigned int bufferMaxLen = strlen(".dir.") + strlen(user->pw_name) + 10;

	// Allocate space for the directory name.
	char* dirName = malloc(bufferMaxLen * sizeof(char));
	assert(dirName != NULL);

	// Build dirName string.
	sprintf(dirName, "%s.dir.%d", user->pw_name, pid);

	return dirName;
}