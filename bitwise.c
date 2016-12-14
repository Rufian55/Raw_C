// Bitwise Operations
#include <stdio.h>

// Prototypes.
char unsetParityBit();
int setParityBit();
int xorMyInt();
int notMyInt();
int notInt(int);
int shiftDemo();
char read_char();
int read_int();

int main() {
	char testy;

/*	11000001	"A" with parity bit set.
	01111111	"127" in binary.
   & --------
	01000001	"A" with parity bit unset.	*/

	testy = unsetParityBit();
	printf("testy = %c\n", testy);


/*	10000000	"128" in binary.
	00000011	"3" in binary.
   | --------
	10000011	"3" in binary with parity bit set.	
			but you see -125 (binary 1000 0011) */

	testy = (int)testy;
	testy = setParityBit();
	printf("testy = %d\n", testy);


/*	11111111	"127" in binary.
	01111000	"120" in binary.
   ^ --------
	00000111	which is "7" in binary. */

	testy = xorMyInt();
	printf("testy = %d\n", testy);

/*	Original byte:		00101100 which is binary for 44.
	One's complement:	11010011 which is binary for -45.
	One's complement:	00101100 back to 44.
*/

	testy = notMyInt();
	printf("not testy = %d\n", testy);
	testy = notInt(testy);
	printf("testy restored = %d\n", testy);

	// Demonstrate bit shifting...
	shiftDemo();

	return 0;
}


char unsetParityBit() {
	char ch;
	// Get a char from the user.
	ch = read_char();
	return (ch & 127);
}

int setParityBit() {
	int anInt;
	anInt = read_int();
	return (128 | anInt);
}

int xorMyInt() {
	int anInt;
	anInt = read_int();
	return (127 ^ anInt);
}

int notMyInt() {
	int anInt = read_int();
	return (~anInt);
}

int notInt(int testy) {
	return (~testy);
}


int shiftDemo() {
	int i = read_int();
	int j;

	/* left shifts */
	for (j = 0; j < 4; j++) {
		// Left shift i by 1, which is the same as a multiply by 2.
		i = i << 1;
		printf("Left shift %d: %d\n", j, i);
	}
	/* right shifts */
	for (j = 0; j < 4; j++) {
		// Right shift i by 1, which is the same as a division by 2.
		i = i >> 1;
		printf("Right shift %d: %d\n", j, i);
	}
	return 0;
}


char read_char() {
	char c;
	printf("Please enter your char!\n");
	scanf("%c", &c);
	getchar();	// Consume the newline with the trailing space.
	return c;
}

int read_int() {
	int i;
	printf("Please enter your integer!\n");
	scanf("%d", &i);
	getchar();	// Consume the newline with the trailing space.
	return i;
}

// Adapted from C - The Complete Reference 4th Edit., Schildt.