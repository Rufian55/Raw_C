/******************************************************************************
* Chris Kearns, CS344-400-F16, Project 3, 17 Nov 2016, kearnsc@oregonstate.edu
* smallsh.c a "personal" shell written in C, designed to be used in a
* linux bash environment.  Handles built-n commands "cd", "status", and "exit".
* Also handles redirection with "<" and ">", comments wiht "#", and 
* backgrounding of processes with "&".
* Usage is: [arg1 arg2 ...arg512][< input_file][> output_file][&]
* Note: max argv: 512, max chars: 2,048.
* Compile with gcc smallsh.c -o smallsh -g -Wall.						[1]
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

// Prototypes.
void printStatus(int status);
void usage();

int main() {
	char* argv[512];		// Max argv 512 per specifications.
	int argc = 0;			// Running argument count on command line input.
	char input[2049];		// Max characters 2,048 + null terminator.
	char* inFileName = NULL;	// Input file name.
	char* outFileName = NULL;	// Output file name.
	char seperator[3] = " \n";	// Seperate command line strings by " " or "\n".
	char* token;			// Holder for individual command string words.
	bool isBackgrounded;		// Process in background bool.
	int fd = -1;			// Input file descriptor for file operations set to error.
	int status = 0;			// Holder int for process status info.
	pid_t pid;			// Process pid.

	// Declare and initialize signal handler to ignore SIGINT (15) signal.
	struct sigaction notify;		// Declare signal handler struct.
	notify.sa_handler = SIG_IGN;		// Set handler attribute to simply SIG_IGN.
	notify.sa_flags = 0;			// No flags needed.
	sigfillset(&notify.sa_mask);		// Set a mask that that masks all signals to notify struct.
	sigaction(SIGINT, &notify, NULL);	// Set SIGINT to be handled by struct notify.		[2][3][4]

	// Run shell.
	while (true) {
		isBackgrounded = 0;	// Default to parent process in foreground.
		printf(": ");       // Print prompt. Extra space for readability of output.
		fflush(stdout);     // Flush stdout prompt.

		// Get smallsh's command line user input.
		if (fgets(input, 2049, stdin) == NULL) {// fgets() returns NULL on error.		[5]
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
				isBackgrounded = 1;
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
		// "cd" - Change Directory.													[6]
		else if (strcmp(argv[0], "cd") == 0) {
			if (argv[1] == NULL) {
				chdir(getenv("HOME"));
			}
			else {
				chdir(argv[1]);
			}
		}
		// "status" - Print status information.
		else if (strcmp(argv[0], "status") == 0) {
			printStatus(status);
		}
		// "exit".
		else if (strcmp(argv[0], "exit") == 0) {
			exit(0);
		}

		/* PROCESS AS A BASH COMMAND */
		else {
			// Fork parent process, call execvp on command.
			pid = fork();
			switch (pid) {
			case -1:	// Error with call to fork().
				perror("Error: call to fork() failed ");
				status = 1;
				break;

			case 0:     // Child.		 			   [7]
				if (!isBackgrounded) {				// Set up Child process for signals.
					notify.sa_handler = SIG_DFL;		// Set handler to "no function".
					sigaction(SIGINT, &notify, NULL);	// Register signal handler for Child process.
				}
				// Infile redirection setup.
				if (inFileName != NULL && !isBackgrounded) {
					fd = open(inFileName, O_RDONLY);
					if (fd == -1) {
						printf("Error: cannot open '%s' for input.\n", inFileName);
						fflush(stdout);
						_Exit(1);
					}
					// Child gets duplicate file descriptor inFileName.			[8]
					if (dup2(fd, 0) == -1) {// 0 means 'stdin'.
						perror("Error: call to dup2() failed ");
						_Exit(1);
					}
					close(fd);
				}
				else if (inFileName != NULL && isBackgrounded) {
					// Inhibit background processes from stdin (keyboard) input.
					fd = open("/dev/null", O_RDONLY);
					if (fd == -1) {
						// Error with open().
						perror("Error: call to open() failed ");
						_Exit(1);
					}
					// Child gets duplicate file descriptor, "/dev/null"
					if (dup2(fd, 0) == -1) {// 0 means 'stdin'.
						// Error with dup2().
						perror("Error: call to dup2() failed ");
						_Exit(1);
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
						_Exit(1);
					}
					// Child gets duplicate file descriptor, "outFileName".
					if (dup2(fd, 1) == -1) {// 1 means 'stdout'.
						// Error with dup2().
						perror("Error: call to dup2() failed ");
						_Exit(1);
					}
					close(fd);
				}
				else if (outFileName != NULL && isBackgrounded) {
					// Inhibit background processes from stdout (terminal) output.
					fd = open("/dev/null", O_WRONLY);
					if (fd == -1) {
						// Error with open().
						perror("Error: call to open() failed ");
						_Exit(1);
					}
					// Child gets duplicate file descriptor, "/dev/null"
					if (dup2(fd, 1) == -1) {// 1 means 'stdout'.
						// Error with dup2().
						perror("Error: call to dup2() failed ");
						_Exit(1);
					}
				}
				// Finally, execute the command.
				if (execvp(argv[0], argv)) {
					// Error with exec.
					printf("Error: '%s' command not found.\n", argv[0]);
					fflush(stdout);
					_Exit(1);
				}
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
					break;
				}
			}
		}

		// Memory management at bottom of loop.
		int i;
		for (i = 0; argv[i] != NULL; ++i) {
			free(argv[i]);
		}
		/* Calls to strdup() use malloc() to allocate memory for the duplicate				[9]
		   string, so free c-strings and pointers here on each loop iteration. */
		free(inFileName);
		inFileName = NULL;
		free(outFileName);
		outFileName = NULL;

		// Check for finished background processes.
		pid = waitpid(-1, &status, WNOHANG);
		// Check them all (-1 returned on waitpid() failure).						[10]
		while (pid > 0) {
			printf("Backgrounded pid %i finished: ", pid);
			fflush(stdout);
			printStatus(status);
			fflush(stdout);
			// Wait for the next finished process.
			pid = waitpid(-1, &status, WNOHANG);
		}
	}
	return 0;
}


/* Prints status of last command executed succesfully or otherwise.						[11]
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


/* CITATIONS: All code by author and where indicated, adapted from the below sources:
[1] Various BLOCK 3 lectures and slides, Prof. B. Brewster, Oregon State University, CS344-400-F16.
[2] http://stackoverflow.com/questions/24393363/what-is-the-meaning-of-sigfillset-do-i-really-needed-it-in-my-implementation
[3] https://www.gnu.org/software/libc/manual/html_node/Flags-for-Sigaction.html#Flags-for-Sigaction
[4] http://cs.baylor.edu/~donahoo/practical/CSockets/code/SigAction.c
[5] http://stackoverflow.com/questions/21679063/return-value-of-fgets
[6] https://www.tutorialspoint.com/c_standard_library/c_function_getenv.htm
[7] http://en.cppreference.com/w/c/program/SIG_strategies
[8] http://man7.org/linux/man-pages/man2/dup.2.html
[9] https://linux.die.net/man/3/strdup
[10] https://www.google.com/webhp?sourceid=chrome-instant&ion=1&espv=2&ie=UTF-8#q=what%20does%20waitpid%20return
[11] http://pubs.opengroup.org/onlinepubs/9699919799/functions/wait.html
*/
