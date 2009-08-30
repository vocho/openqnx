/* scan.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_scan[] = "$Id: scan.c,v 2.26 2003/10/17 17:41:23 steve Exp $";
#endif

#ifdef FEATURE_LITRE
static	struct scan_s saved;
static long changes;
#endif

/* This variable points to the top of the stack of scan contexts */
struct scan_s *scan__top;


/* This variable points to the head of a list of freed scan contexts.
 * The scanalloc() function checks this variable, and recycles the first
 * scan context instead of allocating a new one, to reduce the number of
 * calls to safealloc().
 */
#ifndef DEBUG_ALLOC
static struct scan_s *recycle = (struct scan_s *)0;
#endif


/* This variable is used by the scanmark() macro */
MARKBUF	scan__markbuf;


#ifdef DEBUG_SCAN
/* This checks the scan stack, to make sure we aren't scanning a buffer.
 * The lowbuf.c:lockchars() function calls this, since scanning and modifying
 * should never be done at the same time.  Scanning strings is allowed though.
 */
void scan__nobuf P_((void))
{
	struct scan_s *s;

	for (s = scan__top; s; s = s->next)
		if (s->buffer)
			abort();
}
#endif /* DEBUG_ALLOC */



/* This function creates a new scan context, and starts the scanning at
 * a given mark.  The scan context must be freed by a later scanfree()
 * call.  The cp argument is used to distinguish one scan context from
 * another, and the (CHAR *) that it points to will be set to point to the
 * appropriate CHAR in the buffer.  The value of that pointer is returned.
 * If the seek is past either end of the buffer, *cp is set to NULL.
 */
#ifndef DEBUG_SCAN
CHAR	*scanalloc(cp, start)
#else
CHAR	*_scanalloc(file, line, cp, start)
	char	*file;	/* name of file where allocating */
	int	line;	/* line number where allocating */
#endif
	CHAR	**cp;	/* address of pointer which will be used for scanning */
	MARK	start;	/* where the scanning should begin */
{
	struct scan_s *newp;

	assert(start != NULL && cp != NULL);

	/* allocate a new scan context */
#ifndef DEBUG_ALLOC
	if (recycle)
	{
		newp = recycle;
		recycle = recycle->next;
		memset((char *)newp, 0, sizeof(*newp));
	}
	else
	{
		newp = (struct scan_s *)safealloc(1, sizeof *newp);
	}
#else
	newp = (struct scan_s *)_safealloc(file, line, ElvFalse, 1, sizeof *newp);
#endif
	newp->next = scan__top;
	scan__top = newp;

	/* initialize it */
	newp->ptr = cp;
#ifdef DEBUG_SCAN
	newp->file = file;
	newp->line = line;
#endif
	return scanseek(cp, start);
}



/* This function creates a new scan context for scanning a string.  This makes
 * it possible to write functions which can scan a string or the contents of a
 * buffer with equal ease.
 */
#ifdef DEBUG_SCAN
CHAR	*_scanstring(file, line, cp, str)
	char	*file;
	int	line;
#else
CHAR	*scanstring(cp, str)
#endif
	CHAR	**cp;	/* address of pointer which will be used for scanning */
	CHAR	*str;	/* NUL-terminated string to scan */
{
	struct scan_s *newp;

	assert(str != NULL && cp != NULL);

	/* allocate a new scan context */
#ifndef DEBUG_ALLOC
	if (recycle)
	{
		newp = recycle;
		recycle = recycle->next;
		memset((char *)newp, 0, sizeof(*newp));
	}
	else
	{
		newp = (struct scan_s *)safealloc(1, sizeof *newp);
	}
#else
	newp = (struct scan_s *)_safealloc(file, line, ElvFalse, 1, sizeof *newp);
#endif
	newp->next = scan__top;
	scan__top = newp;

	/* initialize it */
	newp->buffer = (BUFFER)0;
	newp->bufinfo = (BLKNO)0;
	newp->blkno = (BLKNO)0;
	newp->leftedge = str;
	newp->rightedge = str + CHARlen(str);
	newp->ptr = cp;
	newp->leoffset = 0;
#ifdef DEBUG_SCAN
	newp->file = file;
	newp->line = line;
#endif

	/* initialize *cp to point to the start of the string */
	*cp = str;
	return *cp;
}


