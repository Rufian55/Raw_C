/*******************************************************************************************
* slapper.c is a demo program showing how to extract pieces of lines from a text file.
* compile with gcc slapper.c -o slap -g -Wall
*******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void slap();

int main() {
	slap();
	return 0;
}

void slap() {

	FILE *fp = fopen("testy", "r");

	// Declare a c-string container for reading. 
	char str[12];

	fscanf(fp, "ROOM NAME: %s\n", str);
	printf("str = %s\n", str);

	int read;
	int conn_num;
	while ((read = fscanf(fp, "CONNECTION %d: %s\n", &conn_num, str)) != 0 && read != EOF) {
		printf("str = %s\n", str);
		printf("con# = %d\n", conn_num);
	}

	// Read ROOM TYPE and assign to aRoom.
	fscanf(fp, "ROOM TYPE: %s\n", str);
	printf("str = %s\n", str);

	fclose(fp);
}

/* The text file, named "testy" located in the same directory...
ROOM NAME: ReticentRoom
CONNECTION 1: Richard
CONNECTION 2: Stallma
CONNECTION 3: was
CONNECTION 4: here!!!!!
ROOM TYPE: ROMPER_ROOM
*/