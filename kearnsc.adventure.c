#include <stdio.h>
#include <sys/stat.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

const char *roomName[] = {"Blue", "Blullow", "Cyan", "Green", "Grellow", "Magenta", "Red", "Redllow", "White", "Yellow"};
const char *roomType[] = {"START_ROOM", "MID_ROOM", "MID_ROOM", "MID_ROOM", "MID_ROOM", "MID_ROOM", "END_ROOM"};
int roomSelector[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int connSelector[] = {0, 1, 2, 3, 4, 5, 6};
char selected[7][8];

int rand_int(int n) {
	int limit = RAND_MAX - RAND_MAX % n;
	int rnd;
	do {
		rnd = rand();
	} while (rnd >= limit);
	return rnd % n;
}

void shuffle(int *array, int n) {
	int i,j,tmp;
	for (i = n-1; i > 0; i--) {
		j = rand_int(i + 1);
		tmp = array[j];
		array[j] = array[i];
		array[i] = tmp;
	}
}

int getRand3_6(){
	int r = rand() % 4 + 3;
	return r;
}


int main() {
	time_t t;
	srand((unsigned) time(&t));
	int i,j,r;

	char dirName[0] = "";
	char myRooms[] = "kearnsc.rooms.";
	sprintf(dirName, "%s%d", myRooms, getpid());
	mkdir(dirName, 0755);

	shuffle(roomSelector, 10);	

	char fileName[0] = "";

	chdir(dirName);
	FILE *fp;
	for (i = 0; i < 7; i++) {
		sprintf(fileName, "%s", roomName[roomSelector[i]]);
		//Push room names to selected[].
		strncpy(selected[i], fileName, strlen(fileName));		
		fp = fopen(fileName, "w");
		fprintf(fp, "%s%s\n", "ROOM NAME: ", fileName);
		fclose(fp);
		sprintf(fileName,"%s", "");
	}

		//Conections
//		r = getRand3_6();
//		for (j = 1; j <= r; j++) {
//			fprintf(fp, "%s%d%s%s\n", "CONNECTION ", j, ": ", "<room name>");
//		}
//		fprintf(fp, "%s%s\n", "ROOM TYPE: ", roomType[i]);


	chdir("..");

	for (i=0; i<7; i++) {
		printf("selected[%d] = %s\n", i, selected[i]);
	}
	shuffle(connSelector, 7);
	for (i=0; i<7; i++) {
		printf("selected[%d] = %s\n", i, selected[connSelector[i]]);
	}
	
	return 0;
}


/* CITATIONS
[ ] http://www.gnu.org/software/libc/manual/html_node/Creating-Directories.html
[ ] http://stackoverflow.com/questions/10147990/how-to-create-directory-with-right-permissons-using-c-on-posix
[ ] http://cboard.cprogramming.com/c-programming/165757-using-process-id-name-file-directory.html
[ ] http://www.sanfoundry.com/c-program-implement-fisher-yates-algorithm-array-shuffling/
[ ] https://www.tutorialspoint.com/c_standard_library/c_function_srand.htm
[ ] https://www.tutorialspoint.com/cprogramming/c_file_io.htm
[ ] https://www.tutorialspoint.com/c_standard_library/c_function_strncat.htm
[ ] https://www.tutorialspoint.com/c_standard_library/c_function_strlen.htm
[ ] http://stackoverflow.com/questions/8107826/proper-way-to-empty-a-c-string
[ ] http://stackoverflow.com/questions/13204650/how-to-chdir-in-c-program-in-linux-environment
[ ] http://stackoverflow.com/questions/20265328/readdir-beginning-with-dots-instead-of-files
[ ] http://stackoverflow.com/questions/4853556/how-to-add-an-element-to-a-string-array-in-c
[ ] 
[ ] 
[ ] 
[ ] 
[ ] 
[ ] 
*/