/* This function allocates a new scan context that is identical to an
 * existing scan context.  I.e., it is like scanalloc(&new, scanmark(&old))
 * However, this function is usually much faster.
 */
#ifdef DEBUG_SCAN
CHAR *_scandup(file, line, cp, oldp)
	char	*file;
	int	line;
#else
CHAR *scandup(cp, oldp)
#endif
	CHAR	**cp;	/* address of pointer to use in new scan context */
	CHAR	**oldp;	/* address of pointer used in existing scan context */
{
	struct scan_s *newp;

	assert(scan__top && scan__top->ptr == oldp);

	/* allocate a new scan context */
#ifndef DEBUG_ALLOC
	if (recycle)
	{
		newp = recycle;
		recycle = recycle->next;
		memset((char *)newp, 0, sizeof(*newp));
	}
	else
	{
		newp = (struct scan_s *)safealloc(1, sizeof *newp);
	}
#else
	newp = (struct scan_s *)_safealloc(file, line, ElvFalse, 1, sizeof *newp);
#endif
	newp->next = scan__top;
	scan__top = newp;

	/* initialize it to match the old scan context */
	newp->buffer = newp->next->buffer;
	newp->bufinfo = newp->next->bufinfo;
	newp->blkno = newp->next->blkno;
	newp->leftedge = newp->next->leftedge;
	newp->rightedge = newp->next->rightedge;
	newp->ptr = cp;
	newp->leoffset = newp->next->leoffset;
#ifdef DEBUG_SCAN
	newp->file = file;
	newp->line = line;
#endif

	/* initialize *cp to point to the correct character */
	*cp = *oldp;

	/* lock the block that *cp points to. */
	if (newp->blkno)
	{
		seslock(newp->blkno, ElvFalse, SES_CHARS);
	}

	return *cp;
}


/* This function frees a scan context which was created by scanalloc() */
void	scanfree(cp)
	CHAR	**cp;	/* address of pointer used for scanning */
{
	struct scan_s	*context;

	assert(cp != NULL);
	assert(scan__top != NULL);
	assert(scan__top->ptr == cp);

#ifdef FEATURE_LITRE
	/* save info from this scan, to speed up later scans */
	if (!scan__top->next && scan__top->buffer && scan__top->blkno != 0)
	{
		saved = *scan__top;
		changes = saved.buffer->changes;
	}
#endif /* FEATURE_LITRE */

	/* delete it from the list */
	context = scan__top;
	scan__top = scan__top->next;

	/* free its resources */
	if (context->blkno)
	{
		sesunlock(context->blkno, ElvFalse);
	}
#ifndef DEBUG_ALLOC
	context->next = recycle;
	recycle = context;
#else
	safefree(context);
#endif
}

/* This function uses an existing scan context created by scanalloc() or
 * scanstring(), to start a new  scan at a given mark.  If the original scan
 * context was created by scanalloc(), the mark can even refer to a totally
 * different buffer than the original scan.  If the original scan context
 * was created by scanstring(), then the mark can only be moved within the
 * original string buffer.
 */
