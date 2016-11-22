/******************************************************************************
* Chris Kearns, CS344-400-F16, Proejct 4, kearnsc@oregonstate.edu 2 Dec 2016
*
* otp_enc_d stands for "One Time Pad Encryption Daemon".
*
* otp_enc_d.c is a simple single purpose server that listens on the user
* inputted port/socket that, once connected with the otp_enc client (only!),
* accepts a key (see also keygen.c) and a plaintext message. These are used to 
* generate and write back an encrypted ciphertext to otp_enc connected process. 
*
* This is a background "daemon like" program. Kill it with "kill -9 <pid>".
* Usage is: otp_enc_d {int listening_port_number} &
*    where int listenting_port_number is an int from 2000 to 65535 inclusive.
* Compile with the provided "compileall" bash script or individually:
* gcc otp_enc_d.c -o otp_enc_d -g -Wall	   Socket code adapated from [1][2][3]
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
#define SIZE_OF_BUFFER 100000
#define TEST 0

// Prototypes.
void usage();
void dostuff(int);
void error(const char *);

int main(int argc, char **argv) {
	int i;
	char inBuffer[SIZE_OF_BUFFER];		// Incoming plaintext message storage buffer.
	int lengthOFplaintext;				// Calculated length of plaintext message.
	int numCharsSent;					// Number of chars sent with calls to write().
	char keyBuffer[SIZE_OF_BUFFER];		// A buffer to store the incoming key.
	int lengthOFkey;					// Calculated length of the key.
	char encyrptedResponse[SIZE_OF_BUFFER];	// A buffer to store the encrypted ciphertext to be sent to otp_enc client.
	int socketFD;						// A socket file descriptor for the socket() call.
	int newSocketFD;					// Another socket file descriptor but for upto 5x accept() call.
	int portNum;						// The communications port that accepts connections (16 bits).
	socklen_t clientAddressLength;		// Unsigned int type of length of at least 32
									//   bits used to evaluate the sizeof clientAddress.
	pid_t pid;						// Process ID.
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
		fprintf(stderr, "otp_enc_d ERROR opening socket connection.\nPlease try again.");
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
		fprintf(stderr, "otp_enc_d ERROR on call to bind() ");
		exit(EXIT_FAILURE);
	}

	/* Listen on the socket with a maximum size 5 (most systems) backlog queue - the number of connections
	that can be waiting while the process is handling a particular connection. */
	if(listen(socketFD, 5) == -1) {
		fprintf(stderr, "otp_enc_d Error: otp_enc_d busy, please try again.");
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
			fprintf(stderr, "otp_enc_d ERROR on call to accept() connection with client.");
			exit(EXIT_FAILURE);
		}

		/* A connection has been established at this point... */

		// Fork the parent process.
		pid = fork();
		// Error check the call to fork().
		if (pid < 0) {
			fprintf(stderr, "otp_enc_d ERROR on call to fork() ");
			exit(EXIT_FAILURE);
		}

		if (pid == 0) {
			/* We have a child process on a good port, so we begin communicating with error checking routines. */

			// Set all elements of inBuffer to 0.
			memset(inBuffer, 0, SIZE_OF_BUFFER);

			// Get the plaintext message from otp_enc.
			lengthOFplaintext = read(newSocketFD, inBuffer, SIZE_OF_BUFFER);

			// Error check that plaintext message was larger than 0.
			if (lengthOFplaintext < 1) {
				fprintf(stderr, "otp_enc_d Errror: No plaintext message accompanied your request.");
				exit(EXIT_FAILURE);
			}

			if (TEST) {
				fprintf(stdout, "otp_enc_d (1) says plaintext message received = %s\n", inBuffer);
				fprintf(stdout, "otp_enc_d (1) says plaintext string length = %d\n", lengthOFplaintext);
			}

			/* Check for valid plaintextmessage received character set has been used. 
			   Note cast to long for each element of inBuffer to avoid the gcc compiler warning. */
			for(i = 0; i < lengthOFplaintext - 1; i++) {
				// If the char is < A and not also a space or the char is > Z... See [4].
				if( ((long)inBuffer[i] < 65 && (long)inBuffer[i] != 32) || (long)inBuffer[i] > 90 ) {
					fprintf(stderr, "otp_enc_d Error: plaintext message contains bad characters! A-Z and \" \" only!\n");
					exit(EXIT_FAILURE);
				}
			}

			if (TEST) {
				fprintf(stdout, "otp_enc_d (2) says plaintext message received = %s\n", inBuffer);
			}


			// Return an acknowledgement to client that the plaintext message was received.
			numCharsSent = write(newSocketFD, "200", 3);

			//Error check return message sent.
			if (numCharsSent != 3) {
				fprintf(stderr, "otp_enc_d Error: sending plaintext message acknowledgement back to client failed!");
				exit(EXIT_FAILURE);
			}

			// Set all elements of keyBuffer to 0.
			memset(keyBuffer, 0, SIZE_OF_BUFFER);

			// Read the key.
			lengthOFkey = read(newSocketFD, keyBuffer, SIZE_OF_BUFFER);

			// Error check lengthOFkey for < than the plaintextmessage or == 0 (no key sent).
			if (lengthOFkey < lengthOFplaintext || lengthOFkey < 1) {
				fprintf(stderr, "otp_enc_d Error: while reading key\nKey must be at least as long as plaintext message or no key sent!");
				exit(EXIT_FAILURE);
			}

			/* Check for valid plaintextmessage received character set has been used.
			   Note cast to long for each element of inBuffer to avoid the gcc compiler warning. */
			for (i = 0; i < lengthOFkey; i++) {
				if( ((long)keyBuffer[i] < 65 && (long)keyBuffer[i] != 32) || (long)keyBuffer[i] > 90) {
					fprintf(stderr, "otp_enc_d Error: key contains bad characters! A-Z and \" \" only!\n");
					exit(EXIT_FAILURE);
				}
			}

			if (TEST) {
				fprintf(stdout, "otp_enc_d says keyString = %s\n", keyBuffer);
			}

			/* So we should now have a valid plaintext message and a key for encrypting.
			   We now do the actual encryption process of the plaintext message and return it. */

			// Set all elements of encyrptedResponse buffer to 0.
			memset(encyrptedResponse, 0, SIZE_OF_BUFFER);

			// Encrypt the plaintext message.
			for (i = 0; i < lengthOFplaintext - 1; i++) {
				/* Convert spaces to the 'at' symbol in both key and plaintext
				   message otherwise our "% 27" call won't work. Could also use '['.  */
				if (inBuffer[i] == ' ') {
					inBuffer[i] = '@';
				}
				if (keyBuffer[i] == ' ') {
					keyBuffer[i] = '@';
				}

				// Cast chars to ints.
				int tempMessageChar = (int)inBuffer[i];
				int tempKeyChar = (int)keyBuffer[i];

				// Offset ASCII values by 64 so that range is 0 - 26 for 27 total chars.
				tempMessageChar -= 64;
				tempKeyChar -= 64;

				// Add corresponding key chars and message chars and modulo the result. [5]
				int eachCipherChar = (tempMessageChar + tempKeyChar) % 27;

				// Add 64 back so that the ASCII range is the correct 64 - 90, less the ' ' issue.
				eachCipherChar += 64;

				// Cast each "encrypted" int back to chars into the encryptedResponse buffer.
				encyrptedResponse[i] = (char)eachCipherChar;

				// Change the 'at' symbols to spaces.
				if (encyrptedResponse[i] == '@') {
					encyrptedResponse[i] = ' ';
				}
			}

			// Send the encyrpted ciphertext to otp_enc client.
			numCharsSent = write(newSocketFD, encyrptedResponse, lengthOFplaintext);

			// Error check call to write().
			if (numCharsSent < lengthOFplaintext) {
				fprintf(stderr, "otp_enc_d Error: Call to write() to newSocketFD failed!");
				exit(EXIT_FAILURE);
			}

			if (TEST) {
				fprintf(stdout, "otp_end_d says encrypted response = %s\n", encyrptedResponse);
			}

			// Close sockets.
			close(newSocketFD);
			close(socketFD);
			// Exit the child process!
			exit(0);
		}
		else {
			close(newSocketFD);
		}

		// Avoid zombied process after otp_enc call.
		waitpid(pid, NULL, 0);

	}// End while.

	// Close the listener socket, though we never get here...
	close(socketFD);

	return 0;
}

// A usage error message printed to stdout.	[6]
void usage() {
	fprintf(stderr, "otp_enc_d Error: syntax: otp_enc_d {listening_port_number = 2000 - 65535 inclusive} &\n"
				 "Note this is a background \"daemon\" process.\n");
}


/* CITATIONS: Adapted from the following sources:
[1] Lecture slides and CS344-400-F16 forum commentary, Prof. B. Brewster, Oregon State Unviversity.
[2] http://www.linuxhowtos.org/C_C++/socket.htm
[3] http://www.linuxhowtos.org/manpages/2/socket.htm
[4] http://www.asciitable.com/
[5] https://en.wikipedia.org/wiki/One-time_pad
[6] http://stackoverflow.com/questions/12102332/when-should-i-use-perror-and-fprintfstderr
*/