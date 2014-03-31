#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"


/***************
 * ROTATE KERNEL
 ***************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
    {
		for (j = 0; j < dim; j++)
		{
	    	dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
	    }
	}
}

char rotate_c_descr[] = "rotate_c: Naive col-wise traversal of src";
void rotate_c(int dim, pixel *src, pixel *dst) {
    int i, j;

    for(j=0; j < dim; j++) 
    {
		for(i=0; i < dim; i++) {
			dst[RIDX(dim-1-j,i,dim)] = src[RIDX(i,j,dim)];
		}
    }

    return;
}


char rotate_b16_descr[] = "rotate_b16: Rotate using 16x16 blocking";
void rotate_b16(int dim, pixel *src, pixel *dst) {
    int i, j;
    int ii, jj;

    for(ii=0; ii < dim; ii+=16)
    {
		for(jj=0; jj < dim; jj+=16)
		{
			for(i=ii; i < ii+16; i++) 
			{
				for(j=jj; j < jj+16; j++) 
				{
					dst[RIDX(dim-1-j,i,dim)] = src[RIDX(i,j,dim)];
				}
			}
		}
	}

    return;
}

char rotate_u16_descr[] = "rotate_u16: Rotate using 16x unrolling";
void rotate_u16(int dim, pixel *src, pixel *dst) {
    int i, j;

    for(i=0; i < dim; i++)
    {
		for(j=0; j < dim; j+=16) {
			dst[RIDX(dim-1-j,i,dim)] = src[RIDX(i,j,dim)];
			dst[RIDX(dim-1-j-1,i,dim)] = src[RIDX(i,j+1,dim)];
			dst[RIDX(dim-1-j-2,i,dim)] = src[RIDX(i,j+2,dim)];
			dst[RIDX(dim-1-j-3,i,dim)] = src[RIDX(i,j+3,dim)];
			dst[RIDX(dim-1-j-4,i,dim)] = src[RIDX(i,j+4,dim)];
			dst[RIDX(dim-1-j-5,i,dim)] = src[RIDX(i,j+5,dim)];
			dst[RIDX(dim-1-j-6,i,dim)] = src[RIDX(i,j+6,dim)];
			dst[RIDX(dim-1-j-7,i,dim)] = src[RIDX(i,j+7,dim)];
			dst[RIDX(dim-1-j-8,i,dim)] = src[RIDX(i,j+8,dim)];
			dst[RIDX(dim-1-j-9,i,dim)] = src[RIDX(i,j+9,dim)];
			dst[RIDX(dim-1-j-10,i,dim)] = src[RIDX(i,j+10,dim)];
			dst[RIDX(dim-1-j-11,i,dim)] = src[RIDX(i,j+11,dim)];
			dst[RIDX(dim-1-j-12,i,dim)] = src[RIDX(i,j+12,dim)];
			dst[RIDX(dim-1-j-13,i,dim)] = src[RIDX(i,j+13,dim)];
			dst[RIDX(dim-1-j-14,i,dim)] = src[RIDX(i,j+14,dim)];
			dst[RIDX(dim-1-j-15,i,dim)] = src[RIDX(i,j+15,dim)];
		}
	}

    return;
}

char rotate_rec_descr[] = "rotate_rec: Rotate using recursion";

void rotate_rec_helper(int dim, int subsize, pixel *src, pixel *dst);

void rotate_rec(int dim, pixel *src, pixel *dst)
{
	rotate_rec_helper(dim, dim, src, dst);
}

void rotate_rec_helper(int dim, int subsize, pixel *src, pixel *dst) 
{
	if (subsize <= 16) 
	{
		int i, j;
		for (i = 0; i < subsize; i++)
		{
			for (j = 0; j < subsize; j++)
			{
				dst[RIDX(subsize-1-j, i, dim)] = src[RIDX(i, j, dim)];
			}
		}
    } 
    else {
		int newSubsize = subsize >> 1;
		rotate_rec_helper(dim, newSubsize, src, dst+RIDX(newSubsize,0,dim));
		rotate_rec_helper(dim, newSubsize, src+RIDX(0,newSubsize,dim), dst);
		rotate_rec_helper(dim, newSubsize, src+RIDX(newSubsize,0,dim), dst+RIDX(newSubsize,newSubsize,dim));
		rotate_rec_helper(dim, newSubsize, src+RIDX(newSubsize,newSubsize,dim), dst+RIDX(0,newSubsize,dim));
    }
}

void rotate(int dim, pixel *src, pixel *dst)
{
	naive_rotate(dim, src, dst);
}

/*********************************************************************
 * register_rotate_functions - Registers all versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() {
    add_rotate_function(&naive_rotate, naive_rotate_descr);
    add_rotate_function(&rotate_c, rotate_c_descr);
    add_rotate_function(&rotate_b16, rotate_b16_descr);
    add_rotate_function(&rotate_u16, rotate_u16_descr);
	add_rotate_function(&rotate_rec, rotate_rec_descr);
}


/***************
 * Invert KERNEL
 ***************/
 
 /*
 * invert_pixel - inverts each color of a pixel (by subtracting original value from 255)
 */