CHAR	*scanseek(cp, restart)
	CHAR	**cp;	/* address of pointer used for scanning */
	MARK	restart;/* where scanning should resume */
{
	COUNT	      left, right;	/* number of chars in block */
	BLKNO	      nextblkno;

	assert(cp != NULL && restart != NULL);
	assert(scan__top->ptr == cp);

	/* Can't mix string scans with buffer seeks, or vice versa.  Testing
	 * this is a bit tricky because scanalloc() calls scanseek() to fill in
	 * a bunch of fields, we have to allow that; hence the leftedge test.
	 */
	assert(!(markbuffer(restart) && !scan__top->buffer && scan__top->leftedge));
	assert(!(!markbuffer(restart) && scan__top->buffer));

	/* string scan or buffer scan? */
	if (!markbuffer(restart))
	{
		/* STRING */

		/* compute the new value of the pointer */
		*cp = &scan__top->leftedge[markoffset(restart)];

		/* if seeking to a non-existent offset, return NULL */
		if (*cp < scan__top->leftedge
		 || *cp >= scan__top->rightedge)
		{
			*cp = NULL;
			return NULL;
		}
	}
	else
	{
		/* BUFFER */

#if 1 /* I think this is safe, but not very helpful */
		/* if seeking within the same block, then it's easy */
		if (markbuffer(restart) == scan__top->buffer
		 && markoffset(restart) >= scan__top->leoffset
		 && scan__top->leoffset +
		   (int)(scan__top->rightedge - scan__top->leftedge)
				> markoffset(restart))
		{
			*cp = &scan__top->leftedge[markoffset(restart) - scan__top->leoffset];
			return *cp;
		}
#endif

		/* if seeking to a non-existent offset, return NULL */
		if (markoffset(restart) < 0
		 || markoffset(restart) >= o_bufchars(markbuffer(restart)))
		{
			*cp = NULL;
			return NULL;
		}

		/* find the bufinfo block for the buffer that MARK refers to */
		scan__top->buffer = markbuffer(restart);
		scan__top->bufinfo = bufbufinfo(scan__top->buffer);

		/* find the chars block.  If this scan context happens to be
		 * nested inside another one and we're seeking into the same
		 * block, then we can optimize this a lot if the block is
		 * already locked.
		 */
		if (scan__top->next
		 && scan__top->next->blkno != 0
		 && scan__top->next->buffer == scan__top->buffer
		 && scan__top->next->leoffset <= markoffset(restart)
		 && scan__top->next->leoffset +
		   (int)(scan__top->next->rightedge - scan__top->next->leftedge)
				> markoffset(restart))
		{
			/* hooray!  We can avoid calling lowoffset() */
			nextblkno = scan__top->next->blkno;
			left = (int)(markoffset(restart) - scan__top->next->leoffset);
			right = (int)(scan__top->next->rightedge - scan__top->next->leftedge)
				- left;
		}
#ifdef FEATURE_LITRE
		else if (saved.buffer == scan__top->buffer
		 && changes == scan__top->buffer->changes
		 && saved.leoffset <= markoffset(restart)
		 && saved.leoffset + (int)(saved.rightedge - saved.leftedge)
				> markoffset(restart))
		{
			/* In a previous block, but that block isn't locked
			 * anymore.  Need to lock it, but at least we avoided
			 * calling lowoffset().
			 */
			nextblkno = saved.blkno;
			left = (int)(markoffset(restart) - saved.leoffset);
			right = (int)(saved.rightedge - saved.leftedge) - left;
		}
#endif /* FEATURE_LITRE */
		else
		{
			/* can't optimize; must call lowoffset to find block */
			nextblkno = lowoffset(scan__top->bufinfo,
				markoffset(restart), &left, &right, NULL, NULL);
		}
		assert(right > 0 && nextblkno > 0);

		/* unlock the old block, and lock the new one */
		if (scan__top->blkno != nextblkno)
		{
			if (scan__top->blkno)
			{
				sesunlock(scan__top->blkno, ElvFalse);
			}
			seslock(nextblkno, ElvFalse, SES_CHARS);
			scan__top->blkno = nextblkno;
		}

		/* set the other variables */
		scan__top->leftedge = sesblk(scan__top->blkno)->chars.chars;
		scan__top->rightedge = scan__top->leftedge + left + right;
		scan__top->leoffset = markoffset(restart) - left;
		*cp = scan__top->leftedge + left;
	}

	return *cp;
}

#ifdef DEBUG_SCAN
/* This function is a helper function for the scanleft() macro.  This function
 * should not be called directly.
 *
 * It returns the number of buffered characters to the left of the current
 * scan point.
 */
