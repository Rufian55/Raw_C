/******************************************************************************
* client.c is a simple client in the internet domain using TCP.
* Pass the port number as an argument from the command line in the range 2000
* to 65535. For more info, see http://www.linuxhowtos.org/C_C++/socket.htm
* Compile with: gcc client.c -o cleint -g -Wall
* Usage:  client { host: i.e. "localhost" } { 2000 to 65535 inclusive }
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>	// Defines socket structs.
#include <netinet/in.h>	// Contains constants and structs needed for internet domain addresses.
#include <netdb.h>		// Defines struct hostent.


// Prints error message *msg and exits.
void error(const char *msg) {
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[]) {
	char buffer[256];						// Message buffer that stores chars read from the socket connection.
	int socketFD;							// A socket file descriptor for the socket() call.
	int portNum;							// The communications port that accepts connections (16 bits).
	int n;  								// Holder for return values from read() and write() calls.
	struct sockaddr_in serverAddress;			// Struct containing the internet address of the server as defined in netinet/in.h.
	struct hostent *server;					// Pointer to struct type hostent as defined in the header file netdb.h
/*	The struct hosetent definition:
	struct  hostent {
		char    *h_name;					// Official name of host.
		char    **h_aliases;				// Alias list - zero  terminated  array  of  alternate names for the host.
		int     h_addrtype;					// Host address type returned currently always AF_INET.
		int     h_length;					// Length of address in bytes.
		char    **h_addr_list;				// A pointer to a list of network addresses for the named host. Host addresses are
											returned in network byte order.
		#define h_addr  h_addr_list[0]		// h_addr is, for backward compatiblity, an alias for the first address in array of
											network addresses.
	};
*/

	// Error check for proper command line syntax.
	if(argc < 3) {
		fprintf(stderr,"Usage: %s hostname portNumber\n", argv[0]);
		exit(0);
	}

	// PortNum assigned command line argument after string of digits to int conversion.
	portNum = atoi(argv[2]);

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
		error("ERROR opening socket");
	}

	/* Argv[1] contains the name of the host on the Internet. A pointer to a hostent struct that 
	contains needed info on that hose. The field char *h_addr contains the IP address. */
	server = gethostbyname(argv[1]);

	// Error check call to gethostbyname(). NULL means system could not locate host.
	if(server == NULL) {
		fprintf(stderr,"ERROR, host not found.\n");
		exit(0);
	}

	// Sets all characters of serverAddress var to '\0' (similar to memset). Note the cast.
	bzero((char *)&serverAddress, sizeof(serverAddress));

	// Initialize the serverAddress sin_ struct.
	serverAddress.sin_family = AF_INET;		// Address Family = Internet IPv4.

	// Copies n bytes from src to dest [ void bcopy(void *src, void *dest, size_t n); ] note casts.
	bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);

	// Converts portNum in host byte order to portNum in network byte order (Big/Little Endian).
	serverAddress.sin_port = htons(portNum);

	/* The connect function is called by the client to establish a connection to the server.
	   arg_1 => socket file descriptor
	   arg_2 => the address of the host to which it wants to connect(including the port number),
	   arg_3 => the size of this address.
	   returns 0 on success, -1 ion failure.
	   Note that the client needs to know the port number of the server, but it does not need 
	   to know its own port number. This is typically assigned by the system when connect is called. */
	if(connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
		error("ERROR connecting");
	}

	// Message to client user.
	printf("Please enter the message: ");

	// Initialize the buffer to all '\0'.
	bzero(buffer, 256);

	// Read the message from the keyboard to 255 chars, leaving last one as null terminator.
	fgets(buffer, 255, stdin);

	// Send a message to the server machine by writing to socketFD.
	n = write(socketFD, buffer, strlen(buffer));

	// Error check call to write().
	if(n < 0) {
		error("ERROR writing to socket ");
	}

	// Re-initialize the buffer to all '\0'.
	bzero(buffer, 256);

	// Read the reply message from the server, 
	n = read(socketFD, buffer, 255);

	// Error check the call to read().
	if(n < 0) {
		error("ERROR reading from socket");
	}

	// Show me the message!!
	printf("%s\n", buffer);

	// Close socket connection.
	close(socketFD);
	return 0;
}
