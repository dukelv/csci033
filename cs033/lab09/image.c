#include <stdio.h>
#include <math.h>
#include <gd.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include "image.h"
#include "colors.h"

/*
    saves the data stored in the image_data array of given width and height into an output .png file
    returns:    0 on success
                1 otherwise
*/
int save_image_data(char *file_name, color_t *imageData,
                            int width, int height) {

    gdImagePtr img = gdImageCreateTrueColor(width, height);

    unsigned x, y;
    for(y = 0; y < height; ++y) {
        for(x = 0; x < width; ++x) {

            int index = y*width + x;
            int gd_color = gdImageColorAllocate(img,
                                            imageData[index].r,
                                            imageData[index].g,
                                            imageData[index].b);
            gdImageSetPixel(img, x, y, gd_color);
        }
    }

    FILE *img_file = fopen(file_name, "w+");
    if (img_file == NULL) {
        gdImageDestroy(img);
        perror("error opening file");
        return 1;
    }

    gdImagePng(img, img_file);
    gdImageDestroy(img);

    if (fclose(img_file)) {
        perror("error closing file");
        return 1;
    }

    return 0;
}




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
void generate_fractal_region(color_t *image_data, float width, float height, float start_y, unsigned int rows) {
	unsigned int y_pixel, x_pixel;
    for (y_pixel = start_y; y_pixel < start_y + rows; y_pixel++)  {
        for (x_pixel = 0; x_pixel < width; x_pixel++)  {
            float cx = (x_pixel / width * SCALE) - 1.5;
            float cy = ((y_pixel / height) * SCALE) - 2.2;

            float x = 0;
            float y = 0;

            unsigned int i;
            for (i = 0; i < MAX_ITERATIONS && (x*x + y*y <= 4.0); i++) {
                float x_temp = x*x - y*y + cy;
                y = 2.0 * x * y + cx;
                x = x_temp;
            }

            float ratio = ((float)i)/MAX_ITERATIONS;
            int index = (int)(y_pixel * width + x_pixel);

            image_data[index].r = generate_red(ratio);
            image_data[index].g = generate_green(ratio);
            image_data[index].b = generate_blue(ratio);
        }
    }
}
