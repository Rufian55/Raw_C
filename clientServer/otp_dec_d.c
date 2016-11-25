/******************************************************************************
* Chris Kearns, CS344-400-F16, Proejct 4, kearnsc@oregonstate.edu 2 Dec 2016
*
* otp_dec_d stands for "One Time Pad Decryption Daemon".
*
* otp_dec_d.c is a simple single purpose server that listens on the user
* inputted port/socket that, once connected with the otp_dec client (only!),
* accepts a key (see also keygen.c) and a ciphertext message. These are used to
* generate and write back a decrypted plaintext to otp_dec connected process.
*
* This is a background "daemon like" program. Kill it with "kill -9 <pid>".
* Usage is: otp_dec_d {int listening_port_number} &
*    where int listening_port_number is an int from 2000 to 65535 inclusive.
* Compile with the provided "compileall" bash script or individually:
* gcc otp_dec_d.c -o otp_dec_d -g -Wall	   Socket code adapated from [1][2][3]
******************************************************************************/
#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT_LOW 2000
#define PORT_HIGH 65535
#define TEST 0

// Prototypes.
void usage();

int main(int argc, char **argv) {
	int i;
	long err;							// General purpose error testing var.
	char *msgBuffer;					// Pointer to incoming ciphertext messages storage buffer.
	size_t lengthOfMsg = 0;				// Calculated length of ciphertext message.
	char *keyBuffer;					// Pointer to incoming key storage buffer.
	size_t lengthOfKey = 0;				// Calculated length of the key.
	char *plaintextResponse;				// Pointer to the plaintext response buffer prior to sending to otp_dec client.
	int socketFD;						// A socket file descriptor for the socket() call.
	int newSocketFD;					// Another socket file descriptor but for upto 5x accept() call.
	int portNum;						// The communications port that accepts connections (16 bits).
	socklen_t clientAddressLength;		// (Unsigned int) type, at least 32 bits, used to evaluate sizeof clientAddress.
	pid_t pid;						// Process ID.
	char synBuffer[6];					// Storage buffer for client password read.
	char password[6] = "5m)C6";			// Server password.
	char ack[6] = "u9G@_";				// Returned to client on succesful password / synBuffer comparison.
	struct sockaddr_in serverAddress;		// Struct containing the internet address of the server as defined in netinet/in.h.
	struct sockaddr_in clientAddress;		// Struct containing the internet address of the client that connects.

/*   The below struct is defined for you in the netinet/in.h library and noted here for "future me" purposes only.
	struct sockaddr_in {
	short sin_family;					// Must be AF_INET.
	u_short sin_port;					// Port number in network byte order.  See below.
	struct in_addr sin_addr;				// in_addr struct contains only one field: unsigned long s_addr.
	char sin_zero[8];					// Not used, must be zero.
	};
*/

	// Check arg count - must be 2.
	if (argc != 2) {
		usage();
		exit(EXIT_FAILURE);
	}

	// PortNum assigned command line argument after string of digits to int conversion.
	portNum = atoi(argv[1]);
	// Check for proper range (arbitrarily set by author).
	if (portNum < PORT_LOW || portNum > PORT_HIGH) {
		usage();
		exit(EXIT_FAILURE);
	}

	/* Create an endpoint for communications (a socket) and return a File Descriptor reference.
	   Syntax: int socket(int domain, int type, int protocol);
	   AF_INET => Adress Family IPv4 Internet protocols and as opposed to AF_UNIX.
	   SOCK_STREAM => Provides sequenced, reliable, two way, connection based byte streams. SOCK_DGRAM is chunked datagrams.
	   0 => Operating System chooses appropriate protocol.  TCP for stream sockets and UDP for datagram sockets.
	   On success, a file descriptor for the new socket is returned.
	   On error, -1 is returned, and errno is set appropriately.  [2] */
	socketFD = socket(AF_INET, SOCK_STREAM, 0);

	// Check socketFD was instantiated without errors.
	if (socketFD < 0) {
		fprintf(stderr, "otp_dec_d Error_1: opening socket connection. Please try again.");
		exit(EXIT_FAILURE);
	}

	// Sets all characters of serverAddress var to '\0' (similar to memset). Note the cast.
	bzero((char *)&serverAddress, sizeof(serverAddress));

	// Initialize the serverAddress sin_ struct members.
	serverAddress.sin_family = AF_INET;		// Address Family = Internet IPv4.
	serverAddress.sin_addr.s_addr = INADDR_ANY;	// Set the IP address of the host. INADDR_ANY returns host machines (this) address.
	serverAddress.sin_port = htons(portNum);	// Converts portNum in host byte order to portNum in network byte order (Big/Little Endian).

	/* Binds socketFD to the address of the current host and port number on which the server will run.
		arg_1 => the socket file descriptor.
		arg_2 => pointer to the servers Address (note cast).
		arg_3 => the sizeof the address that is to be bound.
		returns -1 on error condition. */
	if (bind(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
		fprintf(stderr, "otp_dec_d Error_2: call to bind() failed! Please try again.\n");
		exit(EXIT_FAILURE);
	}

	/* Listen on the socket with a maximum size 5 (most systems) backlog queue - the number of connections
	   that can be waiting while the process is handling a particular connection. */
	if (listen(socketFD, 5) == -1) {
		fprintf(stderr, "otp_dec_d Error_3: otp_dec_d busy, please try again.\n");
		exit(EXIT_FAILURE);
	}

	// Get the sizeof clientAddress.
	clientAddressLength = sizeof(clientAddress);

	while (true) {
		/* The accept() system call causes the process to block until a client connects to the server.
		Thus, it wakes up the process when a connection from a client has been successfully established.
		Returns a new file descriptor, and all communication on this connection is then done using the new file descriptor.
		The second argument is a reference pointer to the address of the client on the other end of the connection.
		Third argument is the size of the clientAddress struct. */
		newSocketFD = accept(socketFD, (struct sockaddr *)&clientAddress, &clientAddressLength);

		// Error check for call to accept().
		if (newSocketFD < 0) {
			fprintf(stderr, "otp_dec_d Error_4: call to accept() connection with client failed!\n");
			exit(EXIT_FAILURE);
		}

		/* A CONNECTION HAS BEEN ESTABLISHED AT THIS POINT... */

		// Fork the parent process.
		pid = fork();

		// Error check the call to fork().
		if (pid < 0) {
			fprintf(stderr, "otp_dec_d Error_5: call to fork() failed!\n");
			exit(EXIT_FAILURE);
		}

		/* We have a child process on a good port, so begin communicating w/ error checking routines. */
		if (pid == 0) {

			// Initialize synBuffer c-string.
			memset(synBuffer, '\0', 6);

			// Read handshake SYN value.
			err = read(newSocketFD, synBuffer, 5);

			// Test err for good read!
			if (err != 5) {
				fprintf(stderr, "otp_dec_d Error_6: Initial handshake failed!\n");
			}

			if (TEST) {
				printf("synBuffer = %s\n", synBuffer);
				printf("ack = %s\n", ack);
				printf("password = %s\n", password);
			}

			// Test for correct password from client - bounces back client supplied password if no match.
			if (strncmp(synBuffer, password, 5) == 0) {
				err = write(newSocketFD, ack, 5);
				if (err != 5) {
					fprintf(stderr, "otp_dec_d Error_7: call to write() ack failed!\n");
				}
			}
			else {
				err = write(newSocketFD, password, 5);
				if (err != 5) {
					fprintf(stderr, "otp_dec_d Error_8: call to write() (bounce back) password failed!\n");
				}
				/* Since an incorrect password has been supplied, skip everything except closing the newSocketFD
				   at the bottom of this loop.  See the **************  ect... commented pwFail: label below. */
				goto pwFail;
			}

			// Read in the lengthOfMsg from otp_dec.
			err = read(newSocketFD, &lengthOfMsg, sizeof(lengthOfMsg));

			if (TEST) {
				printf("otp_dec_d err for read convertedLOM = %ld\n", err);
			}

			// Error test call to read().
			if (err < 0) {
				fprintf(stderr, "otp_dec_d Error_9: call to read lengthOfMsg from otp_dec failed!\n");
				exit(EXIT_FAILURE);
			}

			// Convert to network Endianess.		[7]
			lengthOfMsg = ntohl(lengthOfMsg);


			// Read in the lengthOfKey from otp_dec.
			err = read(newSocketFD, &lengthOfKey, sizeof(lengthOfKey));

			if (TEST) {
				printf("otp_dec_d err for read convertedLOK = %ld\n", err);
			}

			// Error test call to read().
			if (err < 0) {
				fprintf(stderr, "otp_dec_d Error_10: call to read lengthOfKey from otp_dec failed!\n");
				exit(EXIT_FAILURE);
			}

			// Convert to network Endianess.		[7]
			lengthOfKey = ntohl(lengthOfKey);

			if (TEST) {
				printf("lengthOfMsg = %zu\n", lengthOfMsg);
				printf("lengthOfKey = %zu\n", lengthOfKey);
			}

			// Error check lengthOfKey for < than lengthOfMsg.
			if (lengthOfKey < lengthOfMsg) {
				fprintf(stderr, "otp_dec_d Error_11: while reading key\nKey must be at least as long as plaintext message!\n");
				exit(EXIT_FAILURE);
			}

			/* GET THE CIPHERTEXT MESSAGE */

			// Allocate memory for msgBuffer.
			msgBuffer = malloc(lengthOfMsg * sizeof(char));

			// Error check call to malloc().
			if (msgBuffer == NULL) {
				fprintf(stderr, "otp_dec_d Error_11: call to malloc() failed!\n");
				exit(EXIT_FAILURE);
			}

			// Get the ciphertext message from otp_dec.
			err = read(newSocketFD, msgBuffer, lengthOfMsg);

			// Error check that ciphertext message was larger than 0.
			if (err < 0) {
				fprintf(stderr, "otp_dec_d Errror_12: No ciphertext message accompanied your request.\n");
				exit(EXIT_FAILURE);
			}

			if (TEST) {
				fprintf(stdout, "otp_dec_d says ciphertext message received = %s\n", msgBuffer);
				fprintf(stdout, "otp_dec_d says ciphertext string length = %zu\n", lengthOfMsg);
			}

			/* Check for valid ciphertext message received character set has been used.
			   Note cast to long for each element of msgBuffer to avoid the gcc compiler warning. */
			for (i = 0; i < lengthOfMsg; i++) {
				// If the char is < A and not also a space or the char is > Z... See [4].
				if (((long)msgBuffer[i] < 65 && (long)msgBuffer[i] != 32) || (long)msgBuffer[i] > 90) {
					fprintf(stderr, "otp_dec_d Error_13: plaintext message contains bad characters! A-Z and \" \" only!\n");
					exit(EXIT_FAILURE);
				}
			}

			/* GET THE KEY */

			// Allocate memory for the key storage buffer.
			keyBuffer = malloc(lengthOfKey * sizeof(char));

			// Check for good memory allocation.
			if (keyBuffer == NULL) {
				fprintf(stderr, "otp_dec_d Error_14: call to malloc() failed!\n");
				exit(EXIT_FAILURE);
			}

			// Read the key.
			err = read(newSocketFD, keyBuffer, lengthOfKey);

			// Error test call to read().
			if (err < 0) {
				fprintf(stderr, "otp_dec_d Error_15: call to read() lengthOfKey from otp_dec failed!\n");
				exit(EXIT_FAILURE);
			}

			if (TEST) {
				fprintf(stdout, "otp_dec_d says keyString = %s\n", keyBuffer);
			}

			/* So we should now have a valid ciphertext message and a key for decrypting.
			   We now do the actual decryption process of the ciphertext message and return it. */

			// Allocate memeory for the encyrptedResponse buffer.
			plaintextResponse = malloc(lengthOfMsg * sizeof(char));

			if (plaintextResponse == NULL) {
				fprintf(stderr, "otp_dec_d Error_16: call to malloc() failed!\n");
				exit(EXIT_FAILURE);
			}

			// Decrypt the ciphertext message.		See citation [5]
			for (i = 0; i < lengthOfMsg; i++) {
				/* Convert spaces to the 'at' symbol in both key and ciphertext
				message otherwise our "% 27" call won't work. Could also use '['.  */
				if (msgBuffer[i] == ' ') {
					msgBuffer[i] = '@';
				}
				if (keyBuffer[i] == ' ') {
					keyBuffer[i] = '@';
				}

				// Cast chars to ints.
				int tempMessageChar = (int)msgBuffer[i];
				int tempKeyChar = (int)keyBuffer[i];

				// Offset ASCII values by 64 so that range is 0 - 26 for 27 total chars.
				tempMessageChar -= 64;
				tempKeyChar -= 64;

				// Subtract corresponding key chars from message chars and store the result.
				int eachPlaintextChar = (tempMessageChar - tempKeyChar);

				// Convert negatives so range maintained as 0 - 26 for 27 total chars.
				if (eachPlaintextChar < 0) {
					eachPlaintextChar += 27;
				}

				// Add 64 back so that the ASCII range is the correct 64 - 90, less the ' ' issue.
				eachPlaintextChar += 64;

				// Cast each "encrypted" int back to char and add to the encryptedResponse buffer.
				plaintextResponse[i] = (char)eachPlaintextChar;

				// Change the 'at' symbols to spaces.
				if (plaintextResponse[i] == '@') {
					plaintextResponse[i] = ' ';
				}
			}

			// Send the encyrpted ciphertext to otp_enc client.
			err = write(newSocketFD, plaintextResponse, lengthOfMsg);

			// Error check call to write().
			if (err < 0) {
				fprintf(stderr, "otp_dec_d Error17: Call to write() to newSocketFD failed!");
				exit(EXIT_FAILURE);
			}

			if (TEST) {
				fprintf(stdout, "otp_dec_d says plaintext response = %s\n", plaintextResponse);
			}

			// Manage memory on every iteration of the loop, skipped on password failure.
			free(msgBuffer);
			msgBuffer = NULL;
			free(keyBuffer);
			keyBuffer = NULL;
			free(plaintextResponse);
			plaintextResponse = NULL;

/************  Jump to here because password sent by otp_dec was incorrect (most
			likely because otp_enc rather than otp_dec attempted to connect). */
			pwFail:

			// Close sockets.
			close(newSocketFD);

			// Exit the child process!
			exit(0);
		}
		else {
			close(newSocketFD);
		}

		// Avoid zombied child process after otp_enc call.
		waitpid(pid, NULL, 0);

	}// End while.

	 // Close the listener socket, though we never get here...
	close(socketFD);
	return 0;
}

// A usage error message printed to stdout.	[6]
void usage() {
	fprintf(stderr, "otp_dec_d Error_18: syntax: otp_dec_d {listening_port_number = 2000 - 65535 inclusive} &\n"
		"Note this is a background \"daemon\" process.\n");
}


/* CITATIONS: Adapted from the following sources:
[1] Lecture slides and CS344-400-F16 forum commentary, Prof. B. Brewster, Oregon State Unviversity.
[2] http://www.linuxhowtos.org/C_C++/socket.htm
[3] http://www.linuxhowtos.org/manpages/2/socket.htm
[4] http://www.asciitable.com/
[5] https://en.wikipedia.org/wiki/One-time_pad
[6] http://stackoverflow.com/questions/12102332/when-should-i-use-perror-and-fprintfstderr
[7] https://linux.die.net/man/3/ntohl
*/