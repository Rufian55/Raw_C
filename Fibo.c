/*****************************************************************************
* Fibo.c returns the Fibonnaci sequence number per inputted by user. There are
* three different sequence number generator algorithms included with there
* running time and space utilization factors annotated.
* Compile with: gcc Fibo.c -o Fibo -g -Wall
* Adapted from materials presented by J. Shufert, Oregon State Univ., W17
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

// Prototypes
int fib_1(int n);
int fib_2(int n);
int fib_3(int n);

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: \"Fibo n\" where n = the sequence number you wish to see.\n");
		exit(1);
	}
	int n = atoi(argv[1]);
	printf("The Fibonacci number for sequence %d is %d\n", n, fib_1(n));
//	getchar();
	return 0;
}


// Fibonacci Series using Space Optimized Method runs Θ(n)
// and space utiization of O(1).
int fib_1(int n) {
	int a = 0, b = 1, c, i;
	if (n == 0) {
		return a;
	}
	for (i = 2; i <= n; i++)	{
		c = a + b;
		a = b;
		b = c;
	}
	return b;
}


// Fibonacci Series using Dynamic Programming runs O(n) but
// also has a space utilization of O(n) so NOT USED.
int fib_2(int n) {
	int i;
	int f[n + 1]; // Array to store previously calculated.

	// 0 and 1 return 0 and 1
	f[0] = 0;
	f[1] = 1;

	for (i = 2; i <= n; i++)	{
		// Add the previous 2 numbers in the series and store.
		f[i] = f[i - 1] + f[i - 2];
	}
	return f[n];
}


//Fibonacci Series using Recursion runs O(n^2) so NOT USED.
int fib_3(int n) {
	if (n <= 1) {
		return n;
	}
	return fib_3(n - 1) + fib_3(n - 2);
}