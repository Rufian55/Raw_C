#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

// Constants.
#define MIN_CONNS 3			// Minimum connections per graph node.
#define MAX_CONNS 6			// Maximum connections per graph node.
#define NUM_ROOMS 7			// Number of room in the graph.
#define NUM_ROOM_NAMES 10	// Number of room names to choose from.
#define NAME_BUFFER_LEN 9	// User input buffer length - magenta + 1 for '\0' + 1 for \n from fgets().
#define TYPE_BUFFER 11		// Room type enumeration START_ROOM /= 1 for '\0'.
#define MAX_LOOP 250		// Inhibits infinite loop while generating room connections.
#define MAX_READ 12			// Max chars to read back in function readRoom().

// Room names array.
char *room_names[NUM_ROOM_NAMES] = {
	"Blue", "Brown", "Cyan", "Green", "Indigo",
	"Magenta", "Red", "Silver", "White", "Yellow"
};

// Three room types enumeration.
enum room_type {
	START_ROOM,
	END_ROOM,
	MID_ROOM
};

// Graph node struct of type room.
struct room {
	enum room_type type;					// Each graph node struct has access to room_type enum.
	char *name;							// Name of the individual graph node struct.
	int max_conns;							// Max connections allowed.
	int num_conns;							// Actual number of connections assigned.
	struct room *conns[NUM_ROOMS];			// Connections array for each nodes assigned connection names.
};

// Declare rooms_list[7] array of type struct room for the 7 randomly selected rooms. 
struct room rooms_list[NUM_ROOMS];
// Declare helper array to populate rooms_list with 7 unique room names in unordered fashion.
int roomSelector[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
// Rooms created name storage array - helper for populating rooms_list.
char selected[7][8];
int mem0=0, mem1;

// Function prototypes.
int getRand3_6();
void shuffle(int *, int);
void pickRoomNames(char**);
void make_rooms();
void connect(int, int, struct room rooms_list[NUM_ROOMS]);
bool ok2connect(int, int, struct room rooms_list[NUM_ROOMS]);
void writeRooms(struct room rooms[NUM_ROOMS]);
struct room* readAllRooms();
struct room readRoom(char *);
char *getName(char *);
struct room *getRoom(char *);
void runGame(struct room *);
//void runGame();
void printConns(struct room *);
void theEnd(int, struct room **, int);
void print_room(int);
void print1Room(struct room *);
void tellTime();
void memCleaner(struct room *);

int main() {
	tellTime();
	time_t t;								// Declare a time_t var. [ ]
	srand((unsigned)time(&t));				// Seed the random number generator.
	pickRoomNames(room_names);				// Generate random unique list of rooms.
	make_rooms();							// Make rooms and connect graph datastructure.
	writeRooms(rooms_list);					// Write rooms to specified child directory.
	struct room *read_room = readAllRooms();	// Read rooms from specified child directory.
	runGame(read_room);						// Run the game!
//	runGame();
	memCleaner(read_room);
	return 0;
}

// Returns an int from 3 to 6 inclusive.
int getRand3_6() {
	int r = rand() % (MAX_CONNS - MIN_CONNS + 1) + 3;
	return r;
}

// Fisher-Yates array shuffle algorithm. []
// Note internal _rand_int() function.
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
	shuffle(roomSelector, 10);
	int i;
	char *fileName = malloc(NAME_BUFFER_LEN);
	for (i = 0; i < 7; i++) {
		sprintf(fileName, "%s", room_names[roomSelector[i]]);
		strncpy(selected[i], fileName, strlen(fileName));
		sprintf(fileName, "%s", "");
	}
	free(fileName);
}


