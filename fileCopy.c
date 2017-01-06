/******************************************************************************
* fileCopy.c copies the contents of a file to another file.
* Compile with gcc fileCopy.c -o fileCopy -g- Wall
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	FILE *in, *out;
	char ch;
	if (argc != 3) {
		printf("You forgot to enter a filename.\n");
			exit(1);
	}
	if ((in = fopen(argv[1], "rb")) == NULL) {
		printf("Cannot open input file.\n");
	exit(1);
	}
	if ((out = fopen(argv[2], "wb")) == NULL) {
		printf("Cannot open output file.\n");
		exit(1);
	}
	while (!feof(in)) {
		ch = getc(in);
		if (ferror(in)) {
			printf("Read Error");
			clearerr(in);
			break;
		}
		else {
			if (!feof(in)) putc(ch, out);
			if (ferror(out)) {
				printf("Write Error");
				clearerr(out);
				break;
			}
		}
	}
	fclose(in);
	fclose(out);
	return 0;
}