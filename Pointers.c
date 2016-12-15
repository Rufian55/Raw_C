/* Pointer Demo Program.
 * Adapted from C Complete Reference Manual, Shildt */
#include <stdio.h>
#include <stdlib.h>

int main() {
	double x = 100.1, y;
	int *p;

	/* The next statement causes p (which is an
	integer pointer) to point to a double. */
	p = (int *)&x;

	/* The next statement does not operate as expected.
	   Attempt to assign y the value x through p.  
	   Since ints ar 4 bytes, doubles are 8 bytes, some
	   data is lost on the assignement. */
	y = *p; 

	// The following statement won't output 100.1.
	printf("The (incorrect) value of x is: %f\n", y);
	
	return 0;
}
