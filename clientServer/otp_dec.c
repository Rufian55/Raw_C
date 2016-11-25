/******************************************************************************
* Chris Kearns, CS344-400-F16, Proejct 4, kearnsc@oregonstate.edu 2 Dec 2016
* otp_dec stands for "One Time Pad Decryption (Client)".
* otp_dec.c is a simple single purpose client that requests and maintains
* a connection to the otp_dec_d decryption daemon (only) on the inputted
* port/socket.  Once connected, otp_dec client sends an encrypted message and
* a pregenerated key (see also keygen.c) for decyrption. This key must be
* identical to the key that was used to encrypt the message! The decyrpted
* message is written to a user specified file and/or output to the screen.
* This program will exit after succesful completion of a decrypted message
* but can be called again immediately for additional decyrption work.
* Usage is: otp_dec {encrypted_message_file} {key_file} {port_number}
*  where port_number must match the port the otp_dec_d daemon was started on.
* Compile with the provided "compileall" bash script or individually:
* gcc otp_dec.c -o otp_dec -g -Wall	   Socket code adapated from [1][2][3]
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
	long err;						// General purpose error testing int.
	int FD;						// General purpose File Descriptor.
	int i;						// The ubiquitous looping variable.
	int portNum;					// The user defined communcations port number.
	int socketFD;					// The communications socket.
	long lengthOfMsg;				// Calculated length of ciphertext message - range unknown, so type long.
	size_t convertedLOM;			// lengthOfMsg converted via htonl().
	long lengthOfKey;				// Calculated length of key - range unknown, so type long.
	size_t convertedLOK;			// lengthOfKey converted via htonl().		
	struct stat buffer;				// Needed for calls to stat when determining file size. [1]
	struct sockaddr_in serverAddress;	// Struct containing the internet address of the server as defined in netinet/in.h.
	struct hostent *server;			// Pointer to struct type hostent as defined in the header file netdb.h
	char *plaintextMsg;				// Pointer to otp_dec_d returned plaintext message storage buffer.
	char *key;					// Pointer to key storage buffer.
	char *cipherText;				// Pointer to outgoing ciphertext message storage buffer.
	char syn[6] = "5m)C6";			// Initial Handshake password, 5 chars and '\0'.
	char synAck[6] = "u9G@_";		// Expected Handshake Acnowlegment returned from Server, 10 chars and '\0'.
	char ack[6];					// ACK buffer to store the handshake message acknowledgement from server. 

	// Error check for correct number of arguments.
	if (argc < 4) {
		fprintf(stderr, "Usage Error: otp_dec {encrypted_message_file} {key_file} {port_number}\n");
		exit(EXIT_FAILURE);
	}

	// Test for acceptable port number range.
	sscanf(argv[3], "%d", &portNum);
	if (portNum < 2000 || portNum > 65535) {
		fprintf(stderr, "otp_dec Error_1: Invalid port number - range is 2000 - 65535 inclusive!\n");
		exit(EXIT_FAILURE);
	}

	if (TEST) {
		fprintf(stdout, "Port number = %d.\n", portNum);
	}

	/* READ CIPHERTEXT MESSAGE FILE */

	// Determine length of message while still in ciphertext message file!
	err = stat(argv[1], &buffer);

	if (err < 0) {
		fprintf(stderr, "otp_dec Error_2: call to stat() failed! Does file \"%s\" exist?\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	// Assign length of cipherText message and drop the newline.
	lengthOfMsg = buffer.st_size - 1;

	if (TEST) {
		fprintf(stdout, "Var lengthOfMsg = %ld.\n", lengthOfMsg);
	}

	// Allocate memory for the cipherText message to be read in.
	cipherText = malloc(lengthOfMsg * sizeof(char));

	// Error test call to malloc().
	if (cipherText == NULL) {
		fprintf(stderr, "otp_dec Error_3: Memory Allocation Failure.\n");
		exit(EXIT_FAILURE);
	}

	// Open plaintext message file for reading.
	FD = open(argv[1], O_RDONLY);

	// Error test call to open() FD.
	if (FD < 1) {
		fprintf(stderr, "otp_dec Error_4: Cannot open file %s.\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	// Read contents of ciphertext message file.
	err = read(FD, cipherText, lengthOfMsg);

	// Error test call to read(). Picks up failure (-1) and empty file (0).
	if (err < 0) {
		fprintf(stderr, "otp_dec Error_5: reading cipherText message file \"%s\" failed!\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	if (TEST) {
		fprintf(stdout, "cipherText message = %s\n", cipherText);
	}

	// Validate contents of cipherText message.
	for (i = 0; i < lengthOfMsg; i++) {
		if ((int)cipherText[i] > 90 || ((int)cipherText[i] < 65 && (int)cipherText[i] != 32)) {
			fprintf(stderr, "otp_dec Error_6: %s contains bad characters!\n", argv[1]);
			exit(EXIT_FAILURE);
		}
	}

	// Close cipherText message file descriptor.
	close(FD);


	/*READ KEY FILE*/

	// Determine length of key while still in plaintext message file!
	err = stat(argv[2], &buffer);

	if (err < 0) {
		fprintf(stderr, "otp_dec Error_7: call to stat() failed! Does file %s exist?\n", argv[2]);
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
		fprintf(stderr, "otp_dec Error_8: Memory Allocation Failure.\n");
		exit(EXIT_FAILURE);
	}

	// Open plaintext message file for reading.
	FD = open(argv[2], O_RDONLY);

	// Error test call to open() FD.
	if (FD < 0) {
		fprintf(stderr, "otp_dec Error_9: Cannot open file %s.\n", argv[2]);
		exit(EXIT_FAILURE);
	}

	// Read contents of plaintext message file.
	err = read(FD, key, lengthOfKey);

	// Error test call to read(). Picks up failure (-1) and empty file (0).
	if (err < 0) {
		fprintf(stderr, "otp_dec Error_10: reading key file \"%s\" failed!\n", argv[2]);
		exit(EXIT_FAILURE);
	}

	if (TEST) {
		fprintf(stdout, "key string = %s\n", key);
	}

	// Validate contents of plaintext
	for (i = 0; i < lengthOfKey; i++) {
		if ((int)key[i] > 90 || ((int)key[i] < 65 && (int)key[i] != 32)) {
			fprintf(stderr, "otp_dec Error_11: key string contains bad characters!\n");
			exit(EXIT_FAILURE);
		}
	}

	// Close key file.
	close(FD);

	// Test length of strings contained in cipherText string and key string.
	if (lengthOfKey < lengthOfMsg) {
		fprintf(stderr, "otp_dec Error_12: key file '%s' is too short! %s = %ld chars, "
			"message = %ld chars.\n", argv[2], argv[2], lengthOfKey, lengthOfMsg);
		exit(EXIT_FAILURE);
	}

	// Create communications socket for otp_dec.
	socketFD = socket(AF_INET, SOCK_STREAM, 0);

	// Error test call to socket().
	if (socketFD < 0) {
		fprintf(stderr, "otp_dec Error_13: Call to socket() failed!\n");
		exit(EXIT_FAILURE);
	}

	// Clear all elements of struct serv_addr_in serv_addr.
	memset(&serverAddress, '\0', sizeof(serverAddress));

	// Set struct hostent server to localhost.
	server = gethostbyname("localhost");

	// Test call to gethostbyname().
	if (server == NULL) {
		fprintf(stderr, "otp_dec Error_14: Call to gethostname() failed!\n");
		exit(EXIT_FAILURE);
	}

	// Initialize the serverAddress sin_ struct members.
	serverAddress.sin_family = AF_INET;		// Address Family = Internet IPv4.
	serverAddress.sin_port = htons(portNum);	/* Converts portNum in host byte order to portNum in
										   network byte order (Big/Little Endian). */

	// Copies n bytes from src to dest [ void bcopy(void *src, void *dest, size_t n); ] note casts.
	bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);

	// Connect to otp_dec_d.
	if (connect(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
		fprintf(stderr, "otp_dec Error_15: Could not connect to otp_dec_d on port %d\n", portNum);
		exit(EXIT_FAILURE);
	}

	// Write SYN handshake message.
	err = write(socketFD, syn, 5);

	// Error check call to write().
	if (err != 5) {
		fprintf(stderr, "otp_dec Error_16: SYN handshake message write failed!\n");
	}

	// Initialize ack buffer to all '/0'.
	memset(ack, '\0', 6);

	// Read for acknowledgement from otp_enc_d server.
	err = read(socketFD, ack, 5);

	// Error check call to read().
	if (err != 5) {
		fprintf(stderr, "otp_dec Error_17: call to read ack failed, err = %ld.\n", err);
	}

	if (TEST) {
		printf("ack = %s\n", ack);
		printf("synAck = %s\n", synAck);
	}

	// Compare returned ack and our expected synAck (if succesful, otp_dec_d and otp_dec passwords matched!)
	if (strncmp(ack, synAck, 5) != 0) {
		fprintf(stderr, "otp_dec Error_18: Handshake to otp_dec_d failed, likely due to bad call to otp_enc_d on port %d.\n", portNum);
		exit(EXIT_FAILURE);
	}

	/* Made it here, so task otp_dec_d to unencrypt a string with the key! But first, we have to send the
	   lengthOfMsg and lengthOfKey long integer vars to otp_dec_d so that program can allocate memory, etc. [2]*/

	// Convert lengthofMsg var to network Endian.
	convertedLOM = htonl(lengthOfMsg);

	if (TEST) {
		printf("convertedLOM = %zu\n", convertedLOM);
	}

	// Send the lengthOfMsg int to otp_dec_d.
	err = write(socketFD, &convertedLOM, sizeof(convertedLOM));

	if (TEST) {
		printf("otp_dec err for write convertedLOM = %ld\n", err);
	}

	// Error check call to write().
	if (err < 0) {
		fprintf(stderr, "otp_dec Error_19: Call to write lengthOfMsg to otp_dec_d failed!\n");
		exit(EXIT_FAILURE);
	}

	// Convert lengthOfMsg var to network Endian.
	convertedLOK = htonl(lengthOfKey);

	if (TEST) {
		printf("convertedLOK = %zu\n", convertedLOK);
	}

	// Send the lengthOfMsg int to otp_dec_d.
	err = write(socketFD, &convertedLOK, sizeof(convertedLOK));

	if (TEST) {
		printf("otp_dec err for write convertedLOK = %ld\n", err);
	}

	// Error check call to write().
	if (err < 0) {
		fprintf(stderr, "otp_dec Error_20: Call to write lengthOfKey to otp_dec_d failed!\n");
		exit(EXIT_FAILURE);
	}

	// Send ciphertext message to otp_dec_d daemon.
	err = write(socketFD, cipherText, lengthOfMsg);

	// Error check call to write().
	if (err < 0) {
		fprintf(stderr, "otp_dec Error_21: Could not send cipherText message to otp_dec_d on port %d.\n", portNum);
		exit(EXIT_FAILURE);
	}

	// Send key file to otp_dec_d
	err = write(socketFD, key, lengthOfKey);

	// Error check call to write().
	if (err < 0) {
		fprintf(stderr, "otp_enc Error_22: Could not send key file to otp_dec_d on port %d.\n", portNum);
		exit(EXIT_FAILURE);
	}

	// Allocate memory for otp_dec_d returned plaintText string storage buffer.
	plaintextMsg = malloc(lengthOfMsg * sizeof(char));

	// Check for error condition on call to malloc().
	if (plaintextMsg == NULL) {
		fprintf(stderr, "otp_dec Error_23: Memory Allocation failed!\n");
		exit(EXIT_FAILURE);
	}

	// Receive plaintextMsg from otp_dec_d - the assumption being the plaintextMsg is the same length as the cipherText we sent.
	err = read(socketFD, plaintextMsg, lengthOfMsg);

	// Error check call to read().
	if (err < lengthOfMsg) {
		fprintf(stderr, "otp_dec Error_24: Call to read() for receiving plaintext message from otp_dec_d failed!\n");
		exit(EXIT_FAILURE);
	}

	// Print to stdout ciphertext message received.
	for (i = 0; i < lengthOfMsg; i++) {
		fprintf(stdout, "%c", plaintextMsg[i]);
	}

	// Add newline to ciphertext ouput stream.
	fprintf(stdout, "\n");

	// Close socket.
	close(socketFD);

	// Manage memory.
	free(cipherText);
	cipherText = NULL;
	free(key);
	key = NULL;
	free(plaintextMsg);
	plaintextMsg = NULL;
	return 0;
}

/* CITATIONS:
[1] http://man7.org/linux/man-pages/man2/stat.2.html
[2] http://stackoverflow.com/questions/9140409/transfer-integer-over-a-socket-in-c
*/