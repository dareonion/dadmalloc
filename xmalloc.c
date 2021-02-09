#ifndef _ALLOC_H
#define _ALLOC_H 1

/* title: xmalloc () / free () - pair according to K&R 2, p.185ff */
/*
  extern void *xmalloc(size_t size);
  extern void *xfree( void *ptr );
*/

typedef long align;

union header {
    /* head of an allocation block */
    struct {
        union header * ptr; /* Pointer to circular successor */
        unsigned size; /* Size of the block */
    }s;

    /*   __attribute__ ((aligned (8)));
     * Align x;  Forces block alignment */
};

typedef union header header;

static header base;
/* Start header */
static header * freep = 0; /* Current entry point in free list */

void * xmalloc (unsigned nbytes) {
    header * p, * prevp;
    header *morecore(unsigned); /* Call to the operating system */
    unsigned nunits;

    /* Least multiple of sizeof (header) that the
       may contain the required number of bytes, plus 1 for the header itself: */

    nunits = (nbytes + sizeof (header) -1) / sizeof (header) + 1;

    if ((prevp = freep) == 0) {/* First call, initialization */
	base.s.ptr = freep = prevp = & base;
	base.s.size = 0; /* base becomes a block of length 0 */
    }
    for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {

	/* p iterates the free list, followed by prevp, none
           Termination condition !! */

	if (p->s.size>= nunits) {/* Is p big enough? */
            if (p->s.size == nunits) /* If exactly, it will ... */
		prevp->s.ptr = p->s.ptr; /* ... taken from the list */
            else {/* otherwise ... */
                p->s.size -= nunits; /* p is reduced */
                p += p->s.size; /* and the last part ... */
                p->s.size = nunits; /* ... of the block ... */
            }
            freep = prevp;
            return (void *) (p + 1); /* ... returned, however
                                        at the address of p + 1,
                                        since p points to the header. */
	}
	if (p == freep) /* If the list does not have a block
                           contains sufficient size,
                           will call morecore () */
            if ( (p = morecore(nunits)) == 0)	return 0;
    }
}

#define NALLOC 102400 /* ttt Minimum size for morecore () call */

/* A static function is not visible outside of your file */

header * morecore(unsigned nu){
    char mem1[NALLOC];
    char * cp, * sbrk (int);
    void xfree (void *);
    header * up;
    if (nu <NALLOC) nu = NALLOC;
    /* ttt */
    /* cp = sbrk (nu * sizeof (header)); */
    /* if (cp == (char *) -1) /\* sbrk returns -1 in the event of an error *\/ */
    /*     return 0; */

    cp = &mem1;
    up = (header *) cp;
    up->s.size = nu; /* Size is entered */
    xfree ((void *) (up + 1)); /* Installation in free list */
    return freep;
}

void xfree(void * ap) {
    /* return to free list */
    header * bp, * p;

    bp = (header *) ap - 1;
    /* Here is the header of the block */

    /* The list is scanned, the block should be the
       Address size to be inserted correctly,
       to enable defragmentation. */

    for (p = freep;!(bp >p && bp < p->s.ptr);
         p = p->s.ptr
         )
        if (p>= p->s.ptr && (bp>p || bp <p->s.ptr))
            break; /* bp is in front of block with smallest or behind
                      Block with the largest address */

    if (bp + bp->s.size == p->s.ptr) {
        /* Union with upper neighbor */
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    }
    else
        bp->s.ptr = p->s.ptr;
    if (p + p->s.size == bp) {
        /* Association with lower neighbor */
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else
        p->s.ptr = bp;
    freep = p;
}

#endif

