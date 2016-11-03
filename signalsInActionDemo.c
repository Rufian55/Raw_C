/*****************************************************************************************
* catchSignals.c demonstrates how to catch and process signals using the signal.h library.
* Compile with "gcc catchSignals.c -o sigCatch -g -Wall".
* [1]
* The sigaction struct architecture:
*	struct sigaction {
*		void (*sa_handler)(int);
*		sigset_t sa_mask;
*		int sa_flags;
*		void (*sa_sigaction)(int, siginfo_t*, void*);
*	};
* [2]
* For "bulk" handling a set of signals.
* To define a signal set:
	sigset_t my_signal_set;
* To initialize or reset the signal set to have nosignal types:
	sigemptyset(&my_signal_set);
* To initialize or reset the signal set to have all signal types:
	sigfillset(&my_signal_set);
* To add a single signal type to the set:
	sigaddset(&my_signal_set, signal);
* To remove a single signal type from the set:
	sigdelset(&my_signal_set, signal);
* [3]
* int sigaction(int signo, struct sigaction *newact, struct *oldact);
*	"signo" is the signal to be acted upon, i.e. SIGINT, SIGHUP, etc.
*	"*newact" is ptr to data-filled sigaction struct that describes the action to be taken
		upon reciept of the signal specified in the first parameter.
*	"*oldact" is ptr to to another sigaction struct to which the sigaction() function will
		write out what the hanling settings for this signal were before this change.
******************************************************************************************/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// Prototypes.
void catchSIGINT();
void catchSIGUSR2();

int main() {
	//Declare 3 sigaction structs.
	struct sigaction SIGINT_action, SIGUSR2_action, ignore_action;

	/* Set SIGINT_action handler to catchSIGINT().
	   Could have been SIG_DFL (default) or SIG_IGN (ignore). */
	SIGINT_action.sa_handler = catchSIGINT;

	/* Places all, by way of  other signals recieved while sa_handler is being executed. */
	sigfillset(&SIGINT_action.sa_mask);

	/* Set sa_flags to none. Could be SA_RESTHAND (Resets signal handler to SIG_DFL after
	   the first signal is received and handled) or SA_SIGINFO (Indicates to the OS that 
	   the 4th attribute of the sigaction struct is to be used. */
	SIGINT_action.sa_flags = 0;

	SIGUSR2_action.sa_handler = catchSIGUSR2;
	sigfillset(&SIGUSR2_action.sa_mask);
	SIGUSR2_action.sa_flags = 0;

	/* Register SIGINT & SIGUSR2 signals to their respective structs for potential handling. */
	sigaction(SIGINT, &SIGINT_action, NULL);
	sigaction(SIGUSR2, &SIGUSR2_action, NULL);


	// Register SIGTERM, SIGHUP, & SIGQUIT to be ignored. See [2] above.
	ignore_action.sa_handler = SIG_IGN;

	sigaction(SIGTERM, &ignore_action, NULL);
	sigaction(SIGHUP, &ignore_action, NULL);
	sigaction(SIGQUIT, &ignore_action, NULL);
	printf("SIGTERM, SIGHUP, and SIGQUIT are disabled.\n");

	printf("Send a SIGUSR2 signal to kill this program with \"kill(pid, SIGUSR2)\".\n");
	printf("Here is the pid you'll need: %d\n", getpid());
	printf("Send a SIGINT signal <Ctrl-c> to sleep 5 seconds, (SIGINT handler function's last line is \"exit(0)\".\n");

	while (true) {// Infinite loop.
		pause(); // Sleep, but wake up on signal receipt.
	}

	return 0;
}


/* Handler function for catching SIGINT signal.
   Writes message, raises SIGUSR2 signal, and sleeps(5)
   when called. */
void catchSIGINT() {
	char* message = "\nCaught SIGINT, raising SIGUSR2 signal, sleeping for 5 seconds...\n";
	// For write() syscall - STDIN_FILENO = 0, STDOUT_FILENO = 1, STDERR_FILENO = 2
	write(STDOUT_FILENO, message, 66);		// Can't use printf or strlen(message) as both are non-rentrant.
	raise(SIGUSR2);					// Signal SIGUSR2 is generated.
	sleep(5);
}


/* Handler function for catching SIGUSR2 signal.
Writes message and exits when called. */
void catchSIGUSR2() {
	char* message = "Caught SIGUSR2, exiting!\n";
	write(STDOUT_FILENO, message, 25);
	exit(0);
}
