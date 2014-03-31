#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"


/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros: */
#define WSIZE      4 /* Word and header/footer size (bytes) */
#define DSIZE      (2 * WSIZE)    /* Doubleword size (bytes) */
#define CHUNKSIZE  (1 << 9)      /* Extend heap by this amount (bytes) */

#define MAX(x, y)  ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word. */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p. */
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p. */
#define GET_SIZE(p)   (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)  (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer. */
#define HDRP(bp)  ((char *)(bp) - WSIZE)
#define FTRP(bp)  ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of its "next address" and "previous address". */
#define NEXT(bp)  ((unsigned int)(bp)) /*NEXT(bp) returns address of where address of next free block is stored*/
#define PREV(bp)  ((unsigned int)(bp + WSIZE))

/* Given block ptr bp, compute and set address of its "next" and "previous" free block. */
#define GET_NEXT(bp) (GET(bp)) /*GET_NEXT(bp) returns address of next free block*/
#define GET_PREV(bp) (GET(bp + WSIZE))
#define SET_NEXT(bp, val)  (PUT(bp, (unsigned int)val)) /*SET_NEXT(bp, ptr) sets the address of next free block*/
#define SET_PREV(bp, val)  (PUT((bp + WSIZE), (unsigned int)val))


/* Given block ptr bp, compute address of next and previous blocks. */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

static char *heap_listp; /* Pointer to first block */
static char *free_list;

static void *coalesce(void *bp);
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
void LLadd(char *bp);
void LLrem(char *bp);
/*
 * initializes the dynamic allocator
 * arguments: none
 * returns: 0, if successful
 *          1, if an error occurs
 */
int mm_init(void) {
    if ((heap_listp = mem_sbrk(6 * WSIZE)) == (void *)-1)
    {
	return (-1);
    }
    PUT(heap_listp, 0);                            /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(2*DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2 * WSIZE), 0);              /*next address is null*/
    PUT(heap_listp + (3 * WSIZE), 0);              /*prev address is null*/
    PUT(heap_listp + (4 * WSIZE), PACK(2*DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (5 * WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2 * WSIZE);
    free_list = heap_listp;
    /* Extend the empty heap with a free block of CHUNKSIZE bytes. */
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
    {
	return (-1);
    }
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
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    void *bp;
    /* Ignore spurious requests. */
    if (size == 0)
    {
	return (NULL);
    }
    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
    {
	asize = 2 * DSIZE;
    }
    else
    {
	asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);
    }
    /* Search the free list for a fit. */
    if ((bp = find_fit(asize)) != NULL)
    {
	place(bp, asize);
	return (bp);
    }
    /* No fit found.  Get more memory and place the block. */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
    {
	return (NULL);
    }
    place(bp, asize);
    return (bp);
}





/*
 * frees a block of memory, enabling it to be reused later
 * arguments: ptr: the allocated block to free
 * returns: nothing
 */
void mm_free(void *ptr) {
    size_t size;
    /* Ignore spurious requests. */
    if (ptr == NULL)
    {
	return;
    }
    /* Free and coalesce the block. */
    size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}




/*
 * reallocates a memory block to update it with the given size
 * arguments: ptr: a pointer to the memory block
 *            size: the desired new block size
 * returns: a pointer to the new memory block
 */
