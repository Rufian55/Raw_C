/*****************************************************************************
* File: kearnsc.adventure.c	Date: 29 Oct 2016	Course: CS344-400-F16
* Compile: gcc kearnsc.adventure.c -o kearnsc.adventure -g -Wall
* Genrates a 7 node graph and randomly connects all nodes with between 3 and 6
* (inclusive) two-way connections each. [[Subsequently, writes the room struct
* members to a subdirectory, reads this file data back in, rebuilds the node
* structs and executes gameplay with the read back in data.]]  A time function
* is also provided. Call time, in place of a room name, and current local time
* is displayed.  This time process involves building the current time string,
* writing the string to file, reading the string back in, and displaying it.
* Note that tellTime() is executed by a seperate thread with mutex locks
* installed at first and last line of tellTime(). The thread is in runGame(). 
* Play continues until END_ROOM is visited.  A list of rooms visited and the
* number of steps is provided with the congratulatory message. Valgrind
* memcheck checks good, no leaks or errors reported.
******************************************************************************/
#include <dirent.h>	// DIR Directories.
#include <errno.h>	// Error printing.
#include <stdbool.h>	// Booleans.
#include <stdio.h>	// FILE Input/Output.
#include <stdlib.h>	// Standard library for C.
#include <string.h>	// String functions.
#include <pthread.h>	// Threads.
#include <time.h>	// Time.
#include <unistd.h>	// getpid().
#include <sys/stat.h>	// mkdir().
#include <sys/types.h>	// getpid().

// Constants.
#define MIN_CONNS 3		// Minimum connections per graph node.
#define MAX_CONNS 6		// Maximum connections per graph node.
#define NUM_ROOMS 7		// Number of room in the graph.
#define NUM_ROOM_NAMES 10	// Number of room names to choose from.
#define NAME_BUFFER_LEN 9	// User input buffer length - magenta + 1 for '\0' + 1 for \n from fgets().
#define TYPE_BUFFER 11		// Room type enumeration START_ROOM + 1 for '\0'.
#define MAX_LOOP 250		// Inhibits infinite loop while generating room connections.
#define MAX_READ 12		// Max chars to read back in function readRoom().
#define TIME 50			// Buffer size for reading and writing time with tellTime(), et.al.
#define DIR_NAME 20		// Directory name length limit.

// Three room types enumeration.
enum room_type { START_ROOM, END_ROOM, MID_ROOM };

// Graph node struct of type room.
struct room {
	enum room_type type;				// Each graph node struct has access to room_type enum.
	char *name;					// Name of the individual graph node struct.
	int max_conns;					// Max connections allowed.
	int num_conns;					// Actual number of connections assigned.
	struct room *conns[NUM_ROOMS];			// Connections array for each nodes assigned connection names.
	char rebuildConns[NUM_ROOMS][NAME_BUFFER_LEN];	// Holding array of room names used in readRoom();
};

/***** GLOBALS *****/

// Rooms created name storage array - helper for populating rooms_list.
char g_selected[NUM_ROOMS][NAME_BUFFER_LEN];

// Color selected c-string array for colorizing room names, time string, and console default. [13]
char *color[12] = { "\x1b[38;5;21m", "\x1b[38;5;130m", "\x1b[38;5;6m", "\x1b[38;5;2m",
				"\x1b[38;5;99m", "\x1b[38;5;201m", "\x1b[38;5;1m", "\x1b[38;5;247m",
				"\x1b[15m", "\x1b[38;5;3m", "\x1b[m", "\x1b[38;5;69m" };

// Initialize mutex lock. [16]
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**** END GLOBALS *****/

