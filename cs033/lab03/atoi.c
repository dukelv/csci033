#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "my_atoi.h"

#define BUFSIZE 256

// Enter CTRL-D to quit, otherwise enter whatever input you want piped to your program.

int main(int argc, char** argv) {
	char buffer[BUFSIZE], *ret;
	printf("Enter an integer, or CTRL-D to quit: ");
	while((ret = fgets(buffer, BUFSIZE, stdin))) { // read and wait for EOF
		int len = strlen(buffer);
		if(buffer[len-1] == '\n')
			buffer[len-1] = '\0';

		printf("atoi(\"%s\") returned %d\n", buffer, my_atoi(buffer));
		printf("Enter an integer, or CTRL-D to quit: ");
	}
	return 0;
}
