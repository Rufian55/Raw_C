/******************************************************************************
* server.c is a simple server in the internet domain using TCP.
* Pass the port number as an argument from the command line in the range 2000
* to 65535. For more info, see http://www.linuxhowtos.org/C_C++/socket.htm
* Compile with: gcc server.c -o server -g -Wall
* Usage:  server { 2000 to 65535 inclusive } &
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>	// Defines socket structs.
#include <netinet/in.h>	// Contains constants and structs needed for internet domain addresses.

// Prints error message *msg and exits.
void error(const char *msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[]) {
	int socketFD;					// A socket file descriptor for the socket() call.
	int newSocketFD;				// Another socket file descriptor but for accept() call.
	int portNum;					// The communications port that accepts connections (16 bits).
	socklen_t clientAddressLength;			// Unsigned int type of length of at least 32
							//   bits used to evaluate the sizeof clientAddress.
	char buffer[256];				// Message buffer that stores chars read from the socket connection.
	struct sockaddr_in serverAddress;		// Struct containing the internet address of the server as defined in netinet/in.h.
	struct sockaddr_in clientAddress;		// Struct containing the internet address of the client that connects. 

/*   The below struct is defined for you in the netinet/in.h library:
	struct sockaddr_in {
		short sin_family;			// Must be AF_INET.
		u_short sin_port;  
		struct in_addr sin_addr;		// in_addr struct contains only one field: unsigned long s_addr.
		char sin_zero[8];			// Not used, must be zero.
	};
*/
	int n;  					// Holder for return values from read() and write() calls.

	// Check usage syntax - run in background!.
	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided.\nUsage is \"server {port number} &\"\n");
		exit(1);
	}

	/* Create an endpoint for communications (a socket) and return a File Descriptor reference.
	   Syntax: int socket(int domain, int type, int protocol);
	   AF_INET => Adress Family IPv4 Internet protocols and as opposed to AF_UNIX.
	   SOCK_STREAM => Provides sequenced, reliable, two way, connection based byte streams. SOCK_DGRAM is chunked datagrams.
	   0 => Operating System chooses appropriate protocol.  TCP for stream sockets and UDP for datagram sockets.
	   On success, a file descriptor for the new socket is returned.
	   On error, -1 is returned, and errno is set appropriately.
	   See http://www.linuxhowtos.org/manpages/2/socket.htm */
	socketFD = socket(AF_INET, SOCK_STREAM, 0);

	// Check socketFD was instantiated without errors.
	if (socketFD < 0) {
		error("ERROR opening socket ");
	}

	// Sets all characters of serverAddress var to '\0' (similar to memset). Note the cast.
	bzero( (char *)&serverAddress, sizeof(serverAddress) );

	// PortNum assigned command line argument after string of digits to int conversion.
	portNum = atoi(argv[1]);

	// Initialize the serverAddress sin_ struct members.
	serverAddress.sin_family = AF_INET;		// Address Family = Internet IPv4.
	serverAddress.sin_port = htons(portNum);	// Converts portNum in host byte order to portNum in network byte order (Big/Little Endian).
	serverAddress.sin_addr.s_addr = INADDR_ANY;	// Set the IP address of the host. INADDR_ANY returns host machines returns this address.

	/* Binds socketFD to the address of the current host and port number on which the server will run. 
		arg_1 => the socket file descriptor.
		arg_2 => pointer to the servers Address (note cast).
		arg_3 => the sizeof the address that is to be bound. 
		returns -1 on error condition.					*/
	if (bind(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
		error("ERROR on binding ");
	}

	/* Listen on the socket with a backlog queue size max of 5 (most systems) backlog queue - the number of connections
	   that can be waiting while the process is handling a particular connection. */
	listen(socketFD, 5);

	// Get the sizeof clientAddress.
	clientAddressLength = sizeof(clientAddress);

	/* The accept() system call causes the process to block until a client connects to the server.
	   Thus, it wakes up the process when a connection from a client has been successfully established.
	   Returns a new file descriptor, and all communication on this connection is then done using the new file descriptor.
	   The second argument is a reference pointer to the address of the client on the other end of the connection.
	   Third argument is the size of the clientAddress struct. */
	newSocketFD = accept(socketFD, (struct sockaddr *)&clientAddress, &clientAddressLength);

	if (newSocketFD < 0) {
		error("ERROR on call to accept() ");
	}

	/* You have a connection at this point... */

	// Initialize the buffer to all '\0'.
	bzero(buffer, 256);

	// read() the "newSocketFD" into the buffer for a max of 255 chars, leaving the last null terminator.
	n = read(newSocketFD, buffer, 255);

	// Error check - read returns -1 on error condition.
	if (n < 0) {
		error("ERROR reading from socket ");
	}

	printf("Here is the message server received: %s\n", buffer);

	// Send a message to the client machine by writing to newSocketFD. 
	n = write(newSocketFD, "Server got your message.", 24);

	// Error check call to write().
	if (n < 0) {
		error("ERROR writing to socket ");
	}

	// Close the connections.
	close(newSocketFD);
	close(socketFD);
	return 0; 
}
