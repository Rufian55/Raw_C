/******************************************************************************
* mailList.c is a rudimentary mailing list program that reads and writes db 
* info to disk file, therefore offers data persistance. 
* Compile: gcc mailList.c -o mail -g -Wall
* TODO: Edit(), Sort(), add fields to struct e.g. addr1, addr2, phone...
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 10000

struct addr {
	char name[32];
	char street[42];
	char city[22];
	char state[4]; // Two char state abbreviation, '\n', and '\0'.
	char zip[12];
} addr_list[MAX];

// Protopypes.
void init_list();
void enter();
void delete();
void list();
void load();
void save();
int menu_select();
int find_free();
void trim(char *);

int main() {
	char choice;
	// Initialize the struct addr arrays.
	init_list();

	while(1) {
		choice = menu_select();
		switch (choice) {
		case 1: enter();
			break;
		case 2: delete();
			break;
		case 3: list();
			break;
		case 4: save();
			break;
		case 5: load();
			break;
		case 6: exit(0);
		}
	}
	return 0;
}

/* Initialize the list. */
void init_list() {
	int t;
	for (t = 0; t < MAX; ++t) {
		addr_list[t].name[0] = '\0';
	}
}

// Get a menu selection.
int menu_select() {
	char s[80];
	int c;
	printf("1. Enter a Record.\n");
	printf("2. Delete a Record.\n");
	printf("3. List all the Records.\n");
	printf("4. Save to file.\n");
	printf("5. Load from file.\n");
	printf("6. Quit!\n");
	do {
		printf("\nEnter your choice: ");
		fgets(s, sizeof(s), stdin);
		c = atoi(s);
	} while (c < 0 || c > 6);
	return c;
}

// Input addresses into the list.
void enter() {
	int slot = find_free();
	if (slot == -1) {
		printf("\nList Full");
		return;
	}
	printf("Enter name: ");
	fgets(addr_list[slot].name, sizeof(addr_list[slot].name), stdin);
	trim(addr_list[slot].name);

	printf("Enter street: ");
	fgets(addr_list[slot].street, sizeof(addr_list[slot].street), stdin);
	trim(addr_list[slot].street);

	printf("Enter city: ");
	fgets(addr_list[slot].city, sizeof(addr_list[slot].city), stdin);
	trim(addr_list[slot].city);

	printf("Enter state: ");
	fgets(addr_list[slot].state, sizeof(addr_list[slot].state), stdin);
	trim(addr_list[slot].state);

	printf("Enter zip: ");
	fgets(addr_list[slot].zip, sizeof(addr_list[slot].zip), stdin);
	trim(addr_list[slot].zip);
}

// Find an unused structure.
int find_free() {
	int t;
	for (t = 0; addr_list[t].name[0] && t < MAX; t++);
	if (t == MAX) return -1; // No slots free.
	return t;
}

// Delete an address.
void delete() {
	int slot;
	char s[80];
	printf("Enter record #: ");
	fgets(s, sizeof(s), stdin);
	slot = atoi(s);
	if (slot >= 0 && slot < MAX)
		addr_list[slot].name[0] = '\0';
}

// Display the list to the screen.
void list() {
	int t;
	for (t = 0; t < MAX; t++) {
		if (addr_list[t].name[0]) {
			printf("Record Number: %d\n", t);
			printf("%s\n", addr_list[t].name);
			printf("%s\n", addr_list[t].street);
			printf("%s\n", addr_list[t].city);
			printf("%s, %s\n", addr_list[t].state, addr_list[t].zip);
		}
	}
	printf("\n\n");
}

// Save the list.
void save() {
	FILE *fp;
	int i;
	if ((fp = fopen("maillist", "wb")) == NULL) {
		printf("Cannot open file.\n");
		return;
	}
	for (i = 0; i < MAX; i++) {
		if (*addr_list[i].name) {
			if (fwrite(&addr_list[i], sizeof(struct addr), 1, fp) != 1) {
				printf("File write error.\n");
			}
		}
	}
	fclose(fp);
}

// Load the file.
void load() {
	FILE *fp;
	int i;
	if ((fp = fopen("maillist", "rb")) == NULL) {
		printf("Cannot open file.\n");
		return;
	}
	init_list();
	for (i = 0; i < MAX; i++) {
		if (fread(&addr_list[i], sizeof(struct addr), 1, fp) != 1) {
			if (feof(fp)) break;
			printf("File read error.\n");
		}
	}
	fclose(fp);
}

// Trim off the trailing '\n'.
void trim(char *str) {
	int i;
	i = strlen(str) - 1;
	if (str[i] == '\n') {
		str[i] = '\0';
	}
}
