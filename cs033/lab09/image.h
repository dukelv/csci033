#ifndef IMAGE_H
#define IMAGE_H

#define DEFAULT_HEIGHT 2000
#define DEFAULT_WIDTH  2000
#define DEFAULT_WORKERS 4
#define SCALE 3.0

#define MAX_ITERATIONS 200


typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} color_t;


/*
    saves the data stored in the image_data array of given width and height into an output .png file
    arguments:	file_name:	the name of the file to save data to
    			image_data:	the color data of the fractal
    			width:		the image width
    			height:		the image height
    returns:    0 on success
                1 otherwise
*/
int save_image_data(char *file_name, color_t *image_data,
	int width, int height);


/*
	generates part of the fractal region, computing data for a number of rows
	beginning from start_y
	
	arguments:	image_data:	a pointer to the saved color data
				width:		the width of the image
				height:		the height of the image
				start_y:	the first row for which the function should generate data
				rows:		the number of rows for which the function should generate data
	returns:	nothing
*/
void generate_fractal_region(color_t *image_data, float width, float height,
	float start_y, unsigned int rows);


#endif
