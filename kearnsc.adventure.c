#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#define NAME_BUFFER_LEN 9	// User input buffer length - magenta + 1 for '\0'.
#define TYPE_BUFFER 11		// Room type enumeration START_ROOM /= 1 for '\0'.

// Room names array.
char *room_names[NUM_ROOM_NAMES] = {
	"Blue", "Brown", "Cyan", "Green", "Gold",
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
// Rooms created storage array.
char selected[7][8];


// Prototypes.
int getRand3_6();
void shuffle(int *, int );
void pickRoomNames(char**);
struct room *make_rooms();
bool connect(int, int, struct room rooms_list[NUM_ROOMS]);
bool is_connected(int, int);
void memCleaner();

int main() {
	// Seed the random number generator.
	time_t t;
	srand((unsigned)time(&t));

	// Build and make directory.
	char dirName[0] = "";
	char myRooms[] = "kearns.rooms.";
	sprintf(dirName, "%s%d", myRooms, getpid());
	mkdir(dirName, 0755);

	pickRoomNames(room_names);
	int i;
	for (i = 0; i < 10; i++) {
		printf("%s ", selected[i]);
	}
	printf("\n");

	make_rooms();
	for (i = 0; i < NUM_ROOMS; i++) {
		printf("%s %i %i %i\n", rooms_list[i].name, rooms_list[i].max_conns, rooms_list[i].num_conns, rooms_list[i].type);
	}

	//	struct room *read_room = read_rooms();
	//	runGame();
	//	clearRooms(read_room);

//	memCleaner();
	return 0;
}

// Returns an int from 3 to 6 inclusive.
int getRand3_6() {
	int r = rand() % (MAX_CONNS - MIN_CONNS + 1) + 3;
	return r;
}


struct room *make_rooms() {
	int i,j;
	// Create taken array with NUM_ROOMS elements of type bool.
	bool taken[NUM_ROOM_NAMES];
	// Set all taken's to false.
	memset(&taken, 0, NUM_ROOM_NAMES * sizeof(bool));
	// Iterate through each room initializing as we iterate.
	for (i = 0; i < NUM_ROOMS; i++) {
		rooms_list[i].name = selected[i];
		rooms_list[i].max_conns = getRand3_6();
		rooms_list[i].num_conns = 0;
		rooms_list[i].type = MID_ROOM;
		for (j = 0; j < NUM_ROOMS; j++) {
			rooms_list[i].conns[j] = NULL;
		}
	}
	//Overwrite rooms_list[0] & [6] with START/END type.
	rooms_list[0].type = START_ROOM;
	rooms_list[NUM_ROOMS - 1].type = END_ROOM;

	// Connect the rooms: iterate through rooms_list.
	for (i = 0; i < NUM_ROOMS; i++) {
		// In each room, 
		for (j = 0; j < rooms_list[i].max_conns; j++) {
			// Create index of a random room to connect to.
			int aRoom = rand() % NUM_ROOMS;
			// Attempt to connect the rooms. If connect returns false,
			// create a new random room index and try again.
			while (connect(i, aRoom, rooms_list)) {
				aRoom = rand() % NUM_ROOMS;
			}
		}
	}

	return rooms_list;
}


// Fisher-Yates array shuffle algorithm. []
int rand_int(int n) {
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
		j = rand_int(i + 1);
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
//		selected[i][strlen(fileName)-1] = '\0';// Not needed, but just too cool to delete!!
		sprintf(fileName, "%s", "");
	}
	free(fileName);
}

//Returns true if the connections were made or rm1 is at MAX_CONNS, false otherwise.
bool connect(int rm1, int rm2, struct room rooms_list[NUM_ROOMS]) {
	//Don't connect a room to itself.
	if (rm1 == rm2) {
		return false;
	}

	struct room *room1 = &rooms_list[rm1];
	struct room *room2 = &rooms_list[rm2];

	// If number of connections of room1 is at MAX_CONNS return true.
	// We're done with that room.
	if (room1->num_conns == room1->max_conns) {
		return true;
	}

	// If rooms already connected, don't connect them again.
	if (is_connected(rm1, rm2)) {
		return false;
	}
	
	// If either of the rooms maximum number of connections has been
	// reached don't connect them.
//	if (room2->num_conns == room2->max_conns) {
//		return false;
//	}
	
	// Now connect the rooms.
	room1->conns[room1->num_conns] = room2;
	room2->conns[room2->num_conns] = room1;
	// Increment the number of connections for both rooms.
	room1->num_conns++;
	room2->num_conns++;
	return true;
}

// Determine if the two rooms are already connected.
bool is_connected(int room1, int room2) {
	int i;
	for (i = 0; i < rooms_list[room1].num_conns; i++) {
		if (rooms_list[room1].conns[i] == &rooms_list[room2] && rooms_list[room1].conns[i] != NULL) {
			return true;
		}
	}
	return false;
}



//void memCleaner() {
//}

// [] h ttp://www.sanfoundry.com/c-program-implement-fisher-yates-algorithm-array-shuffling/
