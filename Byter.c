/******************************************************************************
* Byte.c converts a user supplied int from 0 to 255 inclusive to a byte array
* and prints it.  Input is via user polling or from the command line.
* Compile: gcc -o Byter Byte.c -g -Wall
******************************************************************************/
#include<stdio.h>
#include<stdlib.h>

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


	i--;
	while (decimalNumCopied != 0)	{
		binaryNum[i] = decimalNumCopied % 2;
		decimalNumCopied /= 2;
		i--;
	}

	printf("%d converted to binary is: ", printInput);
	for (i = 0; i < sizeOfArray; i++) {
		printf("%d", binaryNum[i]);
	}
	printf("\n");

	free(binaryNum);
	return 0;
}