void make_rooms() {
	int i, j, aRoom, quit;
	// Iterate through each room initializing as we iterate.
	for (i = 0; i < NUM_ROOMS; i++) {
		rooms_list[i].name = selected[i];
		rooms_list[i].max_conns = getRand3_6(); // targeted # of connections for this room.
		rooms_list[i].num_conns = 0;
		rooms_list[i].type = MID_ROOM;
		for (j = 0; j < NUM_ROOMS; j++) {
			rooms_list[i].conns[j] = NULL;
		}
	}
	//Overwrite rooms_list[0] & [6] with START/END type.
	rooms_list[0].type = START_ROOM;
	rooms_list[NUM_ROOMS - 1].type = END_ROOM;

	// Connect the rooms: For each room,
	for (i = 0; i < NUM_ROOMS; i++) {
		// For max allowable conns in rooms_list, 
		for (j = 0; j < rooms_list[i].max_conns; j++) {
			// Inhibits infinte while loop. Set limit @ MAX_LOOP above.
			quit = 0; 
			// Create index of a random room to connect to.
			aRoom = rand() % NUM_ROOMS;
			// Attempt to connect the rooms. If connect() returns false,
			// create a new random room index and try again.
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


// Coonects two rooms - no checks!
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


// Determine if ok2connect by checking if the two rooms are:
// The same: return false.
// Already connected: return false.
// Room 2 (rm2) has reached its max connections: return false.
// Otherwise indicate OK to connect: return true.
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
void writeRooms(struct room rooms[NUM_ROOMS]) {
	// Build directory string.
	char *dirName = malloc(sizeof(char) * 19);
	char myRooms[] = "kearns.rooms.";
	sprintf(dirName, "%s%d", myRooms, getpid());

	// Create the directory in parent(current) directory.
	mkdir(dirName, 0755);

	// Change to the child directory.
	chdir(dirName);
	// For each room make a file.
	int i,j;
	for (i = 0; i < NUM_ROOMS; i++) {
		// Open file.
		FILE *fp = fopen(rooms[i].name, "w");
		// Write room name.
		fprintf(fp, "ROOM NAME: %s\n", rooms[i].name);
		// Write connections.
		for (j = 0; j < rooms[i].num_conns; j++) {
			fprintf(fp, "CONNECTION %d: %s\n", j + 1, rooms[i].conns[j]->name);
		}
		// Write room type via switch().
		switch (rooms[i].type) {
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
struct room* readAllRooms() {
	struct room *rooms = malloc(NUM_ROOMS * sizeof(struct room));
	int room_count = 0;
	// Build directory string.
	char *dirName = malloc(sizeof(char) * 19);
	char myRooms[] = "kearns.rooms.";
	sprintf(dirName, "%s%d", myRooms, getpid());

	// Change to child directory.
	chdir(dirName);
	// Make sure directory exists.
	DIR *dp;											// [ ]
	struct dirent *dir;
	dp = opendir(".");
	if (dp) {
		// Read the rooms individually fromn the directory.
		while ((dir = readdir(dp)) != NULL) {
			if (dir->d_name[0] != '.') {
				mem0++;
				rooms[room_count] = readRoom(dir->d_name);
			}
		}
	}
	else {
		printf("Failed to open %s Directory!\n", dirName);
	}
	// Close child directory.
	closedir(dp);
	// Manage memory.
	free(dirName);
	// Return to child's parent directory.
	chdir("..");

	return rooms;
}


// Read and return a single room from file.
struct room readRoom(char *name) {
	printf("%s\n", name);
	// Declare a struct room.
	struct room aRoom;
	// Open the file by name parameter.
	FILE *fp = fopen(name, "r");
	// Declare a c-string container for reading. 
	char received_name[MAX_READ];

	fscanf(fp, "ROOM NAME: %s\n", name);
	aRoom.name = getName(name);

	int read;
	int conn_number;
	while ((read = fscanf(fp, "CONNECTION %d: %s\n", &conn_number, received_name)) != 0 && read != EOF) {
		aRoom.conns[conn_number - 1] = getRoom(received_name);
	}
	aRoom.num_conns = conn_number - 1;

	// Read ROOM TYPE and assign to aRoom.
	fscanf(fp, "ROOM TYPE: %s\n", received_name);
	// Determine and set aRoom's type.
	if (strcmp(name, "START_ROOM") == 0) {
		aRoom.type = START_ROOM;
		mem1 = mem0;
	}
	else if (strcmp(name, "END_ROOM") == 0) {
		aRoom.type = END_ROOM;
	}
	else {
		aRoom.type = MID_ROOM;
	}
	fclose(fp);

//	print1Room(&aRoom);

	return aRoom;
}


// Return just name of the room_names struct.
char *getName(char *roomName) {
	int i;
	for (i = 0; i < MAX_CONNS; i++) {
		if (strcmp(roomName, room_names[i]) == 0) {
			return room_names[i];
		}
	}
	return NULL;
}


// Return room matching user inputted names first 3 chars.
struct room *getRoom(char *roomName) {
	int i;
	for (i = 0; i < NUM_ROOMS; i++) {
		if (strcmp(roomName, rooms_list[i].name) == 0) {
			// Return the pointer!!
			return &rooms_list[i];
		}
	}
	return NULL;
}


// Main game loop.
void runGame(struct room *read_room) {
	// Set current room to start room.
//	struct room *current_room = &rooms_list[0];
	struct room *current_room = &read_room[mem1];

	// Allocate an array to hold the list of visited rooms.
	struct room **visited = malloc(sizeof(struct room*) * NUM_ROOMS);
	int visited_index = 0;
	int visited_cap = NUM_ROOMS;
	// Declare buffer container for user input.
	char buffer[NAME_BUFFER_LEN];	
	// Declare number of steps taken.
	int num_steps = 0;
	//skips the Huh?... message if true.
	bool skip_Huh;
	while (true) {
		skip_Huh = false;
		// If winning condition:
		if (current_room->type == END_ROOM) {
			theEnd(num_steps, visited, visited_index);
			free(visited);
			return;
		}
		// Print user prompt.
		printf("\nCURRENT LOCATION: %s\n", current_room->name);
		printConns(current_room);
		printf("WHERE TO? >");
		fgets(buffer, NAME_BUFFER_LEN, stdin);
		// Remove newline.
		buffer[strlen(buffer) - 1] = '\0';

		// Compare the user's input with all of the connections
		int i;
		for (i = 0; i < current_room->num_conns; i++) {
			// If connection matches user input.
			if (strncmp(buffer, current_room->conns[i]->name, NAME_BUFFER_LEN) == 0) {
				// Update current room to matched room.
				current_room = current_room->conns[i];
				// If user reaches capacity of visited[] reallocate x 2.
				if (visited_index >= visited_cap) {
					visited_cap += NUM_ROOMS;
					visited = realloc(visited, visited_cap * sizeof(struct room*) * 2);
				}
				// Push room onto visited[] array.
				visited[visited_index] = current_room;
				visited_index++;
				num_steps++;
				//Set skipHuh
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


// Congratulatory message.
void theEnd(int num_steps, struct room **visited, int visited_index) {
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", num_steps);
	int i;
	for (i = 0; i < visited_index; i++) {
		printf("%s\n", visited[i]->name);
	}
}


// Print room connections for user to choose from.
void printConns(struct room *aRoom) {
	printf("POSSIBLE CONNECTIONS: ");
	// Print all but last.
	int i;
	for (i = 0; i < aRoom->num_conns - 1; i++) {
		printf("%s, ", aRoom->conns[i]->name);
	}
	// Print last room with period on the end.
	if (aRoom->num_conns > 0) {
		printf("%s.\n", aRoom->conns[aRoom->num_conns - 1]->name);
	}
}


// Prints out a room's connections by int for debugging.
void print_room(int room) {
	printf("name: %s\nnum_conns %d\nmax_conns %d\n",
		rooms_list[room].name,
		rooms_list[room].num_conns,
		rooms_list[room].max_conns
	);
	int i;
	for (i = 0; i < rooms_list[room].num_conns; i++) {
		printf("connection: %s\n", rooms_list[room].conns[i]->name);
	}
	printf("\n");
}


// Prints out a single room's members for debugging.
void print1Room(struct room *aRoom) {
	printf("name: %s\nnum_conns %d\nmax_conns %d\ntype %i\n",
		aRoom->name,
		aRoom->num_conns,
		aRoom->max_conns,
		aRoom->type
	);
	int i;
	for (i = 0; i < aRoom->num_conns; i++) {
		printf("%s's conn[%d]: %s\n", aRoom->name, i, aRoom->conns[i]->name);
	}
	printf("\n");
}

// Prints the time. Utilizes the time.h library.
void tellTime() {							// [ ]
	time_t result = time(NULL);
	struct tm *t = localtime(&result);
	// Format time strings and ints...
	int hour = t->tm_hour;
	int ap = 0;
	if (hour > 11) { ap = 1; }
	if (hour > 12) { hour -= 12; }
	int min = t->tm_min;
	char *wkDay[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	int day = t->tm_wday;
	char *month[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	int mDay = t->tm_mon;
	int dMonth = t->tm_mday;
	int year = t->tm_year + 1900;	
	char *am_pm[2] = { "am", "pm" };
	// Print time in specified format.
	printf("\n%d:%d%s, %s, %s %d, %d\n", hour, min, am_pm[ap], wkDay[day], month[mDay], dMonth, year);
}


// Memory managment catch all.
void memCleaner(struct room *rooms) {
	free(rooms);
}

// [ ] h ttp://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
// [ ] h ttp://stackoverflow.com/questions/1442116/how-to-get-date-and-time-value-in-c-program (accepted answer)
// [ ] h ttp://en.cppreference.com/w/c/chrono/tm