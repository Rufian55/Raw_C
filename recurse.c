/****************************************************************************************
* findChar.c demonstrates how finding a single char within a string.
* Compile with "gcc findChar.c -o findChar -g -Wall".
*****************************************************************************************/
#include <stdio.h>

// Prototype.
int R_factorial(int);
int I_factorial(int);

int main() {
	int a,b,f;
	printf("Enter an int to be factored.\n");
	fgets(f, sizeof(f), stdin);

	a = R_factorial(f);
	b = I_factorial(f);

	printf("%d Recursively factored = %d\n%d Iteratively factored = %d\n", f, a, f, b);

	return 0;
}

/* Recursive */
int R_factorial(int n) {
	int answer;
	if (n == l)
		return 1;

	answer = R_factorial(n - l) * n;
	return(answer);
}

/* Non-recursive */
int I_factorial(int n) {
	int t, answer;
	answer = 1;
	for (t = 1; t <= n; t++)
		answer *= t;
	return(answer);
}
