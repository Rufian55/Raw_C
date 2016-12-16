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

/* Return index of first match of s2 in s1. */
int findSubString(char *s1, char *s2) {
	int t;
	char *p, *p2;
	for (t = 0; s1[t]; t++) {
		p = &s1[t];
		p2 = s2;
		while (*p2 && *p2 == *p) {
			p++;
			p2++;
		}
		if (!*p2)
			return t;
	}
	return -1;
}