pixel invert_pixel(pixel p) {
	pixel p2;
	if (p.red > p.green) {
		p2.blue = p.red;
	} else {
		p2.blue = p.green;
	}
    
	p2.red = 0;
	p2.green = 0;
    
	return p2;
}

/*
 * naive_invert - The naive baseline version of invert
 */
char naive_invert_descr[] = "naive_invert: Naive baseline implementation";
void naive_invert(int dim, pixel *src, pixel *dst) {
    int i, j;
    
    for (j = 0; j < dim; j++)
    {
		for (i = 0; i < dim; i++)
		{
			dst[RIDX(i, j, dim)] = invert_pixel(src[RIDX(i, j, dim)]);
		}
	}
}

char invert_fast_descr[] = "invert: Invert with ternary expression and inlining";
static pixel invert_pixel_optimized(pixel p);
void invert_fast(int dim, pixel *src, pixel *dst) 
{
    int i, j;
    
    for (j = 0; j < dim; j++)
    {
		for (i = 0; i < dim; i++)
		{
			dst[RIDX(i, j, dim)] = invert_pixel_optimized(src[RIDX(i, j, dim)]);
		}
	}
}

static inline pixel invert_pixel_optimized(pixel p) {
	pixel p2;
	p2.blue = (p.red > p.green) ? p.red : p.green;
    
	p2.red = 0;
	p2.green = 0;
    
	return p2;
}

void invert(int dim, pixel *src, pixel *dst) 
{
	naive_invert(dim, src, dst);
}

/*********************************************************************
 * register_invert_functions - Registers all versions
 *     of the invert kernel with the driver by calling the
 *     add_invert_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.
 *********************************************************************/

void register_invert_functions() {
    add_invert_function(&naive_invert, naive_invert_descr);
    add_invert_function(&invert_fast, invert_fast_descr);
}

/***************
 * Reverse KERNEL
 ***************/
 
 /*
 * reverse_string - reverses the letters in the string
 */
char naive_reverse_descr[] = "naive_reverse: Naive baseline implementation";
int stringLen(char* string);
void naive_reverse(char* string) {
    int i;
    
    for(i = 0; i <= (stringLen(string)-1)/2; i++)
    {
    	char temp = string[i];
    	string[i] = string[stringLen(string)-1-i];
    	string[stringLen(string)-1-i] = temp;
    }
}

char reverse_fast_descr[] = "reverse_fast: Reverse with loop-invariant code motion";
void reverse_fast(char* string) {
    int i;
    
    int len = stringLen(string) - 1;
    for(i = 0; i < len/2; i++)
    {
    	char temp = string[i];
    	string[i] = string[len - i];
    	string[len - i] = temp;
    }
}

void reverse(char* string)
{
	naive_reverse(string);
}

int stringLen(char* string)
{
	int len = 0;
	while(string[len] != '\0')
	{
		len++;
	}
	return len;
}

/*********************************************************************
 * register_reverse_functions - Registers all versions
 *     of the reverse kernel with the driver by calling the
 *     add_reverse_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.
 *********************************************************************/

void register_reverse_functions() {
    add_reverse_function(&naive_reverse, naive_reverse_descr);
    add_reverse_function(&reverse_fast, reverse_fast_descr);
}
 
