#include<stdio.h>
#include<string.h>

void converge(char *target, char * source);

int main() {
	char target[80] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"; //30.
	converge(target, "This is a test of converge().");
	printf("Final string: %s\n", target);

	return 0;
}


// Copies from both ends, converging in the middle.
void converge(char *target, char *source) {
	int i, j;
	printf("%s\n", target);

	// i set to incement from 0, j set to decrement from 30.
	for (i = 0, j = strlen(source); i <= j; i++, j--) {
		target[i] = source[i];
		target[j] = source[j];
		printf("%s\n", target);
	}
}
