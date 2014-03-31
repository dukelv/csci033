/*
 * mm-naive.c - A fast, memory-inefficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"


/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define IGNORE(value) (assert((int)value || 1))


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*
 * initializes the dynamic allocator.
 * In this implementation, this does nothing.
 * arguments: none
 * returns: 0, if successful
 *          1, if an error occurs
 */
int mm_init(void) {
	return 0;
}




/*
 * allocates a block of memory and returns a pointer to that block
 * arguments: size: the desired block size
 * returns: a pointer to the newly-allocated block (whose size
 *          is a multiple of ALIGNMENT), or NULL if an error
 *          occurred
 */
void *mm_malloc(size_t size) {
	int newsize = ALIGN(size + SIZE_T_SIZE);
	void *p = mem_sbrk(newsize);
	if (p == (void *)-1)
		return NULL;
	else {
		*(size_t *)p = size;
		return (void *)((char *)p + SIZE_T_SIZE);
	}
}




/*
 * "frees" a block of memory, enabling it to be reused later.
 * In this implementation, this actually does nothing.
 * arguments: ptr: the allocated block to free
 * returns: nothing
 */
void mm_free(void *ptr) {
	IGNORE(ptr);
}




/*
 * reallocates a memory block to update it with the given size
 * arguments: ptr: a pointer to the memory block
 *            size: the desired new block size
 * returns: a pointer to the new memory block
 */
void *mm_realloc(void *ptr, size_t size) {
	void *oldptr = ptr;
	void *newptr;
	size_t copySize;
	
	newptr = mm_malloc(size);
	if (newptr == NULL)
		return NULL;
	copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
	if (size < copySize)
		copySize = size;
	memcpy(newptr, oldptr, copySize);
	mm_free(oldptr);
	return newptr;
}
