/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * Explicit free list + segregrated fits
 */
#include "mm.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"

team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "123",
    /* First member's email address */
    "123@123.123",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* Basic constants and macros */
#define WSIZE 4             /* Word and header/footer size (bytes) */
#define DSIZE 8             /* Double word size (bytes) */
#define CHUNKSIZE 4800      /* Extend heap by this amount (bytes) */
#define MINSIZE (3 * DSIZE) /* Minimum size block*/
#define MAXSIZE (1 << 20)   /* Maximum size block*/

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (unsigned int)(val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define PRED(bp) ((char *)(bp))
#define SUCC(bp) ((char *)(bp) + WSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

static char *root;
static char *head;
static char *tail;
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static int mm_check(void);
static void inser(void *bp);
static void remov(void *bp);
static void insertAppr(void *bp);
static void *check_list(size_t asize);
static void *realloc_coalesce(void *bp, size_t asize);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
  size_t size = 2 * DSIZE;
  if ((head = mem_sbrk(0)) == (void *)-1) {
    return -1;
  }
  root = head + WSIZE;
  for (size_t i = MINSIZE; i < 1024; i += DSIZE) {
    /* Create the initial empty heap */
    if ((head = mem_sbrk(8 * WSIZE)) == (void *)-1) {
      return -1;
    }
    head = head + WSIZE;
    tail = head + 4 * WSIZE;
    PUT(HDRP(head), PACK(size, 1)); /* head header */
    PUT(PRED(head), tail);          /* head pred */
    PUT(SUCC(head), tail);          /* head succ */
    PUT(FTRP(head), PACK(size, 1)); /* head footer */
    PUT(HDRP(tail), PACK(size, 1)); /* tail header */
    PUT(PRED(tail), head);          /* tail pred */
    PUT(SUCC(tail), head);          /* tail succ */
    PUT(FTRP(tail), PACK(size, 1)); /* tail footer */
  }
  for (size_t i = 1024; i <= MAXSIZE; i *= 2) {
    /* Create the initial empty heap */
    if ((head = mem_sbrk(8 * WSIZE)) == (void *)-1) {
      return -1;
    }
    head = head + WSIZE;
    tail = head + 4 * WSIZE;
    PUT(HDRP(head), PACK(size, 1)); /* head header */
    PUT(PRED(head), tail);          /* head pred */
    PUT(SUCC(head), tail);          /* head succ */
    PUT(FTRP(head), PACK(size, 1)); /* head footer */
    PUT(HDRP(tail), PACK(size, 1)); /* tail header */
    PUT(PRED(tail), head);          /* tail pred */
    PUT(SUCC(tail), head);          /* tail succ */
    PUT(FTRP(tail), PACK(size, 1)); /* tail footer */
  }
  if ((head = mem_sbrk(4 * WSIZE)) == (void *)-1) {
    return -1;
  }
  char *prol = (char *)tail + 3 * WSIZE;
  PUT(prol, 0);                            /* Alignment padding */
  PUT(prol + (1 * WSIZE), PACK(DSIZE, 1)); /* Prologue header */
  PUT(prol + (2 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
  PUT(prol + (3 * WSIZE), PACK(0, 1));     /* Epilogue header */
  return 0;
}

/* Extends the heap with a new free block. */
static void *extend_heap(size_t words) {
  char *bp;
  size_t size;

  /* Allocate an even number of words to maintain alignment */
  size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
  if ((long)(bp = mem_sbrk(size)) == -1) {
    return NULL;
  }
  /* Initialize free block header/footer and the epilogue header */
  PUT(HDRP(bp), PACK(size, 0));         /* Free block header */
  PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

  /* Coalesce if the previous block was free */
  return coalesce(bp);
}

/*
 * mm_malloc - Allocates a block from the free list
 */
void *mm_malloc(size_t size) {
  size_t asize;      /* Adjusted block size */
  size_t extendsize; /* Amount to extend heap if no fit */
  char *bp;

  /* Ignore spurious requests */
  if (size == 0) return NULL;

  /* Adjust block size to include overhead and alignment reqs. */
  if (size <= DSIZE)
    asize = MINSIZE;
  else
    asize = DSIZE * ((size + (2 * DSIZE) + (DSIZE - 1)) / DSIZE);

  /* Search the free list for a fit */
  if ((bp = find_fit(asize)) != NULL) {
    return place(bp, asize);
  }

  /* No fit found. Get more memory and place the block */
  extendsize = MAX(asize, CHUNKSIZE);
  if ((bp = extend_heap(extendsize / WSIZE)) == NULL) return NULL;
  return place(bp, asize);
}

/* allocator places the requested block and optionally splits the excess*/
static void *place(void *bp, size_t asize) {
  size_t size = GET_SIZE(HDRP(bp));
  size_t remain = size - asize;
  char *pre = (char *)GET(PRED(bp));
  char *suc = (char *)GET(SUCC(bp));
  if (remain >= (3 * DSIZE)) {
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(remain, 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(remain, 0));
    PUT(SUCC(pre), suc);
    PUT(PRED(suc), pre);
    coalesce(NEXT_BLKP(bp));
  } else {
    PUT(HDRP(bp), PACK(size, 1));
    PUT(FTRP(bp), PACK(size, 1));
    remov(bp);
  }
  return bp;
}

/* first fit */
static void *find_fit(size_t asize) {
  head = root;
  tail = head + 4 * WSIZE;
  char *bp;
  for (size_t i = MINSIZE; i < 1024; i += DSIZE) {
    if (asize <= i) {
      if ((bp = check_list(asize)) != NULL) {
        return bp;
      }
    }
    head += 8 * WSIZE;
    tail = head + 4 * WSIZE;
  }
  for (size_t i = 1024; i <= MAXSIZE; i *= 2) {
    if (asize <= i) {
      if ((bp = check_list(asize)) != NULL) {
        return bp;
      }
    }
    head += 8 * WSIZE;
    tail = head + 4 * WSIZE;
  }
  return NULL;
}

static void *check_list(size_t asize) {
  char *bp = (char *)GET(SUCC((head)));
  size_t size = GET_SIZE(HDRP(bp));
  while (size < asize) {
    bp = (char *)GET(SUCC((bp)));
    size = GET_SIZE(HDRP(bp));
    if (bp == tail) {
      return NULL;
    }
  }
  return bp;
}

/*
 * inser - insert the block to head of free list.
 */
static void inser(void *bp) {
  char *suc = (char *)GET(SUCC(head));
  PUT(SUCC(head), bp);
  PUT(SUCC(bp), suc);
  PUT(PRED(suc), bp);
  PUT(PRED(bp), head);
}

/*
 * insertAppr - insert the block to an appropriate free list.
 */
static void insertAppr(void *bp) {
  head = root;
  tail = head + 4 * WSIZE;
  size_t size = GET_SIZE(HDRP(bp));
  for (size_t i = MINSIZE; i < 1024; i += DSIZE) {
    if (size <= i) {
      inser(bp);
      return;
    }
    head += 8 * WSIZE;
    tail = head + 4 * WSIZE;
  }
  for (size_t i = 1024; i <= MAXSIZE; i *= 2) {
    if (size <= i) {
      inser(bp);
      return;
    }
    head += 8 * WSIZE;
    tail = head + 4 * WSIZE;
  }
  // if none of the free list fits,insert at the end
  head -= 8 * WSIZE;
  tail = head + 4 * WSIZE;
  inser(bp);
}

/*
 * remov - remove the block from the free list.
 */
static void remov(void *bp) {
  char *pre = (char *)GET(PRED(bp));
  char *suc = (char *)GET(SUCC(bp));
  PUT(SUCC(pre), suc);
  PUT(PRED(suc), pre);
}

/*
 * mm_free - Freeing a block with coalsescing.
 */
void mm_free(void *bp) {
  size_t size = GET_SIZE(HDRP(bp));
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  coalesce(bp);
}

/* coalesce - depends on the case*/
static void *coalesce(void *bp) {
  size_t size = GET_SIZE(HDRP(bp));
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

  if (prev_alloc && next_alloc) { /* Case 1 */
    insertAppr(bp);
  } else if (prev_alloc && !next_alloc) { /* Case 2 */
    remov(NEXT_BLKP(bp));
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    insertAppr(bp);
  } else if (!prev_alloc && next_alloc) { /* Case 3 */
    remov(PREV_BLKP(bp));
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
    insertAppr(bp);
  } else { /* Case 4 */
    remov(PREV_BLKP(bp));
    remov(NEXT_BLKP(bp));
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
    insertAppr(bp);
  }
  return bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
  void *oldptr = ptr;
  void *newptr;
  size_t copySize;
  size_t newSize;
  size_t oldSize = GET_SIZE(HDRP(ptr));
  copySize = GET_SIZE(HDRP(oldptr));
  if (size < copySize) copySize = size;

  /* Adjust block size to include overhead and alignment reqs. */
  if (size <= DSIZE)
    size = MINSIZE;
  else
    size = DSIZE * ((size + (2 * DSIZE) + (DSIZE - 1)) / DSIZE);

  if (ptr == NULL) {
    return mm_malloc(size);
  }
  if (size == 0) {
    mm_free(ptr);
    return ptr;
  }
  if (oldSize >= size) {
    return ptr;
  } else if (oldSize < size) {
    newptr = realloc_coalesce(oldptr, size);
    newSize = GET_SIZE(HDRP(newptr));
    if (newSize < size) {
      newptr = mm_malloc(size);
      if (newptr == NULL) return NULL;
      memmove(newptr, oldptr, copySize);
      mm_free(oldptr);
    } else {
      if (newptr != oldptr) {
        memmove(newptr, oldptr, copySize);
      }
    }
  }
  return newptr;
}

/* coalesce piece if okay */
static void *realloc_coalesce(void *bp, size_t asize) {
  size_t size = GET_SIZE(HDRP(bp));
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

  if (prev_alloc && next_alloc) { /* Case 1 */
    return bp;
  } else if (prev_alloc && !next_alloc) { /* Case 2 */
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    remov(NEXT_BLKP(bp));
    if (size >= asize) {
      PUT(HDRP(bp), PACK(size, 1));
      PUT(FTRP(bp), PACK(size, 1));
    }
  } else if (!prev_alloc && next_alloc) { /* Case 3 */
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    if (size >= asize) {
      remov(PREV_BLKP(bp));
      PUT(FTRP(bp), PACK(size, 1));
      PUT(HDRP(PREV_BLKP(bp)), PACK(size, 1));
      bp = PREV_BLKP(bp);
    }
  } else if (!prev_alloc && !next_alloc) { /* Case 4 */
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
    if (size >= asize) {
      remov(PREV_BLKP(bp));
      remov(NEXT_BLKP(bp));
      PUT(HDRP(PREV_BLKP(bp)), PACK(size, 1));
      PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 1));
      bp = PREV_BLKP(bp);
    }
  }
  return bp;
}

/* Heap Consistency Checker */
int mm_check(void) {
  void *iter = (void *)GET(SUCC(head));
  while (iter != head) {
    iter = (void *)GET(SUCC(iter));
  }
  iter = (void *)GET(PRED(tail));
  while (iter != tail) {
    iter = (void *)GET(PRED(iter));
  }
  return 0;
}