// Function prototypes (struct room's annotated w/ appropriate array name for clarity).
int getRand3_6();
void shuffle(int *, int);
void pickRoomNames(char** room_names);
void make_rooms(struct room rooms_list[NUM_ROOMS]);
void connect(int, int, struct room rooms_list[NUM_ROOMS]);
bool ok2connect(int, int, struct room rooms_list[NUM_ROOMS]);
void writeRooms(struct room rooms_list[NUM_ROOMS]);
struct room* readAllRooms(struct room rooms_rbi[NUM_ROOMS]);
struct room readRoom(char *);
struct room *getRoom(char *, struct room rooms_rbi[NUM_ROOMS]);
void rebuildConnections(struct room rooms_rbi[NUM_ROOMS]);
void runGame(struct room *room_rbi);
void printConns(struct room *);
void theEnd(int, struct room **, int);
void print_room(int, struct room rooms_list[NUM_ROOMS]);	// Unused - debug utility function.
void print1Room(struct room *);					// Unused - debug utility function.
void* tellTime(void *);						// Used by pthread pT_1.
void writeTime(char *);
void readTime();
int colorSelector(char *);
void memCleaner(struct room *);

int main() {
	time_t t;						// Declare a time_t var.
	srand((unsigned)time(&t));				// Seed the random number generator. [1]
	char *room_names[NUM_ROOM_NAMES] = {			// Room names array.
		"Blue", "Brown", "Cyan", "Green",
		"Indigo", "Magenta", "Red", "Silver",
		"White", "Yellow" };
	struct room rooms_list[NUM_ROOMS];			/* Declare rooms_list[7] array of type struct
								   room for the 7 randomly selected rooms. */
	pickRoomNames(room_names);				// Generate random unique list of rooms.
	make_rooms(rooms_list);					// Make rooms and connect graph datastructure.
	writeRooms(rooms_list);					// Write rooms to specified child directory.
	struct room rooms_rbi[NUM_ROOMS];			// Declare rooms_rbi[7] array for "rbi" rooms. 
	struct room *read_room = readAllRooms(rooms_rbi);	// Read rooms from specified child directory.
	runGame(read_room);					// Run the game from the read_room array!
	memCleaner(read_room);					// Manage memory.
	return 0;
}


// Returns an int from 3 to 6 inclusive.
int getRand3_6() {
	int r = rand() % (MAX_CONNS - MIN_CONNS + 1) + 3;
	return r;
}


/* Fisher-Yates array shuffle algorithm. [2]
   Note internal _rand_int() function. */
int _rand_int(int n) {
	int limit = RAND_MAX - RAND_MAX % n;
	int rnd;
	do {
		rnd = rand();
	} while (rnd >= limit);
	return rnd % n;
}
void shuffle(int *array, int n) {
	int i, j, tmp;
	for (i = n - 1; i > 0; i--) {
		j = _rand_int(i + 1);
		tmp = array[j];
		array[j] = array[i];
		array[i] = tmp;
	}
}


// Randomly selects unique room names and pushes to selected[]. 
void pickRoomNames(char** room_names) {
	// Declare helper array to populate rooms_list with 7 unique room names in unordered fashion.
	int roomSelector[NUM_ROOM_NAMES] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	// Randomize roomSelector[].
	shuffle(roomSelector, 10);
	int i;
	char *fileName = malloc(NAME_BUFFER_LEN);
	// Use roomSelector[i] as index while iterting room_names.
	for (i = 0; i < 7; i++) {
		sprintf(fileName, "%s", room_names[roomSelector[i]]);
		strncpy(g_selected[i], fileName, strlen(fileName));
		sprintf(fileName, "%s", "");
	}
	free(fileName);
}


