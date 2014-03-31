#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "fractal.h"
#include "image.h"


/*
	generate and save the entire fractal
	returns:	0 if successful
				1 otherwise
*/
int generate_fractal(char *file, float width, float height, int workers) {
	/*	TODO: use fork(), mmap(), and munmap() to create a region of shared memory
			to store fractal data, as generated by several child processes	*/
	int fd = open(file,O_RDONLY);
	color_t *fractal_data = mmap(NULL,sizeof(color_t)*height*width,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_SHARED,fd,0);
	int i,status;
	int pids[workers];
	for(i = 0; i < workers; i++)
	{
	    if((pids[i]=fork())==0)
	    {
		generate_fractal_region(fractal_data, width, height, i*(height/workers), (height/workers));
		munmap(fractal_data,sizeof(color_t)*height*width);
		exit(0);
	    }
	}
	for(i = 0; i < workers; i++)
	{
	    waitpid(pids[i],&status,0);
	}
	if (save_image_data(file, fractal_data, width, height)) {
		fprintf(stderr, "error saving to image file.\n");
		free(fractal_data);
		return 1;
	}
	munmap(fractal_data,sizeof(color_t)*height*width);
	printf("Complete.\n");
	return 0;
}