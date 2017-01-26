// Returns the length of the longest palindromic subsequence in input string.
// Compile: gcc palSeq.cpp -o palSeq -g -Wall
#include<stdio.h>
#include<string.h>

// Prototypes.
int max(int, int);
int longestPalsequence(char *);

int main() {
	char seq[] = "ACGTGTCAAAATCG";		// Input string.
	int n = strlen(seq);
	printf("The length of n via strlen() is %d\n", n);
	printf("The length of the Longest Palindromic Sequence is %d\n", longestPalsequence(seq));
	printf("Which of course is ACGTGTCA\n");
	return 0;
}

// Returns the length of the longest palindromic subsequence (in table[][]).
int longestPalsequence(char *str) {
	int n = strlen(str);
	int i, j;
	int lenSS;		// Length of substring.
	int table[n][n];	// Create a table to store subproblems results.

    // Length 1 string is palindrome of length 1.
	for (i = 0; i < n; i++) {
		table[i][i] = 1;
	}

	// Build the table.
	for (lenSS = 2; lenSS <= n; lenSS++) {
		for (i = 0; i < n - lenSS + 1; i++) {
			j = i + lenSS - 1;
			if (str[i] == str[j] && lenSS == 2) {
				table[i][j] = 2;
			}
			else if (str[i] == str[j]) {
				table[i][j] = table[i + 1][j - 1] + 2;
			}
			else {
				table[i][j] = max(table[i][j - 1], table[i + 1][j]);
			}
		}
	}
	return table[0][n - 1];
}

// Returns max of two ints.
int max(int x, int y) {
	return (x > y) ? x : y;
}