// Populate the rooms_list[NUM_ROOMS] structs with randomly generated data.
void make_rooms(struct room rooms_list[NUM_ROOMS]) {
	int i, j, aRoom, quit;
	// Iterate through each room initializing as we iterate.
	for (i = 0; i < NUM_ROOMS; i++) {
		rooms_list[i].name = g_selected[i];
		rooms_list[i].max_conns = getRand3_6();		// Targeted # of connections for room "i".
		rooms_list[i].num_conns = 0;
		rooms_list[i].type = MID_ROOM;
		for (j = 0; j < NUM_ROOMS; j++) {		// Set all room connections to NULL.
			rooms_list[i].conns[j] = NULL;
		}
	}
	//Overwrite rooms_list[0] & [6] with START/END type.
	rooms_list[0].type = START_ROOM;
	rooms_list[NUM_ROOMS - 1].type = END_ROOM;

	// Connect the rooms: For each room,
	for (i = 0; i < NUM_ROOMS; i++) {
		// for max allowable conns in rooms_list, 
		for (j = 0; j < rooms_list[i].max_conns; j++) {
			// inhibit infinte while loop. Set limit @ MAX_LOOP above.
			quit = 0; 
			// Now create index of a random room to connect to,
			aRoom = rand() % NUM_ROOMS;
			// attempt to connect the rooms. If connect() returns false,
			// create a new random room index and try again upto MAX_LOOP times.
			while (rooms_list[i].num_conns < rooms_list[i].max_conns && quit < MAX_LOOP) {
				if (ok2connect(i, aRoom, rooms_list)) {
					connect(i, aRoom, rooms_list);
				}
				quit++;
				aRoom = rand() % NUM_ROOMS;
			}
		}
	}
}


// Connects two rooms - no checks!
void connect(int rm1, int rm2, struct room rooms_list[NUM_ROOMS]) {
	// Declare 2 struct rooms, setting them to rm1 & rm2.
	struct room *room1 = &rooms_list[rm1];
	struct room *room2 = &rooms_list[rm2];
	// Connect the rooms.
	room1->conns[room1->num_conns] = room2;
	room2->conns[room2->num_conns] = room1;
	// Increment connections for both rooms.
	room1->num_conns++;
	room2->num_conns++;
}


/* Determine if ok2connect by checking if the two rooms are:
 * The same: return false.
 * Already connected: return false.
 * Room 2 (rm2) has reached its max connections: return false.
 * Otherwise indicate OK to connect: return true.
 */
bool ok2connect(int rm1, int rm2, struct room rooms_list[NUM_ROOMS]) {
	// Fails if rm1 name same as rm2 name.
	if (strncmp(rooms_list[rm1].name, rooms_list[rm2].name, 3) == 0) {
		return false;
	}
	// Fails if rm1 connections list already contains rm2 as a connection.
	int i;
	for (i = 0; i < rooms_list[rm1].num_conns; i++) {
		if (strncmp(rooms_list[rm1].conns[i]->name, rooms_list[rm2].name, 3) == 0) {
			return false;
		}
	}
	// Fails if rm2 connections already has it max allowable connections.
	if (rooms_list[rm2].num_conns == rooms_list[rm2].max_conns) {
		return false;
	}
	return true;
}


// Write the room data to the hard coded directory.
void writeRooms(struct room rooms_list[NUM_ROOMS]) {
	// Build directory string.
	char *dirName = malloc(sizeof(char) * DIR_NAME);
	char myRooms[] = "kearnsc.rooms.";
	sprintf(dirName, "%s%d", myRooms, getpid());// [3]
	// Create the directory in parent(current) directory. [4]
	mkdir(dirName, 0755);
	// Change to the child directory. [5]
	chdir(dirName);
	// For each room make a file.
	int i,j;
	for (i = 0; i < NUM_ROOMS; i++) {
		// Open file.
		FILE *fp = fopen(rooms_list[i].name, "w");// [6]
		if (fp == NULL) {
			perror("writeRooms' fopen() call failed: ");
		}
		// Write room name.
		fprintf(fp, "ROOM NAME: %s\n", rooms_list[i].name);// [7]
		// Write connections.
		for (j = 0; j < rooms_list[i].num_conns; j++) {
			fprintf(fp, "CONNECTION %d: %s\n", j + 1, rooms_list[i].conns[j]->name);
		}
		// Write room type via switch().
		switch (rooms_list[i].type) {
		case END_ROOM:
			fprintf(fp, "ROOM TYPE: END_ROOM");
			break;
		case MID_ROOM:
			fprintf(fp, "ROOM TYPE: MID_ROOM");
			break;
		case START_ROOM:
			fprintf(fp, "ROOM TYPE: START_ROOM");
			break;
		}
		// Close file.
		fclose(fp);
	}
	// Return to parent directory.
	chdir("..");
	// Manage memory.
	free(dirName);
}


