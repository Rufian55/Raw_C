/******************************************************************************
* otp_enc_d.c is a simple single purpose server that listens on the user
* inputted port/socket that, once connected with the otp_enc client (only!),
* accepts a key (see also keygen.c) and a plaintext message. These are used to 
* generate and write back an encrypted ciphertext to otp_enc connected process. 
* This is a background "daemon" like program.
* Usage is: otp_enc_d {int listening_port_number} & where int 
* listenting_port_number is an int from 2000 to 65535 inclusive. Compile with 
* the "compileall" bash script or individually:
* gcc otp_enc_d.c -o otp_enc_d -g -Wall								[1]
******************************************************************************/
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <unistd.h>

#define PORT_LOW 2000
#define PORT_HIGH 65535

// Prototypes.
void usage();
void dostuff(int);
void error(const char *);

int main(int argc, char **argv) {
	int socketFD;						// A socket file descriptor for the socket() call.
	int newSocketFD;					// Another socket file descriptor but for accept() call.
	int portNum;						// The communications port that accepts connections (16 bits).
	socklen_t clientAddressLength;		// Unsigned int type of length of at least 32
									//   bits used to evaluate the sizeof clientAddress.
	pid_t pid;
	struct sockaddr_in serverAddress;		// Struct containing the internet address of the server as defined in netinet/in.h.
	struct sockaddr_in clientAddress;		// Struct containing the internet address of the client that connects.

/*   The below struct is defined for you in the netinet/in.h library and noted here for edification.
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
		exit(1);
	}

	// PortNum assigned command line argument after string of digits to int conversion.
	portNum = atoi(argv[1]);
	// Check for proper range (arbitraily set by author).
	if (portNum < PORT_LOW || portNum > PORT_HIGH) {
		usage();
		exit(1);
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
		error("ERROR opening socket");
	}

	// Sets all characters of serverAddress var to '\0' (similar to memset). Note the cast.
	bzero((char *)&serverAddress, sizeof(serverAddress));

	// Initialize the serverAddress sin_ struct members.
	serverAddress.sin_family = AF_INET;		// Address Family = Internet IPv4.
	serverAddress.sin_addr.s_addr = INADDR_ANY;	// Set the IP address of the host. INADDR_ANY returns host machines returns this address.
	serverAddress.sin_port = htons(portNum);	// Converts portNum in host byte order to portNum in network byte order (Big/Little Endian).

	/* Binds socketFD to the address of the current host and port number on which the server will run.
		arg_1 => the socket file descriptor.
		arg_2 => pointer to the servers Address (note cast).
		arg_3 => the sizeof the address that is to be bound.
		returns -1 on error condition. */
	if (bind(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
		error("ERROR on binding");
	}

	/* Listen on the socket with a backlog queue size max of 5 (most systems) backlog queue - the number of connections
	that can be waiting while the process is handling a particular connection. */
	listen(socketFD, 5);

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
			error("ERROR on accept");
		}

		/* A connection has been established at this point... */

		pid = fork();
		if (pid < 0)
			error("ERROR on call to fork() ");

		if (pid == 0) {
			close(socketFD);			// NOT SURE ABOUT THAT...
			dostuff(newSocketFD);
			exit(0);					// OR THAT...
		}
		else close(newSocketFD);
	}// End while.

	// Close the listener socket.
	close(socketFD);

	printf("Made it!\n");
	return 0;
}

void usage() {
	fprintf(stderr, "Error: syntax: otp_enc_d {listening_port_number = 2000 - 65535 inclusive} &\n"
				 "Note this is a background \"daemon\" process.\n");
}


/******** DOSTUFF() *********************
There is a separate instance of this function
for each connection.  It handles all communication
once a connnection has been established.
*****************************************/
void dostuff(int sock) {
	int n;								// Holder for return values from read() and write() calls.
	char buffer[256];						// Message buffer that stores chars read from the socket connection.
	bzero(buffer, 256);
	n = read(sock, buffer, 255);
	if (n < 0) {
		error("ERROR reading from socket");
	}

	printf("Here is the message: %s\n", buffer);

	// Send a message to the client machine by writing to newSocketFD. 
	n = write(sock, "I got your message", 18);

	// Error check call to write().
	if (n < 0) error("ERROR writing to socket");
}

// Error reporting.
void error(const char *msg) {
	perror(msg);
	exit(1);
}

/* CITATIONS: Adapted from the following sources:
[1] http://www.linuxhowtos.org/C_C++/socket.htm
[2] http://www.linuxhowtos.org/manpages/2/socket.htm
[3] Lecture slides and CS344-400-F16 forum commentary, Prof. B. Brewster, Oregon State Unviversity.
*/