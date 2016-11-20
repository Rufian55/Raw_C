/******************************************************************************
* Chris Kearns, CS344-400-F16, Proejct 4, kearnsc@oregonstate.edu 2 Dec 2016
* otp_enc stands for "One Time Pad Encryption (Client)".
* otp_enc.c is a simple single purpose client that requests and maintains 
* a connection to the otp_enc_d encryption daemon (only) on the inputted
* port/socket.  Once connected, otp_enc client sends a pregenerated key (see 
* also keygen.c) and a plaintext message for encyrption. The encyrpted message
* is written to a user specified file and output to the screen.
* This program will exit after succesful completion of encrytpted message so
* but can be called again immediately for additional work.
* Usage is: otp_enc {plaintext_message_file} {key_file} {port_number}
*  where port_number must match the port the otp_enc_d daemon was started on.
* Compile with the provided "compileall" bash script or individually:
* gcc otp_enc.c -o otp_enc -g -Wall	   Socket code adapated from [1][2][3]
******************************************************************************/
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 100000
#define TEST 1

int main(int argc, char** argv) {
	char inBuffer[BUFFER_SIZE];		// The plaintext message storage buffer and the returned encypted message.
	int lengthOFplaintext;			// Calculated length of plaintext message string
	char keyBuffer[BUFFER_SIZE];		// The key file storage buffer.
	int lengthOFkey;				// Calculated length of key file string.
	char amBuffer[3];				// The acknowledgement message (from otp_enc_d) storage buffer.
	int FD;						// General purpose File Descriptor.
	int i;
	int numCharsRecvd;				// The length of the filestream, in chars, received from otp_enc_d. 
	int numCharsSent;				// Number of chars sent through the socket connection.
	int portNum;					// The user defined communcations port number.
	int socketFD;					// The communications socket.
	struct sockaddr_in serverAddress;	// Struct containing the internet address of the server as defined in netinet/in.h.
	struct hostent *server;			// Pointer to struct type hostent as defined in the header file netdb.h

	// Error check for correct number of arguments.
	if (argc < 4) {
		printf("Usage Error: otp_enc {plaintext_message_file} {key_file} {port_number}\n");
		exit(1);
	}

	// Test for acceptable port number range.
	sscanf(argv[3], "%d", &portNum);
	if (portNum < 2000 || portNum > 65535) {
		printf("otp_enc Error: Invalid port number - range is 2000 - 65535 inclusive!\n");
		exit(1);
	}

	// Open plaintext message file for reading.
	FD = open(argv[1], O_RDONLY);

	// Error test call to open().
	if (FD < 0) {
		printf("otp_enc Error: Cannot open file %s or file is empty!\n", argv[1]);
		exit(1);
	}

	// Read contents of plaintext message file.
	lengthOFplaintext = read(FD, inBuffer, BUFFER_SIZE);

	// Validate contents of plaintext
	for (i = 0; i != 0 /*< plaintextLength - 1*/; i++) {
		if ((int)inBuffer[i] > 90 || ((int)inBuffer[i] < 65 && (int)inBuffer[i] != 32)) {
			printf("otp_enc Error: plaintext message string contains bad characters!\n");
			exit(1);
		}
	}

	// Close plaintext message file.
	close(FD);

	// Open key file for reading.
	FD = open(argv[2], O_RDONLY);

	// Error check call to read() of key file.
	if (FD < 0) {
		printf("otp_enc Error: Cannot open key file %s !\n", argv[2]);
		exit(1);
	}

	// Read contents of key file.
	lengthOFkey = read(FD, keyBuffer, BUFFER_SIZE);

	// Test for bad characters in key file.
	for (i = 0; i < lengthOFkey - 1; i++) {
		if ((int)keyBuffer[i] > 90 || ((int)keyBuffer[i] < 65 && (int)keyBuffer[i] != 32)) {
			printf("otp_enc Error: key file contains bad characters!\n");
			exit(1);
		}
	}

	// Close key file.
	close(FD);

	// Test length of strings contained in plaintext message file and key file.
	if (lengthOFkey < lengthOFplaintext) {
		printf("otp_enc Error: key file '%s' is too short!\n", argv[2]);
	}

	// Create communications socket for otp_enc.
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	// Error test call to socket().
	if (socketFD < 0) {
		printf("otp_enc Error: Call to socket() failed!\n");
		exit(1);
	}

	// Clear all elements of struct serv_addr_in serv_addr.
	memset(&serverAddress, '\0', sizeof(serverAddress));

	// Set struct hostent server to localhost.
	server = gethostbyname("localhost");

	// Test call to gethostbyname().
	if (server == NULL) {
		printf("otp_enc Error: Call to gethostname() failed!\n");
		exit(1);
	}

	// Initialize the serverAddress sin_ struct members.
	serverAddress.sin_family = AF_INET;		// Address Family = Internet IPv4.
	serverAddress.sin_port = htons(portNum);	/* Converts portNum in host byte order to portNum in
									     network byte order (Big/Little Endian). */

	// Copies n bytes from src to dest [ void bcopy(void *src, void *dest, size_t n); ] note casts.
	bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
	
	// Connect to otp_enc_d
	if (connect(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
		printf("otp_enc Error: Could not connect to otp_enc_d on port %d\n", portNum);
		exit(1);
	}

	// Send plaintext message to otp_enc_d daemon.
	numCharsSent = write(socketFD, inBuffer, lengthOFplaintext - 1);

	// Error check call to write().
	if (numCharsSent < lengthOFplaintext - 1) {
		printf("otp_enc Error: Could not send plaintext message to otp_enc_d on port %d.\n", portNum);
		exit(1);
	}

	// Set all elements of amBuffer to 0.
	memset(amBuffer, 0, 3);

	// Get acknowledgement message from server.
	numCharsRecvd = read(socketFD, amBuffer, 3);
	if (numCharsRecvd != 3) {
		printf("otp_enc Error: call to read() acknowledgement message from otp_enc_d failed!\n");
		exit(1);
	}

	if (TEST) {
		printf("otp_enc says numCharsRead into amBuffer = %d amBuffer = %s\n", numCharsRecvd, amBuffer);
	}

	// Send key file to otp_enc_d
	numCharsSent = write(socketFD, keyBuffer, lengthOFkey - 1);

	// Error check call to write().
	if (numCharsSent < lengthOFkey - 1) {
		printf("otp_enc Error: Could not send key file to otp_enc_d on port %d.\n", portNum);
		exit(1);
	}

	// Set all elements of inBuffer to 0, clearing the plaintext message.
	memset(inBuffer, 0, BUFFER_SIZE);

	// Receive ciphertext from otp_enc_d.
	numCharsRecvd = read(socketFD, inBuffer, lengthOFplaintext - 1);

	// Error check call to read().
	if (numCharsRecvd < 1) {
		printf("otp_enc Error: Call to read() for receiving ciphertext message from otp_enc_d failed!\n");
		exit(1);
	}

	// Print to stdout ciphertext message received.
	for (i = 0; i < numCharsRecvd - 1; i++) {
		printf("%c", inBuffer[i]);
	}

	// Add newline to ciphertext ouput stream.
	printf("\n");

	// Close socket.
	close(socketFD);
	return 0;
}
