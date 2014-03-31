#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "fractal.h"
#include "image.h"

/*	TODO: define a struct which wraps the arguments to generate_fractal_region()	*/
typedef struct {
    color_t *ptr;
    float width;
    float height;
    float start_y;
    unsigned int rows;
} info;


/*
	generates part of the fractal region, computing data for a number of rows
	beginning from start_y
	
	arguments:	data: a struct wrapping the data needed to generated the fractal
					- a pointer to the color data region of memory
					- the width of the image
					- the height of the image
					- the starting row for filling out data
					- the number of rows to fill out
	returns:	NULL
*/
void *gen_fractal_region(void *data) {
	/*	TODO: unpack the data struct and call generate_fractal_region()	*/
	info *data1 = data;
	generate_fractal_region(data1->ptr,data1->width,data1->height,data1->start_y,data1->rows);

	return NULL;
}



int generate_fractal(char *file, float width, float height, int workers) {
	/*	TODO: initialize several threads which will compute a unique
			region of the fractal data, and have them each execute
			gen_fractal_region().	*/
	int i;
	info data[workers];
	pthread_t threads[workers];
	color_t *fractal_data = malloc(sizeof(color_t)*width*height);
	for(i = 0; i < workers; i++)
	{
	    data[i].ptr = fractal_data;
	    data[i].width = width;
	    data[i].height = height;
	    data[i].start_y = i*(height/workers);
	    data[i].rows = (height/workers);
	    pthread_create(&threads[i], 0, gen_fractal_region, &data[i]);
	}
	for(i = 0; i < workers; i++)
	{
	    pthread_join(threads[i],0);
	}
	if (save_image_data(file, fractal_data, width, height)) {
		fprintf(stderr, "error saving to image file.\n");
		free(fractal_data);
		return 1;
	}
	free(fractal_data);
	printf("Complete.\n");
	return 0;
}