// Recreate the rooms structure by reading in the files.
struct room* readAllRooms(struct room rooms_rbi[NUM_ROOMS]) {
	// Build directory string.
	char *dirName = malloc(sizeof(char) * DIR_NAME);
	char myRooms[] = "kearnsc.rooms.";
	sprintf(dirName, "%s%d", myRooms, getpid());

	// Change to child directory.
	chdir(dirName);
	// Open dir and read in each room file.	[8]
	DIR *dp;
	struct dirent *dir;
	dp = opendir(".");
	int count = 0;
	if (dp) {
		// Read the rooms individually fromn the directory. [9][10]
		while ((dir = readdir(dp)) != NULL || count < NUM_ROOMS) {
			if (dir->d_name[0] != '.') {
				rooms_rbi[count] = readRoom(dir->d_name);
				count++;
			}
		}
	}
	else if (ENOENT == errno) { // [17]
		printf("readAllRooms' opendir() failed: %s directory does not exist!\n", dirName);
	}
	else {
		printf("readAllRooms' opendir() failed to open %s directory!\n", dirName);
	}
	// Close child directory.
	closedir(dp);
	// Manage memory.
	free(dirName);
	// Return to child's parent directory.
	chdir("..");
	/* Now that all rooms are rebuilt, populate connection
	   pointers via room struct's rebuildConns array. */
	rebuildConnections(rooms_rbi);
	return rooms_rbi;
}


// Read, build room, and return that single room from file.
struct room readRoom(char *rmName) {
	// Declare a struct room.
	struct room aRoom;
	// Open the file by name parameter.
	FILE *fp = fopen(rmName, "r");
	if (fp == NULL) {
		perror("readRoom's fopen() call failed: ");
	}
	// Declare a c-string container for reading. 
	char *str = malloc(sizeof(char) * MAX_READ);
	// Retrieve the room name.
	fscanf(fp, "ROOM NAME: %s\n", str);
	// Requires a malloc here else "issues". Free via memCleaner().
	aRoom.name = malloc(sizeof(char) * MAX_READ);
	strcpy(aRoom.name, str);
	int read;
	int conn_num;
	// Retrieve the room connection's connection number and name.
	while ((read = fscanf(fp, "CONNECTION %d: %s\n", &conn_num, str)) != 0 && read != EOF) {
		strcpy(aRoom.rebuildConns[conn_num -1],str);
	}
	aRoom.num_conns = conn_num;

	// Read ROOM TYPE and assign to aRoom.
	fscanf(fp, "ROOM TYPE: %s\n", str);

	// Determine and set aRoom's type.
	if (strcmp(str, "START_ROOM") == 0) {
		aRoom.type = START_ROOM;
	}
	else if (strcmp(str, "END_ROOM") == 0) {
		aRoom.type = END_ROOM;
	}
	else {
		aRoom.type = MID_ROOM;
	}

	fclose(fp);
	free(str);
	return aRoom;
}


/* Accepts rebuilt rooms_rbi (that does not yet have room conns
   rebuilt) and adds the room connection pointers via getRoom(). */
void rebuildConnections(struct room rooms_rbi[NUM_ROOMS]) {
	int i,j;
	// For each room in rooms_rbi,
	for (i = 0; i < NUM_ROOMS; i++) {
		// for each connection in room,
		for (j = 0; j < rooms_rbi[i].num_conns; j++) {
			// assign pointer to connected room via getRoom().
			rooms_rbi[i].conns[j] = getRoom(rooms_rbi[i].rebuildConns[j], rooms_rbi);
		}
	}
}


// Return room matching calling function's argument.
struct room *getRoom(char *roomName, struct room rooms_rbi[NUM_ROOMS]) {
	int i;
	for (i = 0; i < NUM_ROOMS; i++) {
		if (strcmp(roomName, rooms_rbi[i].name) == 0) {
			// Return the pointer!!
			return &rooms_rbi[i];
		}
	}
	return NULL;
}


