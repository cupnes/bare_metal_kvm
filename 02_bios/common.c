#include <stdio.h>
#include <stdlib.h>

void assert(unsigned char condition, char *msg)
{
	if (!condition) {
		perror(msg);
		exit(EXIT_FAILURE);
	}
}
