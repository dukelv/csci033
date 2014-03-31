#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fractal.h"
#include "image.h"


/*
	parse input for arguments and then call generate_fractal() with those arguments
*/
int main(int argc, char **argv) {
	char *file_name = "out.png";
	float width = DEFAULT_WIDTH;
	float height = DEFAULT_HEIGHT;
	int workers = DEFAULT_WORKERS;
	
	unsigned int arg;
	for (arg = 1; arg < argc; arg++) {
		if (!strcmp(argv[arg], "-f") || !strcmp(argv[arg], "--file")) {
			arg++;
			if (arg < argc)
				file_name = argv[arg];
			else {
				fprintf(stderr, "error: usage ./fractal [-s <scale>] [-f <file>] [-d <width> <height>]\n");
				exit(1);
			}
		}
		else if (!strcmp(argv[arg], "-n") || !strcmp(argv[arg], "--workers")) {
			arg++;
			if (arg < argc)
				workers = atoi(argv[arg]);
			else {
				fprintf(stderr, "error: usage ./fractal [-s <scale>] [-f <file>] [-d <width> <height>]\n");
				exit(1);
			}
		}
		else if (!strcmp(argv[arg], "-d") || !strcmp(argv[arg], "--dimensions")) {
			if (arg + 2 < argc) {
				width = strtof(argv[arg + 1], NULL);
				height = strtof(argv[arg + 2], NULL);
			}
			else {
				fprintf(stderr, "error: usage ./fractal [-s <scale>] [-f <file>] [-d <width> <height>]\n");
				exit(1);
			}

			arg += 2;
		}
		else {
			fprintf(stderr, "error: usage ./fractal [-s <scale>] [-f <file>] [-d <width> <height>]\n");
			exit(1);
		}
	}
	
	return generate_fractal(file_name, width, height, workers);
}