// Main game loop. **NOTE - This function contains a thread.**
void runGame(struct room *read_room) {
	// Determine and set start roon index.
	int i, cur, q, ch;
	// Get read_room index for START_ROOM. 
	for (i = 0; i < NUM_ROOMS; i++) {
		if (read_room[i].type == START_ROOM) {
			cur = i;
		}
	}
	// Set current room to start room.
	struct room *current_room = &read_room[cur];
	// Allocate an array to hold the list of visited rooms.
	struct room **visited = malloc(sizeof(struct room*) * NUM_ROOMS);
	int visited_idx = 0;
	int visited_cap = NUM_ROOMS;
	// Declare buffer container for user input.
	char buffer[NAME_BUFFER_LEN];	
	// Declare number of steps taken.
	int num_steps = 0;
	// Skips the Huh?... message if true.
	bool skip_Huh;
	while (true) {
		skip_Huh = false;
		// If winning condition:
		if (current_room->type == END_ROOM) {
			theEnd(num_steps, visited, visited_idx);
			free(visited);
			return;
		}
		// Print user prompt.
		q = colorSelector(current_room->name);
		printf("\nCURRENT LOCATION: %s%s%s\n", color[q], current_room->name, color[10]);
		printConns(current_room);
		printf("WHERE TO? >");
		fgets(buffer, NAME_BUFFER_LEN, stdin);
		// Inhibit stdin/buffer overrun type ahead user behaviour, string spaces, etc.
		if (!strchr(buffer, '\n')) {
			// Consume rest of chars up to '\n'.
			while (((ch = getchar()) != EOF) && (ch != '\n'));// [14]
		}
		else {
			// Remove newline.
			buffer[strlen(buffer) - 1] = '\0';
		}

		// Compare the user's input with all of the connections.
		for (i = 0; i < current_room->num_conns; i++) {
			// If connection matches user input,
			if (strncmp(buffer, current_room->conns[i]->name, NAME_BUFFER_LEN) == 0) {
				// update current room to matched room.
				current_room = current_room->conns[i];
				// If user reaches capacity of visited[], reallocate x 2.
				if (visited_idx >= visited_cap) {
					visited_cap += NUM_ROOMS;
					visited = realloc(visited, visited_cap * sizeof(struct room*) * 2);
				}
				// Push room onto visited[] array.
				visited[visited_idx] = current_room;
				visited_idx++;
				num_steps++;
				// Set skip_Huh
				skip_Huh = false;
				break;
			} // Else, tell the local system time.
			else if (strncmp(buffer, "time", 4) == 0) {
				// Control changes to thread pT_1.	[16]
				// Initialize thread pT_1.
				pthread_t pT_1;
				/* Run tellTime() inside thread pT_1.
				   Mutex lock & unlock handled in tellTime(). */
				pthread_create(&pT_1, NULL, tellTime, NULL);
				// Join pT_1 thread to main()'s thread.
				pthread_join(pT_1, NULL);
				// Processing resumed by main()'s thread.
				readTime();
				skip_Huh = false;
				break;
			}
			else {
				skip_Huh = true;
			}
		}
		// No match - print error message and loop again.
		if (skip_Huh) {
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
		}
	}
	free(visited);
}


// Congratulatory message, steps taken, and rooms visited in appropriate color.
void theEnd(int num_steps, struct room **visited, int visited_index) {
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", num_steps);
	int i,q;
	for (i = 0; i < visited_index; i++) {
		q = colorSelector(visited[i]->name);
		printf("%s%s%s\n", color[q], visited[i]->name, color[10]);
	}
}


