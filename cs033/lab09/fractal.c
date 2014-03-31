#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fractal.h"
#include "image.h"

/*
	generate and save the entire fractal
	returns:	0 if successful
				1 otherwise
*/
int generate_fractal(char *file, float width, float height, int workers) {
	
	color_t *fractal_data = malloc(sizeof(color_t)*width*height);

	/*
		generate data for the entire fractal
	*/
	generate_fractal_region(fractal_data, width, height, 0, height);


	/*
		save the generated fractal data into the file specified by argv[1]
	*/
	if (save_image_data(file, fractal_data, width, height)) {
		fprintf(stderr, "error saving to image file.\n");
		free(fractal_data);
		return 1;
	}

	free(fractal_data);
	
	printf("Complete.\n");

    return 0;
}
