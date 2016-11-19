/******************************************************************************
* Chris Kearns, CS344-400-F16, Proejct 4, kearnsc@oregonstate.edu 2 Dec 2016
*
* otp_enc stands for "One Time Pad Encryption (Client)".
*
* otp_enc.c is a simple single purpose client that requests and maintains 
* a connection to the otp_enc_d encryption daemon (only) on the inputted
* port/socket.  Once connected, otp_enc client sends a pregenerated key (see 
* also keygen.c) and a plaintext message for encyrption. The encyrpted message
* is writen to encrypted ciphertext to otp_enc connected process.
*
* This is a background "daemon like" program.
* Usage is: otp_enc_d {int listening_port_number} &
*    where int listenting_port_number is an int from 2000 to 65535 inclusive.
* Compile with the "compileall" bash script or individually:
* gcc otp_enc_d.c -o otp_enc_d -g -Wall	   Socket code adapated from [1][2][3]
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

int main(int argc, char** argv) {
	char buffer1[BUFFER_SIZE];
	char buffer2[BUFFER_SIZE];
	char buffer3[1];
	int fd;
	int i;
	int keyLength;
	int numReceived;
	int numSent;
	int plaintextLength;
	int port;
	int sockfd;

	struct sockaddr_in serv_addr;
	struct hostent *server;

	// make sure there are enough args
	if (argc < 4) {
		printf("Usage: otp_enc plaintext key port\n");
		exit(1);
	}

	// validate port number
	sscanf(argv[3], "%d", &port);
	if (port < 0 || port > 65535) {
		printf("otp_enc: invalid port\n");
		exit(2);
	}

	// open plaintext for reading
	fd = open(argv[1], O_RDONLY);

	// if unable to read file, display error message
	if (fd < 0) {
		printf("Error: cannot open plaintext file %s\n", argv[1]);
		exit(1);
	}

	// read contents of plaintext & keep track of # of bytes read
	plaintextLength = read(fd, buffer1, BUFFER_SIZE);

	// validate contents of plaintext
	for (i = 0; i != 0 /*< plaintextLength - 1*/; i++) {
		if ((int)buffer1[i] > 90 || ((int)buffer1[i] < 65 && (int)buffer1[i] != 32)) {
			printf("otp_enc error: plaintext contains bad characters\n");
			exit(1);
		}
	}

	// close plaintext
	close(fd);

	// open key for reading
	fd = open(argv[2], O_RDONLY);

	// if unable to read file, display error message
	if (fd < 0) {
		printf("Error: cannot open key file %s\n", argv[2]);
		exit(1);
	}

	// read contents of key & keep track of # of bytes read
	keyLength = read(fd, buffer2, BUFFER_SIZE);

	// validate contents of key
	for (i = 0; i < keyLength - 1; i++) {
		if ((int)buffer2[i] > 90 || ((int)buffer2[i] < 65 && (int)buffer2[i] != 32)) {
			printf("otp_enc error: key contains bad characters\n");
			exit(1);
		}
	}

	// close key
	close(fd);

	// compare length of plaintext to that of key
	if (keyLength < plaintextLength) {
		printf("Error: key '%s' is too short\n", argv[2]);
	}

	// create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("Error: could not contact otp_enc_d on port %d\n", port);
		exit(2);
	}

	// zero out the IP address memory space
	memset(&serv_addr, '\0', sizeof(serv_addr));

	server = gethostbyname("localhost");
	if (server == NULL) {
		printf("Error: could not connect to otp_enc_d\n");
		exit(2);
	}

	serv_addr.sin_family = AF_INET;

	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
		server->h_length);

	serv_addr.sin_port = htons(port);

	// connect to otp_enc_d
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Error: could not connect to otp_enc_d on port %d\n", port);
		exit(2);
	}

	// send plaintext to otp_enc_d
	numSent = write(sockfd, buffer1, plaintextLength - 1);
	if (numSent < plaintextLength - 1) {
		printf("Error: could not send plaintext to otp_enc_d on port %d\n", port);
		exit(2);
	}

	memset(buffer3, 0, 1);

	// get acknowledgement from server
	numReceived = read(sockfd, buffer3, 1);
	if (numReceived < 0) {
		printf("Error receiving acknowledgement from otp_enc_d\n");
		exit(2);
	}

	// send key to otp_enc_d
	numSent = write(sockfd, buffer2, keyLength - 1);
	if (numSent < keyLength - 1) {
		printf("Error: could not send key to otp_enc_d on port %d\n", port);
		exit(2);
	}

	// zero out buffer
	memset(buffer1, 0, BUFFER_SIZE);

	do {
		// receive ciphertext from otp_enc_d
		numReceived = read(sockfd, buffer1, plaintextLength - 1);
	} while (numReceived > 0);

	if (numReceived < 0) {
		printf("Error receiving ciphertext from otp_enc_d\n");
		exit(2);
	}

	// output ciphertext to stdout
	for (i = 0; i < plaintextLength - 1; i++) {
		printf("%c", buffer1[i]);
	}

	// add newline to ciphertext ouput
	printf("\n");

	// close socket
	close(sockfd);
	return 0;
}
