/****************************************************************************************
* recuse.c demonstrates recusion and iteration...
* Compile with "gcc recurse.c -o recurse -g -Wall".
*****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>

// Prototype.
int R_factorial(int);
int I_factorial(int);

int main() {
	int a,b,f;
	printf("Enter an int to be factored.\n");
	scanf("%d", &f);

	f = (int)f;// Just in case a char is entered.

	if (f < 1 || f > 13) {
		printf("Factorial man! ints between 1 and 13 inclusive only!\n");
		exit(EXIT_FAILURE);
	}

	a = R_factorial(f);
	b = I_factorial(f);

	printf("%d Recursively factored = %d\n%d Iteratively factored = %d\n", f, a, f, b);

	return 0;
}

/* Recursive */
int R_factorial(int n) {
	int answer;
	if (n == 1)
		return 1;

	answer = R_factorial(n - 1) * n;
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
