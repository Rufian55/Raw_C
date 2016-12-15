/****************************************************************************************
* funcPtr.c demonstrates how function pointers are declared and used.
* Compile with "gcc funcPtr.c -o funky -g -Wall".
*****************************************************************************************/
#include <stdio.h>

// Function Prototype.
int addOne(int inputArg);

int main() {

	int(*fpArg)(int) = addOne;
/*       ^     ^    ^        ^
	 |     |    |	     |
	 |     |    |        Sets @ of addOne() to fpArg.
	 |     |    Argument - same as addOne().
	 |     Declared function pointer fpArg.
	 Return.
*/

	printf("10 + 1 = %d\n", fpArg(10));
	// Use ptr to addOne()	  ^

	return 0;
}

// The AddOne function.
int addOne(int input) {
	return input + 1;
}
