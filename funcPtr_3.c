/****************************************************************************************
* funcPtr_3.c demonstrates how function pointers are declared and used.
* Compile with "gcc funcPtr_3.c -o funky3 -g -Wall".
*****************************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Prototypes.
void check(char *a, char *b, int(*cmp)(const char *, const char *));
int compvalues(const char *a, const char *b);

int main() {
	char s1[80], s2[80];

	printf("Enter first value or string.\n");
	fgets(s1, sizeof(s1), stdin);
	printf("Enter 2nd value or string.\n");
	fgets(s2, sizeof(s2), stdin);

	if(isdigit(*s1)) {
		printf("Testing values for equality.\n");
			check(s1, s2, compvalues);
	}
	else {
		printf("Testing strings for equality.\n");
		check(s1, s2, strcmp);
	}
	return 0;
}

void check(char *a, char *b, int(*cmp)(const char *, const char *)) {
	if(!cmp(a, b))
		printf("Equal\n");
	else
		printf("Not Equal\n");
}

int compvalues(const char *a, const char *b) {
	if (atoi(a) == atoi(b))
		return 0;
	else
		return 1;
}