// Print room connections for user to choose from in appropriate color.
void printConns(struct room *aRoom) {
	printf("POSSIBLE CONNECTIONS: ");
	// Print all but last.
	int i, q;
	for (i = 0; i < aRoom->num_conns - 1; i++) {
		q = colorSelector(aRoom->conns[i]->name);
		printf("%s%s%s, ", color[q], aRoom->conns[i]->name, color[10]);
	}
	// Print last room with period on the end.
	if (aRoom->num_conns > 0) {
		q = colorSelector(aRoom->conns[aRoom->num_conns - 1]->name);
		printf("%s%s%s.\n", color[q], aRoom->conns[aRoom->num_conns - 1]->name, color[10]);
	}
}


// Prints out a room's members by index (for debugging).
void print_room(int room, struct room rooms_list[NUM_ROOMS]) {
	printf("name: %s\nnum_conns %d\nmax_conns %d\ntype %i\n",
		rooms_list[room].name,
		rooms_list[room].num_conns,
		rooms_list[room].max_conns,
		rooms_list[room].type
	);
	int i;
	for (i = 0; i < rooms_list[room].num_conns; i++) {
		printf("CONNECTION %d: %s\n", i+1, rooms_list[room].conns[i]->name);
	}
	printf("\n");
}


// Prints out a single room's members by struct room* (for debugging).
void print1Room(struct room *aRoom) {
	printf("name: %s\nnum_conns %d\nmax_conns %d\ntype %i\n",
		aRoom->name,
		aRoom->num_conns,
		aRoom->max_conns,
		aRoom->type
	);
	int i;
	for (i = 0; i < aRoom->num_conns; i++) {
		printf("CONNECTION %d: %s\n", i+1, aRoom->conns[i]->name);
	}
	printf("\n");
}


/* Prints the time. Utilizes the time.h library. [11][12]
   Note mutex lock/unlock [16] and function signature set
   for threading with void* pointer returned (but not utilized). */
void* tellTime(void *param) {
	// Lock process.
	pthread_mutex_lock(&lock);
	time_t result = time(NULL);
	struct tm *t = localtime(&result);
	// Format time strings and int modifiers...
	int hour = t->tm_hour;
	int ap = 0;
	if (hour > 11) { ap = 1; }				// am or pm.
	char *am_pm[2] = { "am", "pm" };
	if (hour > 12) { hour -= 12; }				// No 24 hour military time here!
	int min = t->tm_min;
	char *zero[2] = { "\0", "0" };				// Modifier for preceding 0, like 13:01
	int preMin = 0;						//  so that we can avoid
	if (min < 10 && min >= 0) { preMin = 1; }		//  this scenario: 10:1 instead of 10:01.
	char *wkDay[7] = { "Sunday", "Monday",
		"Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday" };
	int day = t->tm_wday;
	char *month[12] = { "January", "February",
		"March", "April", "May", "June",
		"July", "August", "September",
		"October", "November", "December" };
	int mDay = t->tm_mon;
	int dMonth = t->tm_mday;
	int year = t->tm_year + 1900;	
	// Build theTime c-string.
	char *theTime = malloc(sizeof(char) * 50);
	sprintf(theTime, "%d:%s%d%s, %s, %s %d, %d",
		hour, zero[preMin], min, am_pm[ap],
		wkDay[day], month[mDay], dMonth, year);
	// Write theTime string to file.
	writeTime(theTime);

	// Below is to just print the time without the file write/read operation for debug.
	// printf("\n%d:%s%d%s, %s, %s %d, %d\n", hour, zero[preMin], min, am_pm[ap], wkDay[day], month[mDay], dMonth, year);

	free(theTime);
	// Unlock process.
	pthread_mutex_unlock(&lock);
	return 0;
}

// Writes passed argument "theTime" to dirName/currentTime.txt.
void writeTime(char *theTime) {
	// Build directory string.
	char *dirName = malloc(sizeof(char) * DIR_NAME);
	char myRooms[] = "kearnsc.rooms.";
	sprintf(dirName, "%s%d", myRooms, getpid());
	// Change to the child directory.
	chdir(dirName);
	FILE *fp = fopen("currentTime.txt", "w");
	if (fp == NULL) {
		perror("writeTime() fopen() Failed: ");
	}
	fprintf(fp, "%s", theTime);
	fclose(fp);
	// Change to parent directory.
	chdir("..");
	free(dirName);
}


