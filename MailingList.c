/******************************************************************************
* MailingList.c is a rudimentary mailing list program. 
* Compile: gcc MailingList.c -o mailer -g -Wall
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
void initList();
int menuSelect();
void enter();
int findFree();
void delete();
void list();
void trim(char *);

int main() {
	char choice;
	initList(); // Initialize the structure array.

	while(1) {
		choice = menuSelect();
		switch (choice) {
		case 1: enter();
			break;
		case 2: delete();
			break;
		case 3: list();
			break;
		case 4: exit(0);
		}
	}

	return 0;
}

// Initialize the list.
void initList() {
	int t;
	for (t = 0; t < MAX; t++) {
		addr_list[t].name[0] = '\0';
	}
}


// Get menu selection.
int menuSelect() {
	char s[80];
	int c;
	printf("\n1. Enter a record.\n");
	printf("2. Delete a record.\n");
	printf("3. Print the records.\n");
	printf("4. Quit.\n");

	do {
		printf("\nEnter your choice: ");
		fgets(s, sizeof(s), stdin);
		c = atoi(s);
	} while (c < 0 || c > 4);

	return c;
}


// Input addresses into the list.
void enter() {
	int slot;
	slot = findFree();
	if (slot == -1) {
		printf("\nList Full\n");
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
int findFree() {
	int t;
	for (t = 0; addr_list[t].name[0] && t < MAX; t++) {
		if (t == MAX)
			return -1; // No slots free.
	}
	return t;
}


// Delete an address.
void delete() {
	int slot;
	char s[80];
	printf("Enter record #: ");
	fgets(s, sizeof(s), stdin);
	slot = atoi(s);
	if (slot >= 0 && slot < MAX) {
		addr_list[slot].name[0] = '\0';
	}
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


// Trim off the trailing '\n'.
void trim(char *str) {
	int i;
	for (i = 0; i < strlen(str); i++) {
		if (str[i] == '\n') {
			str[i] = '\0';
		}
	}
}