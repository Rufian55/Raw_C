/****************************************************************************************
* findChar.c demonstrates how finding a single char within a string.
* Compile with "gcc findChar.c -o findChar -g -Wall".
*****************************************************************************************/
#include <stdio.h>

// Prototype.
char *match(char c, char *s);

int main(void) {
	char s[80], *p, ch;

	printf("Enter the string to search.\n");
	fgets(s, sizeof(s), stdin);
	printf("Enter a single char to search.\n");
	ch = getchar();

	p = match(ch, s);

	if (*p) {
		printf("%c was matched!\n", *p);
		// Prints remainder of string from ch found.
		printf("%s\n", p);
	}
	else
		printf("No match found.\n");
	return 0;
}

/* Return pointer of first occurrence of c in s. */
char *match(char c, char *s) {
	while (c != *s && *s)
		s++;
	return(s);
}