// Reads dirName/currentTime.txt contents to char *the2timer.
void readTime() {
	// Read back in theTime string back in.
	char *the2timer = malloc(sizeof(char) * TIME);
	// Build directory string.
	char *dirName = malloc(sizeof(char) * DIR_NAME);
	char myRooms[] = "kearnsc.rooms.";
	sprintf(dirName, "%s%d", myRooms, getpid());
	// Change to the child directory.
	chdir(dirName);
	FILE *fp = fopen("currentTime.txt", "r");
	if (fp == NULL) {
		perror("readTime fopen() call failed: ");
	}
	fgets(the2timer, TIME, fp);
	fclose(fp);
	// Change to parent directory.
	chdir("..");
	// Print time in specified format.
	printf("\n%s%s%s\n", color[11], the2timer, color[10]);

	free(dirName);
	free(the2timer);
}


/* Returns corresponding int based upon input c-string.
   Console default is color[10]. */
int colorSelector(char *color) {
	int result = 10;

	if (strcmp(color, "Blue") == 0) {
		result = 0;
	}
	else if (strcmp(color, "Brown") == 0) {
		result = 1;
	}
	else if (strcmp(color, "Cyan") == 0) {
		result = 2;
	}
	else if (strcmp(color, "Green") == 0) {
		result = 3;
	}
	else if (strcmp(color, "Indigo") == 0) {
		result = 4;
	}
	else if (strcmp(color, "Magenta") == 0) {
		result = 5;
	}
	else if (strcmp(color, "Red") == 0) {
		result = 6;
	}
	else if (strcmp(color, "Silver") == 0) {
		result = 7;
	}
	else if (strcmp(color, "White") == 0) {
		result = 8;
	}
	else if (strcmp(color, "Yellow") == 0) {
		result = 9;
	}
	return result;
}


// Memory management catch all.
void memCleaner(struct room *rooms) {
	int i;
	for (i = 0; i < NUM_ROOMS; i++) {
		free(rooms[i].name);
	}
}


/* CITATIONS - All code is by author with the exception of adapting information found at the following sources:
[1] https://www.tutorialspoint.com/c_standard_library/c_function_srand.htm
[2] http://www.sanfoundry.com/c-program-implement-fisher-yates-algorithm-array-shuffling/
[3] http://cboard.cprogramming.com/c-programming/165757-using-process-id-name-file-directory.html
[4] http://stackoverflow.com/questions/10147990/how-to-create-directory-with-right-permissons-using-c-on-posix
[5] http://stackoverflow.com/questions/13204650/how-to-chdir-in-c-program-in-linux-environment
[6] https://www.tutorialspoint.com/cprogramming/c_file_io.htm
[7] http://stackoverflow.com/questions/4853556/how-to-add-an-element-to-a-string-array-in-c
[8] http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
[9] http://stackoverflow.com/questions/20265328/readdir-beginning-with-dots-instead-of-files
[10] http://www.gnu.org/software/libc/manual/html_node/Creating-Directories.html
[11] http://stackoverflow.com/questions/1442116/how-to-get-date-and-time-value-in-c-program (accepted answer)
[12] http://en.cppreference.com/w/c/chrono/tm
[13] http://bitmote.com/index.php?post/2012/11/19/Using-ANSI-Color-Codes-to-Colorize-Your-Bash-Prompt-on-Linux
[14] https://stackoverflow.com/questions/38767967/clear-input-buffer-after-fgets-in-c (user "pmg" answer)
[15] https://computing.llnl.gov/tutorials/pthreads/
[16] http://stackoverflow.com/questions/14888027/mutex-lock-threads (accepted answer)
[17] http://stackoverflow.com/questions/12510874/how-can-i-check-if-a-directory-exists-on-linux-in-c (accepted answer)
[**] Generally, lecture materials by Prof. B. Brewster, CS344-400-F16, Oregon State University.
*/
