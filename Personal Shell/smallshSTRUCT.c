/******************************************************************************
* Chris Kearns, CS344-400-F16, Project 3, 17 Nov 2016, kearnsc@oregonstate.edu
* smallsh.c a "personal" shell written in C, designed to be used in a
* linux bash environment.  Handles built-n commands "cd", "status", and "exit".
* Also handles redirection with "<" and ">", comments with "#", and 
* backgrounding of processes with "&".
* Usage is: [arg1 arg2 ...arg512][< input_file][> output_file][&]
* Note: max argv: 512, max chars: 2,048.
* Compile with gcc smallshSTRUCT.c -o smallsh -g -Wall.						[1]
* This version includes a struct array to house and kill backgournded processes
* on exit rather than letting them complete and potentially "Z" for awhile... 
******************************************************************************/
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


/* A "stack" struct housing a pid_t array for
managing backgrounded process pid's. */
struct backgrounded_children {
	int numBGC;		// Number of Back Grounded Children.
	int capBGC;		// Capacity of bgChildArr.children array.
	pid_t *children;	// pushPID2bg() has a doubling call to realloc.
} bgChildArr;


// Prototypes.
void printStatus(int status);
void usage();
void cntlC(int signo);
void initBGC();
bool isInBackground(pid_t pid);
void clear_bgChildArr();
void pushPID2bg(pid_t child);
pid_t popPID2bg();
// Signal handlers.
void manageExit(int signo);