COUNT	scan__left(cp)
	CHAR	**cp;	/* address of pointer used for scanning */
{
	assert(cp != NULL);
	assert(scan__top->ptr == cp);

	/* return the number of buffered CHARs */
	return (COUNT)(*cp - scan__top->leftedge);
}

/* This function is a helper function for the scanright() macro.  This function
 * should not be called directly.
 *
 * It returns the number of buffered characters to the right of the current
 * scan point.
 */
COUNT	scan__right(cp)
	CHAR	**cp;	/* address of pointer used for scanning */
{
	assert(cp != NULL);
	assert(scan__top->ptr == cp);

	/* return the number of buffered CHARs */
	return (COUNT)(scan__top->rightedge - *cp);
}

/* create a mark which refers to the current point of the scan */
MARK	scanmark(cp)
	CHAR	**cp;	/* address of pointer used for scanning */
{
	assert(cp != NULL);
	assert(scan__top->ptr == cp);

	/* return a temporary mark at the proper position */
	return marktmp(scan__markbuf, scan__top->buffer, scan__top->leoffset + (int)(*cp - scan__top->leftedge));
}
#endif /* DEBUG_SCAN */

/* This function is a helper function for the scannext() macro.  This function
 * should not be called directly.
 */
CHAR	*scan__next(cp)
	CHAR	**cp;	/* address of pointer used for scanning */
{
	MARKBUF	      markbuf;

	assert(cp != NULL && *cp != NULL);
	assert(scan__top->ptr == cp);

	/* if the next character is in the same block, its easy */
	if (++*cp < scan__top->rightedge)
	{
		return *cp;
	}

	assert(*cp == scan__top->rightedge);

	/* else seek to the first character of following block */
	return scanseek(cp, marktmp(markbuf, scan__top->buffer,
		scan__top->leoffset + (int)(scan__top->rightedge - scan__top->leftedge)));
}

/* This function is a helper function for the scanprev() macro.  This function
 * should not be called directly.
 */
CHAR	*scan__prev(cp)
	CHAR	**cp;	/* address of pointer used for scanning */
{
	MARKBUF	      markbuf;

	assert(cp != NULL);
	assert(scan__top->ptr == cp);

	/* if the previous character is in the same block, its easy */
	if (--*cp >= scan__top->leftedge)
	{
		return *cp;
	}

	/* if this was the first block, we're at the end */
	if (scan__top->leoffset == 0)
	{
		if (scan__top->blkno != 0)
		{
			sesunlock(scan__top->blkno, ElvFalse);
			scan__top->blkno = 0;
		}
		*cp = NULL;
		return NULL;
	}

	/* else seek to the last character of preceding block */
	return scanseek(cp, marktmp(markbuf, scan__top->buffer, scan__top->leoffset - 1));
}


/* This function returns the single character at a given location.  It is
 * similar to scanseek(), except that this function doesn't use a scan context,
 * and it returns a CHAR instead of a pointer-to-CHAR.
 */
CHAR scanchar(mark)
	MARK	mark;	/* buffer/offset of the character to fetch */
{
	COUNT	left, right;	/* number of chars in block */
	BLKNO	bufinfo;	/* block describing the buffer */
	BLKNO	blkno;		/* block containing the character */
	CHAR	ch;		/* the character */

	/* if seeking to a non-existent offset, return '\0' */
	if (markoffset(mark) < 0
	 || markoffset(mark) >= o_bufchars(markbuffer(mark)))
	{
		return '\0';
	}

	/* find the bufinfo block for the buffer that MARK refers to */
	bufinfo = bufbufinfo(markbuffer(mark));

	/* find the chars block */
	blkno = lowoffset(bufinfo, markoffset(mark), &left, &right, NULL, NULL);
	assert(right != 0);

	/* lock the block */
	seslock(blkno, ElvFalse, SES_CHARS);

	/* fetch the character */
	ch = sesblk(blkno)->chars.chars[left];

	/* unlock the block */
	sesunlock(blkno, ElvFalse);

	return ch;
}
