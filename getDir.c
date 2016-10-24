/*******************************************************************
* getDir.c returns a directory based upon userID and <pid> for 
* general usage wherever needed...
* Compile with "gcc getDir.c -o getDir -g -Wall".
* Note: if executed with valgrind call, will produce a memleak.
* See link at end of file or SO question on the topic.
*******************************************************************/
#include <assert.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>


int main() {
	// Get the current process id.
	pid_t pid = getpid();

	// Get "real" userId. and their entry in /etc/passwd.
	uid_t uid = getuid();

	// Get user entry in /etc/passwd.
	struct passwd *user = getpwuid(uid);

	/* Define the maximum length for the directory name.
	   Assumes pid is not larger than 1 Billion... */
	unsigned int bufferMaxLen = strlen(".dir.") + strlen(user->pw_name) + 10;

	// Allocate space for the directory name.
	char* dirName = malloc(bufferMaxLen *sizeof(char));
	assert(dirName != NULL);

	// Build dirName string.
	sprintf(dirName, "%s.dir.%d", user->pw_name, pid);

	printf("bufferMaxLen is: %d\n", bufferMaxLen);

	printf("Directory name is: %s\n", dirName);

	free(dirName);
	return 0;
}
// http://stackoverflow.com/questions/40226297/struct-passwd-is-source-of-memory-leak-how-to-properly-free#40226418