int main() {
	char* argv[512];		// Max argv 512 per specifications.
	int argc = 0;			// Running argument count on command line input.
	char input[2049];		// Max characters 2,048 + null terminator.
	char* inFileName = NULL;	// Input file name.
	char* outFileName = NULL;	// Output file name.
	char seperator[3] = " \n";	// Seperate command line strings by " " or "\n".
	char* token;			// Holder for individual command string words.
	bool isBackgrounded;		// Process in background bool.
	int fd = -1;			// File descriptor for file operations set to error.
	int fd2 = -1;			// File descriptor for file operations set to error, for dup2().
	int status = 0;			// Holder int for process status info.
	pid_t pid;			// Process pid.

	// Declare and initialize signal handler to ignore SIGINT (15) signal.
	struct sigaction notify;		// Declare signal handler struct.
	notify.sa_handler = SIG_IGN;		// Set handler attribute to macro SIG_IGN vs. cntlC;
	notify.sa_flags = 0;			// No flags needed.
	sigfillset(&notify.sa_mask);		// Set a mask that that masks all signals to notify struct.
	sigaction(SIGINT, &notify, NULL);	// Set SIGINT to be handled by struct notify.		[2][3][4]

	// Declare and initialize signal handler to for children to handle Ctrl C (signal 2).
	struct sigaction child;			// Declare signal handler struct.
	child.sa_handler = cntlC;		// Set handler attribute to function cntlC vs. SIG_IGN;
	child.sa_flags = 0;			// No flags needed.
	sigfillset(&child.sa_mask);		// Set a mask that that masks all signals to child struct.
	sigaction(SIGINT, &child, NULL);	// Set SIGINT to be handled by struct child.

	initBGC();				// Initialize the backgrounded proceses storage array.

	// Run shell.
	while (true) {
		isBackgrounded = false;	// Default to parent process in foreground.
		printf(": ");		// Print prompt. Extra space for readability of output.
		fflush(stdout);		// Flush stdout prompt.

		// Get smallsh's command line user input.
		if (fgets(input, 2049, stdin) == NULL) {// fgets() returns NULL on error.				[5]
			return 0;
		}

		// PROCESS COMMAND LINE c-string by parsing and tokenizing per specifications.
		argc = 0;	// Reset argc for next line of input.
		token = strtok(input, seperator);
		while (token != NULL) {
			if (strcmp(token, "<") == 0) {
				// Next token should be an input file.
				token = strtok(NULL, seperator);
				inFileName = strdup(token);
				token = strtok(NULL, seperator);
			}
			else if (strcmp(token, ">") == 0) {
				// Next token should be an output file.
				token = strtok(NULL, seperator);
				outFileName = strdup(token);
				token = strtok(NULL, seperator);
			}
			else if (strcmp(token, "&") == 0) {
				// & token indicates process to be run in background.
				isBackgrounded = true;
				break;
			}
			else {
				// Token is a command, argument or flag.
				argv[argc] = strdup(token);
				token = strtok(NULL, seperator);
				argc++;
			}
		}
		argv[argc] = NULL; // Last element of argv set to NULL for execvp() call.

	    /* BUILT-IN COMMANDS: */
	    // Handle comments and blank command line input by...
		if (argv[0] == NULL || *(argv[0]) == '#') {
			;// ...doing nothing.  There is no 'NOP' command in C...
		}
		// "cd" - Change Directory.										[6]
		else if (strcmp(argv[0], "cd") == 0) {
			if (argv[1] == NULL) {
				chdir(getenv("HOME"));
			}
			else {
				if (chdir(argv[1]) == -1) {
					perror("Error: call to chdir() failed ");
					status = 1;
				}
			}
		}
		// "status" - Print status information.
		else if (strcmp(argv[0], "status") == 0) {
			printStatus(status);
		}
		// "exit".
		else if (strcmp(argv[0], "exit") == 0) {
			_exit(0);
		}

		/* PROCESS AS A BASH COMMAND */
		else {
			// Fork parent process, call execvp on command.
			pid = fork();
			if (isBackgrounded) {
				pushPID2bg(pid);
			}
			switch (pid) {
			case -1:	// Error with call to fork().
				perror("Error: call to fork() failed ");
				status = 1;
				break;

			case 0:     // Child.
				if (!isBackgrounded) {				// Set up Child process for signals.
					child.sa_handler = SIG_DFL;		// Set handler to "no function".		 [7]
					sigaction(SIGINT, &child, NULL);	// Register signal handler for Child process.
				}
				// Infile redirection setup.
				if (inFileName != NULL && !isBackgrounded) {
					fd = open(inFileName, O_RDONLY);
					if (fd == -1) {
						printf("Error: cannot open '%s' for input.\n", inFileName);
						fflush(stdout);
						_exit(EXIT_FAILURE);//								[8]
					}
					else { // Close fd on execvp() call.
						fcntl(fd, F_SETFD, FD_CLOEXEC);
					}
					// Child gets duplicate file descriptor inFileName.					[9]
					if (dup2(fd, 0) == -1) {// 0 means 'stdin'.
						perror("Error: call to dup2() failed ");
						_exit(EXIT_FAILURE);
					}
				}
				else if (inFileName != NULL && isBackgrounded) {
					// Add backgrounded pid to storage array.
					pushPID2bg(pid);
					// Inhibit background processes from stdin (keyboard) input.
					inFileName = "dev/null";
					fd = open(inFileName, O_RDONLY);
					if (fd == -1) {
						// Error with open().
						perror("Error: call to open() failed ");
						_exit(EXIT_FAILURE);
					}
					else { // Close fd on execvp() call.
						fcntl(fd, F_SETFD, FD_CLOEXEC);
					}
					// Child gets duplicate file descriptor, "/dev/null"
					fd2 = dup2(fd, 0);// 0 means 'stdin'.
					if (fd2 == -1) {
						// Error with dup2().
						perror("Error: call to dup2() failed ");
						_exit(EXIT_FAILURE);
					}
				}

				// Outfile redirection setup.
				if (outFileName != NULL && !isBackgrounded ) {
					/* Open file write only, create if not found, truncate (overwrite),
					   set permisions octal. */
					fd = open(outFileName, O_WRONLY | O_CREAT | O_TRUNC, 0744);
					if (fd == -1) {
						printf("Error: cannot open '%s' for output.\n", outFileName);
						fflush(stdout);
						_exit(EXIT_FAILURE);
					}
					else { // Close fd on execvp() call.
						fcntl(fd, F_SETFD, FD_CLOEXEC);
					}
					// Child gets duplicate file descriptor, "outFileName".
					if (dup2(fd, 1) == -1) {// 1 means 'stdout'.
						// Error with dup2().
						perror("Error: call to dup2() failed ");
						_exit(EXIT_FAILURE);
					}
				}
				else if (outFileName != NULL && isBackgrounded) {
					// Add backgrounded pid to storage array.
					pushPID2bg(pid);
					// Inhibit background processes from stdout (terminal) output.
					outFileName = "/dev/null";
					fd = open(outFileName, O_WRONLY);
					if (fd == -1) {
						// Error with open().
						perror("Error: call to open() failed ");
						_exit(EXIT_FAILURE);
					}
					else { // Close fd on execvp() call.
						fcntl(fd, F_SETFD, FD_CLOEXEC);
					}
					// Child gets duplicate file descriptor, "/dev/null"
					fd2 = dup2(fd, 1);// 1 means 'stdout'.
					if (fd2 == -1) {
						// Error with dup2().
						perror("Error: call to dup2() failed ");
						_exit(EXIT_FAILURE);
					}
				}
				// Finally, execute the command.
				if (execvp(argv[0], argv)) {
					// Error with exec.
					printf("Error: '%s' command not found.\n", argv[0]);
					fflush(stdout);
					_exit(EXIT_FAILURE);
				}
				printf("\n");
				fflush(stdout);
				break;

			default:	// Parent.
				if (!isBackgrounded) {
					// Wait for foreground process to complete.
					waitpid(pid, &status, 0);
				}
				else {
					// Print background process pid.
					printf("Backgrounded pid is %i\n", pid);
					fflush(stdout);
					waitpid(pid, &status, WNOHANG);
					break;
				}
			}
		}

		// Memory management at bottom of loop.
		int i;
		for (i = 0; argv[i] != NULL; ++i) {
			free(argv[i]);
		}
		/* Calls to strdup() use malloc() to allocate memory for the duplicate				[10]
		   string, so free c-strings and pointers here on each loop iteration. */
		free(inFileName);
		inFileName = NULL;
		free(outFileName);
		outFileName = NULL;

		// Check for finished background processes.
<<<<<<< HEAD
		pid = waitpid(-1, &status, WNOHANG);
		// Check them all (-1 returned on waitpid() failure).								[11]
		while (pid > 0 && !isInBackground(pid))  {
=======
		pid = waitpid(pid, &status, WNOHANG);
		// Check them all (-1 returned on waitpid() failure).						[11]
		while (pid > 0 && isInBackground(pid))  {
>>>>>>> origin/master
			printf("Backgrounded pid %i finished: ", pid);
			fflush(stdout);
			popPID2bg(pid);
			printStatus(status);
			fflush(stdout);
			// Wait for the next finished process.
			pid = waitpid(-1, &status, WNOHANG);
		}
	}
	return 0;
}


/* Prints status of last command executed succesfully or otherwise.						[12]
*  param: status	- Updated by smallsh during command processing.
*  param: status	- Alternativly, signal status passed and printed. */
void printStatus(int status) {
	if (WIFEXITED(status)) {
		printf("Exit value %i\n", WEXITSTATUS(status));
		fflush(stdout);
	}
	else {
		printf("Terminated by signal %i\n", status);
		fflush(stdout);
	}
}


/* When user needs a reminder on smallsh() command line syntax. Not used at present
   in order to comply with specifications.*/
void usage() {
	printf("Usage is: [arg1 arg2 ...arg512][< input_file][> output_file][&]\n"
		"Note: max argv: 512, max chars: 2,048.\n");
	fflush(stdout);
}


/* Signal Handler cntlC() contains a simple re-entrant safe write() for printing
   the terminated by signal message upon receipt of Ctrl C (signal 2). */
void cntlC(int signo) {
	if (signo == 2) {
		char buffer[] = " Terminated by signal 2\n";
		write(STDOUT_FILENO, buffer, 24);
		fflush(stdout);
	}
}

/* Backgrounded Processes Struct Array Management Functions. */

/* Initialize the stack of backgrounded child processes. */
void initBGC() {
	bgChildArr.numBGC = 0;
	bgChildArr.capBGC = 4;
	bgChildArr.children = malloc(bgChildArr.capBGC * sizeof(pid_t));
}


/* Push child process pid onto the struct stack of backgrounded child processes.
*  param: child - The pid of a child process pushed. */
void pushPID2bg(pid_t child) {
	if (bgChildArr.numBGC == bgChildArr.capBGC) {
		bgChildArr.capBGC *= 2;
		bgChildArr.children = realloc(bgChildArr.children, (bgChildArr.capBGC * sizeof(pid_t)));
	}
	bgChildArr.children[bgChildArr.numBGC] = child;
	bgChildArr.numBGC++;
}

/* Pop a backgrounded child process off the stack and
*  returns:	pid of the popped child process or 0 if empty. */
pid_t popPID2bg() {
	if (bgChildArr.numBGC > 0) {
		bgChildArr.numBGC--;
		return bgChildArr.children[bgChildArr.numBGC + 1];
	}
	else {
		return 0;
	}
}


/* Determines whether a process was backgrounded.
*  param: pid - a running child process. */
bool isInBackground(pid_t pid) {
	int i;
	for (i = 0; i < bgChildArr.numBGC; i++) {
		if (pid == bgChildArr.children[i]) {
			return true;
		}
	}
	return false;
}


/* Destroy the stack of backgrounded child processes. */
void clear_bgChildArr() {
	free(bgChildArr.children);
}


/* Manage the exit process by killing all backgrounded children on the struct stack.
*  param: signo (ignored). */
void manageExit(int signo) {
	pid_t child;
	int i;
	for (i = 0; i < bgChildArr.numBGC; i++) {
		child = popPID2bg();
		kill(child, SIGINT);
	}
}


/* CITATIONS: All code by author and where indicated, adapted from the below sources:
[1] Various BLOCK 3 lectures and slides, Prof. B. Brewster, Oregon State University, CS344-400-F16.
[2] http://stackoverflow.com/questions/24393363/what-is-the-meaning-of-sigfillset-do-i-really-needed-it-in-my-implementation
[3] https://www.gnu.org/software/libc/manual/html_node/Flags-for-Sigaction.html#Flags-for-Sigaction
[4] http://cs.baylor.edu/~donahoo/practical/CSockets/code/SigAction.c
[5] http://stackoverflow.com/questions/21679063/return-value-of-fgets
[6] https://www.tutorialspoint.com/c_standard_library/c_function_getenv.htm
[7] http://en.cppreference.com/w/c/program/SIG_strategies
[8] http://stackoverflow.com/questions/5422831/what-is-the-difference-between-using-exit-exit-in-a-conventional-linux-fo
[9] http://man7.org/linux/man-pages/man2/dup.2.html
[10] https://linux.die.net/man/3/strdup
[11] https://www.google.com/webhp?sourceid=chrome-instant&ion=1&espv=2&ie=UTF-8#q=what%20does%20waitpid%20return
[12] http://pubs.opengroup.org/onlinepubs/9699919799/functions/wait.html
*/
