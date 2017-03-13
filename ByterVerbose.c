/******************************************************************************
* ByteVerbose.c converts a user supplied int from 0 to 255 inclusive to a byte
* array and prints it.  Input is via user polling or from the command line.
* Output is verbose to show how the program works.
* Compile: gcc -o ByterV ByteVerbose.c -g -Wall
******************************************************************************/
#include<stdio.h>
#include<stdlib.h>

#define SHOW_HOW 1

int main(int argc, char**argv) {
	int decimalNum;
	int *binaryNum;
	int i = 0;
	int sizeOfArray;
	int decimalNumCopied;
	int printInput;

	if (argc == 2) {
		decimalNum = atoi(argv[1]);
	}
	if(argc == 1 || decimalNum < 0 || decimalNum > 255){
		printf("Please enter a number from 0 to 255: ");
		scanf("%d", &decimalNum);
		while (decimalNum < 0 || decimalNum > 255) {
			printf("Program is expecting an int from 0 to 255 inclusive\n"
				  "Please enter a number from 0 to 255: ");
			scanf("%d", &decimalNum);
		}
	}
	printInput = decimalNumCopied = decimalNum;

	while (decimalNum != 0) {
		decimalNum /= 2;
		i++;
	}
	sizeOfArray = i;

	binaryNum = malloc(sizeOfArray * sizeof(int));

#if SHOW_HOW
	printf("decimalNumCopied = %d\n", decimalNumCopied);
#endif

	i--;

#if SHOW_HOW
	printf("i = %d\n", i);
	int count = 0;
#endif

	while (decimalNumCopied != 0)	{

#if SHOW_HOW
		printf("decimalNumcopied = %d\n", decimalNumCopied);
#endif

		binaryNum[i] = decimalNumCopied % 2;

#if SHOW_HOW
		printf("binaryNum[%d] = %d\n", i, binaryNum[i]);
#endif

		decimalNumCopied /= 2;
		i--;

#if SHOW_HOW
		count++;
#endif

	}

#if SHOW_HOW
	printf("Number of times while loop executied = %d\n", count);
#endif

	printf("%d converted to binary is: ", printInput);
	for (i = 0; i < sizeOfArray; i++) {
		printf("%d", binaryNum[i]);
	}
	printf("\n");

	free(binaryNum);
	return 0;
}