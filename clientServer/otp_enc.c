/******************************************************************************
* Chris Kearns, CS344-400-F16, Proejct 4, kearnsc@oregonstate.edu 2 Dec 2016
* otp_enc stands for "One Time Pad Encryption (Client)".
* otp_enc.c is a simple single purpose client that requests and maintains 
* a connection to the otp_enc_d encryption daemon (only) on the inputted
* port/socket.  Once connected, otp_enc client sends a plaintext message and
* a pregenerated key (see also keygen.c) for encyrption. The encyrpted message
* is written to a user specified file and output to the screen.
* This program will exit after succesful completion of an encrypted message 
* but can be called again immediately for additional encyrption work.
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

#define TEST 0

int main(int argc, char** argv) {
	long err;						// General purpose error testing long int whose max size should be 2,147,483,647.
	int FD;						// General purpose File Descriptor.
	int i;						// The ubiquitous looping variable.
	int portNum;					// The user defined communcations port number.
	int socketFD;					// The communications socket.
	long lengthOfMsg;				// Calculated length of plaintext message - range unknown, so type long.
	size_t convertedLOM;			// lengthOfMsg converted via htonl().
	long lengthOfKey;				// Calculated length of key - range unknown, so type long.
	size_t convertedLOK;			// lengthOfKey converted via htonl().		
	struct stat buffer;				// Needed for calls to stat when determining file size. [1]
	struct sockaddr_in serverAddress;	// Struct containing the internet address of the server as defined in netinet/in.h.
	struct hostent *server;			// Pointer to struct type hostent as defined in the header file netdb.h
	char *plaintextMsg;				// Pointer to plaintext message storage buffer.
	char *key;					// Pointer to key storage buffer.
	char *cipherText;				// Pointer to otp_enc_d returned cipherText message storage buffer.
	char syn[6] = "j5K(e";			// Initial Handshake password, 5 chars and '\0'.
	char synAck[6] = "x#sB2";		// Expected Handshake Acnowlegment returned from Server, 10 chars and '\0'.
	char ack[6];					// ACK buffer to store the handshake message acknowledgement from server. 

	// Error check for correct number of arguments.
	if (argc < 4) {
		fprintf(stderr, "Usage Error: otp_enc {plaintext_message_file} {key_file} {port_number}\n");
		exit(EXIT_FAILURE);
	}

	// Test for acceptable port number range.
	sscanf(argv[3], "%d", &portNum);
	if (portNum < 2000 || portNum > 65535) {
		fprintf(stderr, "otp_enc Error_1: Invalid port number - range is 2000 - 65535 inclusive!\n");
		exit(EXIT_FAILURE);
	}

	if (TEST) {
		fprintf(stdout, "Port number = %d.\n", portNum);
	}

	/* READ PLAINTEXT MESSAGE FILE */

	// Determine length of message while still in plaintext message file!
	err = stat(argv[1], &buffer);

	if (err < 0) {
		fprintf(stderr, "otp_enc Error_2: call to stat() failed! Does file \"%s\" exist?\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	// Assign length of plaintext message and drop the newline.
	lengthOfMsg = buffer.st_size - 1;

	if (TEST) {
		fprintf(stdout, "Var lengthOfMsg = %ld.\n", lengthOfMsg);
	}

	// Allocate memory for the plaintext message to be read in.
	plaintextMsg = malloc(lengthOfMsg * sizeof(char));

	// Error test call to malloc().
	if (plaintextMsg == NULL) {
		fprintf(stderr, "otp_enc Error_3: Memory Allocation Failure.\n");
		exit(EXIT_FAILURE);
	}

	// Open plaintext message file for reading.
	FD = open(argv[1], O_RDONLY);

	// Error test call to open() FD.
	if (FD < 1) {
		fprintf(stderr, "otp_enc Error_4: Cannot open file %s.\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	// Read contents of plaintext message file.
	err = read(FD, plaintextMsg, lengthOfMsg);

	// Error test call to read(). Picks up fialure (-1) and empty file (0).
	if (err < 0) {
		fprintf(stderr, "otp_enc Error_5: reading plaintext message file \"%s\" failed!\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	if (TEST) {
		fprintf(stdout, "plaintext message = %s\n", plaintextMsg);
	}

	// Validate contents of plaintext message.
	for (i = 0; i < lengthOfMsg; i++) {
		if ((int)plaintextMsg[i] > 90 || ((int)plaintextMsg[i] < 65 && (int)plaintextMsg[i] != 32)) {
			fprintf(stderr, "otp_enc Error_6: plaintextMsg string contains bad characters!\n");
			exit(EXIT_FAILURE);
		}
	}

	// Close plaintext message file.
	close(FD);


	/*READ KEY FILE*/

	// Determine length of key while still in plaintext message file!
	err = stat(argv[2], &buffer);

	if (err < 0) {
		fprintf(stderr, "otp_enc Error_7: call to stat() failed! Does file %s exist?\n", argv[2]);
		exit(EXIT_FAILURE);
	}

	// Assign length of key and drop the newline.
	lengthOfKey = buffer.st_size - 1;

	if (TEST) {
		fprintf(stdout, "Var lengthOfKey = %ld.\n", lengthOfKey);
	}

	// Allocate memory for the key to be read in.
	key = malloc(lengthOfKey * sizeof(char));

	// Error test call to malloc().
	if (key == NULL) {
		fprintf(stderr, "otp_enc Error_8: Memory Allocation Failure.\n");
		exit(EXIT_FAILURE);
	}

	// Open plaintext message file for reading.
	FD = open(argv[2], O_RDONLY);

	// Error test call to open() FD.
	if (FD < 0) {
		fprintf(stderr, "otp_enc Error_9: Cannot open file %s.\n", argv[2]);
		exit(EXIT_FAILURE);
	}

	// Read contents of plaintext message file.
	err = read(FD, key, lengthOfKey);

	// Error test call to read(). Picks up failure (-1) and empty file (0).
	if (err < 0) {
		fprintf(stderr, "otp_enc Error_10: reading key file \"%s\" failed!\n", argv[2]);
		exit(EXIT_FAILURE);
	}

	if (TEST) {
		fprintf(stdout, "key string = %s\n", key);
	}

	// Validate contents of plaintext
	for (i = 0; i < lengthOfKey; i++) {
		if ((int)key[i] > 90 || ((int)key[i] < 65 && (int)key[i] != 32)) {
			fprintf(stderr, "otp_enc Error_11: key string contains bad characters!\n");
			exit(EXIT_FAILURE);
		}
	}

	// Close key file.
	close(FD);

	// Test length of strings contained in plaintext string and key string.
	if (lengthOfKey < lengthOfMsg) {
		fprintf(stderr, "otp_enc Error_12: key file '%s' is too short! %s = %ld chars, "
					 "message = %ld chars.\n", argv[2], argv[2], lengthOfKey, lengthOfMsg);
		exit(EXIT_FAILURE);
	}

	// Create communications socket for otp_enc.
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	// Error test call to socket().
	if (socketFD < 0) {
		fprintf(stderr, "otp_enc Error_13: Call to socket() failed!\n");
		exit(EXIT_FAILURE);
	}

	// Clear all elements of struct serv_addr_in serv_addr.
	memset(&serverAddress, '\0', sizeof(serverAddress));

	// Set struct hostent server to localhost.
	server = gethostbyname("localhost");

	// Test call to gethostbyname().
	if (server == NULL) {
		fprintf(stderr, "otp_enc Error_14: Call to gethostname() failed!\n");
		exit(EXIT_FAILURE);
	}

	// Initialize the serverAddress sin_ struct members.
	serverAddress.sin_family = AF_INET;		// Address Family = Internet IPv4.
	serverAddress.sin_port = htons(portNum);	/* Converts portNum in host byte order to portNum in
									     network byte order (Big/Little Endian). */

	// Copies n bytes from src to dest [ void bcopy(void *src, void *dest, size_t n); ] note casts.
	bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
	
	// Connect to otp_enc_d
	if (connect(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
		fprintf(stderr, "otp_enc Error_15: Could not connect to otp_enc_d on port %d\n", portNum);
		exit(EXIT_FAILURE);
	}

	// Write SYN handshake message.
	err = write(socketFD, syn, 5);

	// Error check call to write().
	if(err != 5) {
		fprintf(stderr, "otp_enc Error_16: SYN handshake message write failed!\n");
	}

	// Initialize ack buffer to all '/0'.
	memset(ack, '\0', 6);

	// Read for acknowledgement from otp_enc_d server.
	err = read(socketFD, ack, 5);
	if (err != 5) {
		fprintf(stderr, "otp_enc Error_17: call to read ack failed, err = %ld.\n", err);
	}

	if (TEST) {
		printf("ack = %s\n", ack);
		printf("synAck = %s\n", synAck);
	}

	// Compare returned ack and our expected synAck (if succesful, otp_enc_d and otp_enc passwords matched!)
	if (strncmp(ack, synAck, 5) != 0) {
		fprintf(stderr, "otp_enc Error_18: Handshake to otp_enc_d failed! Is port number %d correct?\n", portNum);
		exit(EXIT_FAILURE);
	}

	/* Made it here, so task otp_enc_d to encrypt a string with the key! But first, we have to send the 
	   lengthOfMsg and lengthOfKey size_t integer vars to otp_enc_d so that program can allocate memory, etc. [2]*/

	// Convert lengthofMsg var to network Endian.
	convertedLOM = htonl(lengthOfMsg);

	if (TEST) {
		printf("convertedLOM = %zu\n", convertedLOM);
	}

	// Send the lengthOfMsg int to otp_enc_d.
	err = write(socketFD, &convertedLOM, sizeof(convertedLOM));

	if (TEST) {
		printf("otp_enc err for write convertedLOM = %ld\n", err);
	}

	// Error check call to write().
	if (err < 0) {
		fprintf(stderr, "otp_enc Error_19: Call to write lengthOfMsg to otp_enc_d failed!\n");
		exit(EXIT_FAILURE);
	}


	// Convert lengthOfMsg var to network Endian.
	convertedLOK = htonl(lengthOfKey);

	if (TEST) {
		printf("convertedLOK = %zu\n", convertedLOK);
	}

	// Send the lengthOfMsg int to otp_enc_d.
	err = write(socketFD, &convertedLOK, sizeof(convertedLOK));

	if (TEST) {
		printf("otp_enc err for write convertedLOK = %ld\n", err);
	}

	// Error check call to write().
	if (err < 0) {
		fprintf(stderr, "otp_enc Error_20: Call to write lengthOfKey to otp_enc_d failed!\n");
		exit(EXIT_FAILURE);
	}

	// Send plaintext message to otp_enc_d daemon.
	err = write(socketFD, plaintextMsg, lengthOfMsg);

	// Error check call to write().
	if (err < 0) {
		fprintf(stderr, "otp_enc Error_21: Could not send plaintext message to otp_enc_d on port %d.\n", portNum);
		exit(EXIT_FAILURE);
	}

	// Send key file to otp_enc_d
	err = write(socketFD, key, lengthOfKey);

	// Error check call to write().
	if (err < 0) {
		fprintf(stderr, "otp_enc Error_22: Could not send key file to otp_enc_d. err = %ld.\n", err);
		exit(EXIT_FAILURE);
	}

	// Allocate memory for otp_enc_d returned cipherText string storage buffer.
	cipherText = malloc(lengthOfMsg * sizeof(char));

	// Check for error condition on call to malloc().
	if (cipherText == NULL) {
		fprintf(stderr, "otp_enc Error_23: Memory Allocation failed!\n");
		exit(EXIT_FAILURE);
	}

	// Receive ciphertext from otp_enc_d - the assumption being the ciphertext is the same length as the plaintextMsg we sent.
	err = read(socketFD, cipherText, lengthOfMsg);

	// Error check call to read().
	if (err < 0) {
		fprintf(stderr, "otp_enc Error_24: Call to read() for receiving ciphertext message from otp_enc_d failed!\n");
		exit(EXIT_FAILURE);
	}

	// Print to stdout ciphertext message received.
	for (i = 0; i < lengthOfMsg; i++) {
		fprintf(stdout, "%c", cipherText[i]);
	}

	// Add newline to ciphertext ouput stream.
	fprintf(stdout, "\n");

	// Close socket.
	close(socketFD);

	// Manage memory.
	free(plaintextMsg);
	plaintextMsg = NULL;
	free(key);
	key = NULL;
	free(cipherText);
	cipherText = NULL;
	return 0;
}

/* CITATIONS:
[1] http://man7.org/linux/man-pages/man2/stat.2.html
[2] http://stackoverflow.com/questions/9140409/transfer-integer-over-a-socket-in-c
*/