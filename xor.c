// XOR (Exclusive Or)
#include <stdio.h>

// Prototype.
int xor(int a, int b);

int main(void) {
	printf("1 xor 0 is %d\n", xor(1, 0) ); // 1
	printf("1 xor 1 is %d\n", xor(1, 1) ); // 0
	printf("0 xor 1 is %d\n", xor(0, 1) ); // 1
	printf("0 xor 0 is %d\n", xor(0, 0) ); // 0
	return 0;
}

/* Perform a logical XOR operation using the
two arguments. */
int xor(int a, int b){
	return (a || b) && !(a && b);
}

// From C The Complete Reference 4th Edit., Schildt.