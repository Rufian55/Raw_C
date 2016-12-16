/****************************************************************************************
* funcPtr_2.c demonstrates how function pointers are declared and used.
* Compile with "gcc funcPtr_2.c -o funky2 -g -Wall".
*****************************************************************************************/
#include <stdio.h>
#include <string.h>

// Prototype.
void check(char *a, char *b, int(*cmp)(const char *, const char *));

int main() {
	char s1[80], s2[80];
	// Declare function pointer p.
	int(*p)(const char *, const char *);
	// Assign address of strcmp to p.
	p = strcmp;

	printf("Enter first string to be compared.\n");
	fgets(s1, sizeof(s1), stdin);

	printf("Enter second string to be compared.\n");
	fgets(s2, sizeof(s2), stdin);

	// Pass address of strcmp via p.
	check(s1, s2, p);
	return 0;
}

void check(char *a, char *b, int(*cmp) (const char *, const char *)) {
	printf("Testing for equality.\n");
	if (!(*cmp)(a, b))
		printf("Equal\n");
	else
		printf("Not Equal\n");
}