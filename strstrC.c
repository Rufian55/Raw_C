/****************************************************************************************
* strstrC.c demonstrates how finding a substring within a string.
* Compile with "gcc strstrC.c -o substr -g -Wall".
*****************************************************************************************/
#include <stdio.h>

// Prototype.
int findSubString(char *s1, char *s2);

int main() {
	if (findSubString("C is fun", "is") != -1)
		printf("Substring is found.\n");
	else
		printf("Substring not found!\n");
	return 0;
}

/* Return index of first match of substring s2 in string s1. */
int findSubString(char *s1, char *s2) {
	int t;
	// Declare 2 char pointers.
	char *p, *p2;
	// Iterate through s1 with t as index.
	for (t = 0; s1[t]; t++) {
		// Set p to each s1 element's memory address.
		p = &s1[t];
		// Set p2 to point at string s2.
		p2 = s2;
		/* Check each char of p2 to make sure there is another char and if it
		   matches, increment to the next char, checking for a match on each pass. */
		while (*p2 && *p2 == *p) {
			p++;
			p2++;
		}
		/* Check to see if all of s2 was consumed, if so, we have a match, and 
		   now know the starting index (t) of s2 within s1. */
		if (!*p2)
			return t;
	}
	// No match...
	return -1;
}