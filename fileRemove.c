/******************************************************************************
* The program removes the user supplied file name but asks for confirmation 
* first. You could do the same much easier from the Command Line... ;)
* Compile with gcc fileRemove.c -o fileRemove -g -Wall
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char *argv[]) {
	char str[80];
	int len;
	if (argc != 2) {
		printf("Usage: fileRemove <filename>\n");
		exit(1);
	}
	printf("Erase %s? (Y/N): ", argv[1]);
	fgets(str, 2, stdin);
	// Trim the '/n' from fgets call.
	len = strlen(str);
	if (len > 0 && str[len - 1] == '\n') {
		str[len - 1] = '\0';
	}

	if (toupper(*str) == 'Y') {
		if (remove(argv[1])) {
			printf("Cannot erase file.\n");
			exit(1);
		}
	}
	return 0;
}