void *mm_realloc(void *ptr, size_t size) {
    void *loc;
    if (size == 0)
    {
	mm_free(ptr);
	return (NULL);
    }
    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
    {
	return (mm_malloc(size));
    }
    size_t oldsize = GET_SIZE(HDRP(ptr));
    size_t asize;
    if (size <= DSIZE)
    {
	asize = 2 * DSIZE;
    }
    else
    {
	asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);
    }
    if(oldsize > asize)// truncate and add new free block to LL, NEVER CALLED
    {
	PUT(HDRP(ptr), PACK(oldsize,0));
	PUT(FTRP(ptr), PACK(oldsize,0));
	place(ptr,asize);
	return ptr;
    }
    else if(oldsize < asize)// find new fit
    {
	bool prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr)));
	bool next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
	if (prev_alloc && next_alloc)
	{
	    loc = find_fit(asize);
	    if(loc!=NULL)
	    {
		place(loc, asize);
		memcpy(loc, ptr, oldsize);
		mm_free(ptr);
		return loc;
	    }
	    else
	    {
		void *newptr = mm_malloc(size);
		memcpy(newptr, ptr, oldsize);
		mm_free(ptr);
		return (newptr);
	    }
	}
	else if (!prev_alloc && next_alloc) 
	{
	    size_t psize = GET_SIZE(HDRP(PREV_BLKP(ptr)));
	    if((psize+oldsize)>=asize)
	    {
		LLrem(PREV_BLKP(ptr));
		PUT(FTRP(ptr), PACK(psize+oldsize, 1));
		PUT(HDRP(PREV_BLKP(ptr)), PACK(psize+oldsize, 1));
		void *bp = PREV_BLKP(ptr);
		memcpy(bp,ptr,oldsize);
		place(bp,asize);
		return bp;
	    }
	    else
	    {
		loc = find_fit(asize);
		if(loc!=NULL)
		{
		    place(loc, asize);
		    memcpy(loc, ptr, oldsize);
		    mm_free(ptr);
		    return loc;
		}
		else
		{
		    void *newptr = mm_malloc(size);
		    memcpy(newptr, ptr, oldsize);
		    mm_free(ptr);
		    return (newptr);
		}
	    }
	}
	else if (prev_alloc && !next_alloc) 
	{
	    size_t nsize = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
	    if((nsize+oldsize)>=asize)
	    {
		LLrem(NEXT_BLKP(ptr));
		PUT(HDRP(ptr), PACK(nsize+oldsize, 1));
		PUT(FTRP(ptr), PACK(nsize+oldsize, 1));
		place(ptr,asize);
		return ptr;
	    }
	    else
	    {
		loc = find_fit(asize);
		if(loc!=NULL)
		{
		    place(loc, asize);
		    memcpy(loc, ptr, oldsize);
		    mm_free(ptr);
		    return loc;
		}
		else
		{
		    void *newptr = mm_malloc(size);
		    memcpy(newptr, ptr, oldsize);
		    mm_free(ptr);
		    return (newptr);
		}
	    }
	}
	else
	{
	    size_t psize = GET_SIZE(HDRP(PREV_BLKP(ptr)));
	    size_t nsize = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
	    if((nsize+oldsize)>=asize)
	    {
		LLrem(NEXT_BLKP(ptr));
		PUT(HDRP(ptr), PACK(nsize+oldsize, 1));
		PUT(FTRP(ptr), PACK(nsize+oldsize, 1));
		place(ptr,asize);
		return ptr;
	    }
	    else if((psize+oldsize)>=asize)
	    {
		LLrem(PREV_BLKP(ptr));
		PUT(FTRP(ptr), PACK(psize+oldsize, 1));
		PUT(HDRP(PREV_BLKP(ptr)), PACK(psize+oldsize, 1));
		void *bp = PREV_BLKP(ptr);
		memcpy(bp,ptr,oldsize);
		place(bp,asize);
		return bp;
	    }
	    else if((psize+oldsize+nsize)>=asize)//NEVER CALLED
	    {
		LLrem(PREV_BLKP(ptr));
		LLrem(NEXT_BLKP(ptr));
		PUT(HDRP(PREV_BLKP(ptr)), PACK(oldsize+psize+nsize, 0));
		PUT(FTRP(NEXT_BLKP(ptr)), PACK(oldsize+psize+nsize, 0));
		void* bp = PREV_BLKP(ptr); //pointer to start of big block
		memcpy(bp,ptr,oldsize); //copy memory into top of block
		place(bp,asize);
		return bp;
	    }
	    else
	    {
		loc = find_fit(asize);
		if(loc!=NULL)
		{
		    place(loc, asize);
		    memcpy(loc, ptr, oldsize);
		    mm_free(ptr);
		    return loc;
		}
		else
		{
		    void *newptr = mm_malloc(size);
		    memcpy(newptr, ptr, oldsize);
		    mm_free(ptr);
		    return (newptr);
		}
	    }
	}
    }
    else
    {
	return ptr;
    }
}

/*my helper functions*/
static void *extend_heap(size_t words)
{
    void *bp;
    size_t size;
    /* Allocate an even number of words to maintain alignment. */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((bp = mem_sbrk(size)) == (void *)-1)
    {
	return (NULL);
    }
    /* Initialize free block header/footer and the epilogue header. */
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */
    PUT(NEXT(bp), 0);  /*next address is null*/
    PUT(PREV(bp), 0);  /*prev address is null*/
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */
    /* Coalesce if the previous block was free. */
    return (coalesce(bp));
}

static void *find_fit(size_t asize)
{
    void *bp = free_list;
    /* Search for the first fit. */
    while(GET_NEXT(bp)!=0)
    {
	if(asize <= GET_SIZE(HDRP(bp)))
	{
	    return bp;
	}
	bp = (void *)GET_NEXT(bp);
    }
    /* No fit was found. */
    return (NULL);
}

static void *coalesce(void *bp) 
{
    bool prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    bool next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    if (prev_alloc && next_alloc)
    {
	LLadd(bp);/* Case 1 */
	return (bp);
    }
    else if (prev_alloc && !next_alloc) 
    {         /* Case 2 */
	LLrem(NEXT_BLKP(bp));
	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
    } 
    else if (!prev_alloc && next_alloc) 
    {         /* Case 3 */
	LLrem(PREV_BLKP(bp));
	size += GET_SIZE(HDRP(PREV_BLKP(bp)));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    } 
    else
    {                                        /* Case 4 */
	LLrem(PREV_BLKP(bp));
	LLrem(NEXT_BLKP(bp));
	size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    }
    LLadd(bp);
    return (bp);
}

static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    if ((csize - asize) >= (2 * DSIZE))
    {
	LLrem(bp);
	PUT(HDRP(bp), PACK(asize, 1));
	PUT(FTRP(bp), PACK(asize, 1));
	bp = NEXT_BLKP(bp);
	PUT(HDRP(bp), PACK(csize - asize, 0));
	PUT(FTRP(bp), PACK(csize - asize, 0));
	coalesce(bp);
    }
    else
    {
	LLrem(bp);
	PUT(HDRP(bp), PACK(csize, 1));
	PUT(FTRP(bp), PACK(csize, 1));
    }
}

void LLadd(char *bp)
{
    char *head = free_list;
    SET_NEXT(bp, head);
    SET_PREV(bp, NULL);
    if(head != NULL)
    {
	SET_PREV(head, bp);
    }
    free_list = bp;
}

void LLrem(char *bp)
{
    unsigned int next = GET_NEXT(bp);
    unsigned int prev = GET_PREV(bp);
    if(prev == 0)// bp is head of list
    {
	free_list = (char *) next;
	if(next != 0)
	{
	    SET_PREV((char *)next, NULL);
	}
    }
    else
    {
	SET_NEXT((char *) prev, next);
	if(next != 0)
	{
	    SET_PREV((char *) next, prev);
	}
    }
}