Samuel Breslow
Sbreslow

Malloc Readme

Structure of Free Blocks:
My free blocks consist of a WSIZE (4) header and footer with the "payload" inbetween.  The "payload" must be at least DSIZE (8) long (and an integer multiple of DSIZE if it is larger than DSIZE) and contains two WSIZE blocks (located immediately after the header) which contain pointers to the next and previous free blocks respectively (for use in the explicit free list).

Structure of Allocated Blocks:
My allocated blocks have the same structure as the free blocks, without the pointers to the next and previous free blocks in the payload.

Organization of Free Blocks:
My free blocks are organized into a LIFO, explicit doubly-linked free list as mandated by the assignment.  I have two helper methods (LLadd and LLrem) that handle the adding and removing of blocks from the free list respectively.

Manipulation of Free Blocks:
Every time a block is freed (or a new free block is created by extending the heap), the newly freed (or new free) block is coalesced and added to the free list.  Every time a free block is allocated, it is removed from the free list.

Strategy for Maintaining Compaction:
My realloc first checks if the new size is smaller or bigger than the oldsize.  If it is smaller, I truncate the already allocated block and then coalesce the newly freed block before adding it to the explicit free list.  If the new size is bigger, I check for fits if either or both of the adjacent blocks are free.  If there is a fit, I place the new block accordingly, splitting and coalescing if necessary.  If there isn't a fit with either (or both) adjacent blocks, I search the rest of the free list for a new place (first fit search, as mandated by the assignment).  If a fit exists, I copy the memory to it and free the original block before coalescing.  If no fit exists, I call malloc (to extend the heap), copy the contents to the newly allocated block, and then free and coalesce the original block.

General Notes:
My program has no bugs that I know of and returns a 78 when I run the tester.  Nearly all of the tests run reasonably well except for realloc2 (an embarassingly low 9% Util), which I can't for the life of me figure out how to improve without deviating from the mandated LIFO, explicit free list structure (e.g. using segregated/buddy lists, or an explicit free list sorted by address).
