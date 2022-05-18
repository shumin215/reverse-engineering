/* 
 * mm.c -  Simple allocator based on implicit free lists, 
 *                  first fit placement, and boundary tag coalescing. 
 *
 * Each block has header of the form:
 * 
 *      31                     3  2  1  0 
 *      -----------------------------------
 *     | s  s  s  s  ... s  s  s  0  0  a/f
 *      ----------------------------------- 
 * 
 * where s are the meaningful size bits and a/f is set 
 * iff the block is allocated. The list has the following form:
 *
 * heap                             heap
 * start                            end  
 *  ---------------------------------
 * |  pad   | zero or more usr blks |
 *  ---------------------------------
 *          |                       |
 *          |                       |
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "mm.h"
#include "memlib.h"

/* Team structure */
team_t team = {
    "implicit first fit", 
    "Dave OHallaron", "droh",
    "", ""
}; 

typedef struct block {
    size_t header;
    unsigned char payload[];
} block_t;

/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE       4       /* word size (bytes) */  
#define DSIZE       8       /* doubleword size (bytes) */
#define OVERHEAD    8       /* overhead of header and footer (bytes) */

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(size_t *)(p))
#define PUT(p, val)  (*(size_t *)(p) = (val))  

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(p)  (block_t *) ((unsigned char *) p - offsetof(block_t, payload))

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(block)  (block_t *) ((unsigned char *) block+ GET_SIZE(block))

/* Get payload pointer */
#define GET_PLD_PTR(block) ((void *) block->payload)
/* $end mallocmacros */

/* Global variables */
static block_t *heap_start; 
static block_t *heap_end; 

/* function prototypes for internal helper routines */
static block_t *extend_heap(size_t words);
static void place(block_t *bp, size_t asize);
static block_t *find_fit(size_t asize);
static void coalesce(block_t *bp);

/* 
 * mm_init - Initialize the memory manager 
 */
int mm_init(void) 
{
    /* create the initial empty heap */
    char *bp = (char *) heap_start;
    if ((bp = mem_sbrk(WSIZE)) == NULL)
	return -1;
    PUT(bp, 0);                        /* alignment padding */
    bp += WSIZE;
    heap_start = (block_t *) bp;
    heap_end = heap_start;
    return 0;
}

/* 
 * mm_malloc - Allocate a block with at least size bytes of payload 
 */
void *mm_malloc(size_t size) 
{
    size_t asize;      /* adjusted block size */
    block_t *bp;      

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = DSIZE + OVERHEAD;
    else
        asize = OVERHEAD + ((size + (DSIZE - 1)) / DSIZE) * DSIZE;
    
    /* TODO: Search the free list for a fit */
    bp = find_fit(asize);
    if (bp != NULL) {
        place(bp, asize);
        return GET_PLD_PTR(bp);
    }
    /* TODO: No fit found. Get more memory and place the block */
    if ((bp = extend_heap(asize / WSIZE)) == NULL) {
        exit(1);
    }

    place(bp, asize);
    return GET_PLD_PTR(bp);
} 

/* 
 * mm_free - Free a block 
 */
void mm_free(void *bp)
{
    /* TODO */
    block_t *blockp = HDRP(bp);
    size_t size = GET_SIZE(blockp);
    PUT(blockp, PACK(size, 0));
    coalesce(blockp);
}

/*
 * mm_realloc - naive implementation of mm_realloc
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *newp;
    size_t copySize;

    if ((newp = mm_malloc(size)) == NULL) {
	printf("ERROR: mm_malloc failed in mm_realloc\n");
	exit(1);
    }
    copySize = GET_SIZE(HDRP(ptr));
    if (size < copySize)
      copySize = size;
    memcpy(newp, ptr, copySize);
    mm_free(ptr);
    return newp;
}

/* The remaining routines are internal helper routines */

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
static block_t *extend_heap(size_t words) 
{
    block_t *bp;
    size_t size;
	
    /* TODO: Allocate an even number of words to maintain 8-byte alignment */
    if (words % 2 == 1) {
        words += 1;
    }
    size = words * WSIZE;
    if ((bp = mem_sbrk(size)) == NULL) {
        return NULL;
    }

    /* Update heap end */
    char *tmp = (char *) heap_end;
    tmp += size;
    heap_end = (block_t *) tmp;

    return bp;
}

/* 
 * place - Place block of asize bytes at start of free block bp 
 *         and split if remainder would be at least minimum block size (16 bytes)
 *
 *         like split function
 */
static void place(block_t *bp, size_t asize)
{
    /* TODO */
    size_t csize = GET_SIZE(bp);
    if (csize - asize >= 16) {
        PUT(bp, PACK(asize, 1));
        block_t *next_bp = NEXT_BLKP(bp);
        PUT(next_bp, PACK(csize - asize, 0));
    } else {
        PUT(bp, PACK(csize, 1));
    }
}

/* 
 * find_fit - Find a fit for a block with asize bytes 
 */
static block_t *find_fit(size_t asize)
{
    /* TODO: first fit search */
    block_t *bp;
    for (bp = heap_start; bp != heap_end; bp=NEXT_BLKP(bp)) {
        if (!GET_ALLOC(bp) && GET_SIZE(bp) >= asize) {
            return bp;
        }
    }
    return NULL;
}

/*
 * coalesce, namely merge 
 */
static void coalesce(block_t *bp) 
{
    /* TODO */
    block_t *next_block = NEXT_BLKP(bp);
    if (next_block < heap_end && !GET_ALLOC(bp) && !GET_ALLOC(next_block)) {
        size_t size = GET_SIZE(bp) + GET_SIZE(next_block);
        size_t header = PACK(size, 0);
        PUT(bp, header);
    }
}
