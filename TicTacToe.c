/* A simple Tic Tac Toe game.
 * Adapted from C Reference Manyual, Shildt */
#include <stdio.h>
#include <stdlib.h>

// Prototypes.
char matrix[3][3];
char check();
void initializeMatrix();
void getPlayerMove();
void getComputerMove();
void displayMatrix();

int main() {
	char done = ' ';

	printf("Welcome to Tic Tac Toe.\n");
	printf("You are 'X', playing against the Computer as 'O'.\n");
	printf("These are the coordinates pairs.\n");
	printf("1,1|1,2|1,3");
	printf("\n---|---|---\n");
	printf("2,1|2,2|2,3");
	printf("\n---|---|---\n");
	printf("3,1|3,2|3,3\n");

	initializeMatrix();
	do {
		displayMatrix();
		getPlayerMove();
		// Check for a winner!
		done = check();
		if (done != ' ') {
			break;
		} // We have a winner!
		getComputerMove();
		// Check for a winner!
		done = check();
	} while (done == ' ');

	if (done == 'X')
		printf("The Human has won!\n");
	else
		printf("The Computer has won!\n");
	// Show final positions in matrix.
	displayMatrix();

	return 0;
}

/* Initialize the matrix. */
void initializeMatrix() {
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			matrix[i][j] = ' ';
		}
	}
}

/* Get Player's move. */
void getPlayerMove(){
	int x, y;
	printf("Enter X,Y coordinates for your move with <Enter> after each coordinate: ");
	scanf("%d%*c%d", &x, &y);
	x--;
	y--;
	if (matrix[x][y] != ' ') {
		printf("Invalid move, try again.\n");
		getPlayerMove();
	}
	else matrix[x][y] = 'X';
}


/* Get Computer's move. */
void getComputerMove() {
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (matrix[i][j] == ' ') {
				break;
			}
		}
		if (matrix[i][j] == ' ') {
			break;
		}
	}
	if (i * j == 9) {
		printf("The game is a draw!\n");
		exit(0);
	}
	else
		matrix[i][j] = 'O';
}

/* Display the matrix on the screen. */
void displayMatrix() {
	int t;
	printf("\nCurrent Game!\n");
	for (t = 0; t < 3; t++) {
		printf(" %c | %c | %c ", matrix[t][0], matrix[t][1], matrix[t][2]);
		if (t != 2) {
			printf("\n---|---|---\n");
		}
	}
	printf("\n");
}

/* See if there is a winner. */
char check() {
	int i;
	/* Check rows. */
	for (i = 0; i < 3; i++) {
		if (matrix[i][0] == matrix[i][1] && matrix[i][0] == matrix[i][2]) {
			return matrix[i][0];
		}
	}

	/* Check Columns. */
	for (i = 0; i < 3; i++) {
		if (matrix[0][i] == matrix[1][i] && matrix[0][i] == matrix[2][i]) {
				return matrix[0][i];
			}
		}

	/* Chcek Diagonals. */
	if (matrix[0][0] == matrix[1][1] && matrix[1][1] == matrix[2][2]) {
		return matrix[0][0];
	}
	if (matrix[0][2] == matrix[1][1] && matrix[1][1] == matrix[2][0]) {
		return matrix[0][2];
	}

	return ' ';
}
