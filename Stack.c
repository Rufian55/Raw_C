/* Simple Stack Demo Program.
 * Adapted from C Complete Reference Manual, Shildt
 * See DataStructures Repositry for more advanced stacks.
 */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 50

// Prototypes.
void push(int i);
int pop();
int *tos, *p1, stack[SIZE];

int main() {
	int value;
	tos = stack;	// tos points to the top of stack.
	p1 = stack;	// Initialize p1.
	do {
		printf("Enter value: ");
		scanf("%d", &value);
		if (value != 0) {
			push(value);
		}
		else {
			printf("value on top is %d\n", pop());
		}
	} while (value != -1);
	return 0;
}

void push(int i) {
	p1++;
	if (p1 == (tos + SIZE)) {
		printf("Stack Overflow.\n");
		exit(1);
	}
	*p1 = i;
}

int pop() {
	if (p1 == tos) {
		printf("Stack Underflow.\n");
		exit(1);
	}
	p1--;
	return *(p1 + 1);
}
