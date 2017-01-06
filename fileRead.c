/******************************************************************************
* fileRead.c is basically similar to calling cat <filename> from the command 
* line. 
* Compile: gcc fileRead.c -o fileRead -g -Wall
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	FILE *fp;
	char ch;
	if ((fp = fopen(argv[1], "r")) == NULL) {
		printf("Cannot open file.\n");
		exit(1);
	}
	while ((ch = getc(fp)) != EOF) {
		printf("%c", ch);
	}
	fclose(fp);
	return 0;
}