/* lowbuf.c */
/* Copyright 1995 by Steve Kirkendall */


/* This file contains functions which divide the session file into several
 * buffers.  These functions use session.c, and are used by buffer.c
 */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_lowbuf[] = "$Id: lowbuf.c,v 2.46 2003/10/17 17:41:23 steve Exp $";
#endif

/* These control the sizes of some tiny caches for storing line offsets and
 * info about random offsets.  You can disable these by making them 0.  This
 * is intended to make lowline() run faster.
 */
#define LINECACHE	4

#if USE_PROTOTYPES
static short checksum(BLK *blk);
static BLKNO delblock(_BLKNO_ blklist, _LBLKNO_ lblkno, COUNT *ncharsptr, COUNT *nlinesptr);
static BLKNO insblock(_BLKNO_ blklist, _LBLKNO_ before, _BLKNO_ chars, _COUNT_ nchars, _COUNT_ nlines);
static BLKNO lockchars(_BLKNO_ bufinfo, _LBLKNO_ lblkno, _BLKNO_ blkno);
static void unlockchars(_BLKNO_ bufinfo, _LBLKNO_ lblkno, _BLKNO_ blkno, int chgchars, int chglines);
static void helpinit(_BLKNO_ bufinfo, void (*bufproc)(_BLKNO_ bufinfo, long nchars, long nlines, long changes, long prevloc, CHAR *name));
#endif

#if LINECACHE
static void clobbercache P_((_BLKNO_ dst));
#endif

/******************************************************************************/
/* Some internal functions...						      */

/* Compute the checksum for a bufinfo block */
static short checksum(blk)
	BLK	*blk;	/* block whose checksum should be calculated */
{
	short	oldsum;
	register short	sum;
	register short	*scan;

	/* remember the old sum, and then force it to 0 temporarily */
	oldsum = blk->bufinfo.checksum;
	blk->bufinfo.checksum = 0;

	/* count the sum, treating the block as an array of BLKNOs. */
	for (sum = 0, scan = &blk->sumshorts[o_blksize / sizeof(short)];
	     scan != blk->sumshorts;
	     )
	{
		/* We know that o_blksize is a power of 2, >= 256.  We can
		 * safely assume that sizeof(short) is also a power of 2, <=16,
		 * so we can safely unroll this loop.
		 */
		sum += scan[-1] + scan[-2] + scan[-3] + scan[-4]
			+ scan[-5] + scan[-6] + scan[-7] + scan[-8]
			+ scan[-9] + scan[-10] + scan[-11] + scan[-12]
			+ scan[-13] + scan[-14] + scan[-15] + scan[-16];
		scan -= 16;
	}

	/* restore the old sum, but then return the new sum */
	blk->bufinfo.checksum = oldsum;
	return sum;
}


/* This function deletes one whole CHARS block from a BLKLIST block, and
 * returns the BLKNO of the altered version of the block which the "blklist"
 * argument referred to.  This function is recursive.
 */
static BLKNO delblock(blklist, lblkno, ncharsptr, nlinesptr)
	_BLKNO_	blklist;	/* a BLKLIST block */
	_LBLKNO_ lblkno;	/* index into blklist's array of blocks */
	COUNT	*ncharsptr;	/* number of characters deleted */
	COUNT	*nlinesptr;	/* number of lines deleted */
{
	BLKNO	next;
	BLK	*blk;

	assert(blklist != 0);

	/* if the lblkno is in a later block, then delete it recursively
	 * and watch out for the next blklist being copied-on-write!
	 */
	if (lblkno >= SES_MAXBLKLIST)
	{
		blklist = seslock(blklist, ElvTrue, SES_BLKLIST);
		blk = sesblk(blklist);
		next = delblock(blk->blklist.next, (LBLKNO)(lblkno - SES_MAXBLKLIST), ncharsptr, nlinesptr);
		if (next == blk->blklist.next)
		{
			sesunlock(blklist, ElvFalse);
		}
		else
		{
			blk->blklist.next = next;
			sesunlock(blklist, ElvTrue);
		}
		return blklist;
	}

	/* the doomed lblkno is in this block.  Free the char block */
	(void)seslock(blklist, ElvFalse, SES_BLKLIST);
	blk = sesblk(blklist);
	assert(blk->blklist.blk[lblkno].blkno != 0);
	sesfree(blk->blklist.blk[lblkno].blkno);

	/* copy the chars & lines counts to caller's variables */
	if (ncharsptr) *ncharsptr = blk->blklist.blk[lblkno].nchars;
	if (nlinesptr) *nlinesptr = blk->blklist.blk[lblkno].nlines;

	/* If it is the only block in this blklist (and, by implication,
	 * this is the final blklist), then free the blklist block.
	 */
	if (lblkno == 0 && blk->blklist.blk[1].blkno == 0)
	{
		sesunlock(blklist, ElvFalse);
		sesfree(blklist);
		return 0;
	}
	sesunlock(blklist, ElvFalse);

	/* shift this blklist's blk[] array to delete the element */
	blklist = seslock(blklist, ElvTrue, SES_BLKLIST);
	blk = sesblk(blklist);
	while (lblkno < SES_MAXBLKLIST - 1)
	{
		blk->blklist.blk[lblkno] = blk->blklist.blk[lblkno + 1];
		lblkno++;
	}

	/* shift in a 0 (in the last block) or the first item from the
	 * next blklist block.
	 */
	if (blk->blklist.next == 0)
	{
		/* shift in a 0 */
		blk->blklist.blk[lblkno].blkno = 0;
		blk->blklist.blk[lblkno].nchars = 0;
		blk->blklist.blk[lblkno].nlines = 0;
	}
	else
	{
		/* shift in the first element of the next blklist's blk[] */
		next = blk->blklist.next;
		(void)seslock(next, ElvFalse, SES_BLKLIST);
		blk->blklist.blk[lblkno] = sesblk(next)->blklist.blk[0];
		sesalloc(blk->blklist.blk[lblkno].blkno, SES_CHARS);
		sesunlock(next, ElvFalse);

		/* recursively shift following blklists' blk[] arrays */
		blk->blklist.next = delblock(next, 0, (COUNT *)0, (COUNT *)0);
	}

	/* return BLKNO of altered version of this blklist */
	sesunlock(blklist, ElvTrue);
	return blklist;
}

/* This function inserts a CHARS block into the blklist, at a given LBLKNO.
 * If necessary, it will also allocate a new BLKLIST block to extend the
 * blklist.
 *
 * This function is recursive; each invocation returns the blklist argument,
 * or the blkno of an updated version of that block.
 */
static BLKNO insblock(blklist, before, chars, nchars, nlines)
	_BLKNO_	blklist;	/* a BLKLIST */
	_LBLKNO_ before;	/* where to insert a block */
	_BLKNO_	chars;		/* the CHARS block to be inserted */
	_COUNT_	nchars;		/* number of character in "chars" */
	_COUNT_	nlines;		/* number of lines in "chars" */
{
	BLKNO	next;	/* the block after blklist */
	BLK	*blk;	/* contents of a BUFINFO or BLKLIST block */
	int	i;

	assert(chars != 0 && nchars != 0 && nchars < SES_MAXCHARS && nlines <= nchars);

	/* if no blklist, then create one */
	if (!blklist)
	{
		assert(before == 0);

		/* give the buffer its first blkno */
		blklist = sesalloc(0, SES_BLKLIST);

		/* make the new CHARS block be its first entry */
		(void)seslock(blklist, ElvTrue, SES_BLKLIST);
		blk = sesblk(blklist);
		blk->blklist.blk[0].blkno = chars;
		blk->blklist.blk[0].nchars = nchars;
		blk->blklist.blk[0].nlines = nlines;
		sesunlock(blklist, ElvTrue);
	}
	/* should the chars block be inserted into this blklist block? */
	else if (before < SES_MAXBLKLIST)
	{
		/* yes!  Get this blocklist block */
		blklist = seslock(blklist, ElvTrue, SES_BLKLIST);
		blk = sesblk(blklist);

		/* if the blklist is full, then insert the last item of
		 * this block before the first item in the next block.
		 */
		i = SES_MAXBLKLIST - 1;
		if (blk->blklist.blk[i].blkno)
		{
			blk->blklist.next = insblock(blk->blklist.next, 0,
				blk->blklist.blk[i].blkno,
				blk->blklist.blk[i].nchars,
				blk->blklist.blk[i].nlines);
		}

		/* insert the chars block into this blklist block */
		for (; (unsigned)i > before; --i)
		{
			blk->blklist.blk[i] = blk->blklist.blk[i - 1];
		}
		blk->blklist.blk[before].blkno = chars;
		blk->blklist.blk[before].nchars = nchars;
		blk->blklist.blk[before].nlines = nlines;

		/* done! */
		sesunlock(blklist, ElvTrue);
	}
	else /* chars will be inserted into a later blklist block */
	{
		blklist = seslock(blklist, ElvTrue, SES_BLKLIST);
		blk = sesblk(blklist);
		next = insblock(blk->blklist.next, (LBLKNO)(before - SES_MAXBLKLIST), chars, nchars, nlines);
		if (blk->blklist.next != next)
		{
			blk->blklist.next = next;
			sesunlock(blklist, ElvTrue);
		}
		else
		{
			sesunlock(blklist, ElvFalse);
		}
	}
	return blklist;
}

/* This function locks a CHARS block for writing.  Doing this may require
 * doing a copy-on-write, which may in turn require doing a copy-on-write
 * of the blklist block which refers to the CHARS block, and so on back to
 * the bufinfo block.  Returns the BLKNO of the CHARS block.
 */
static BLKNO lockchars(bufinfo, lblkno, blkno)
	_BLKNO_	bufinfo;	/* a BUFINFO block */
	_LBLKNO_ lblkno;	/* index info BLKLIST of the CHARS block */
	_BLKNO_	blkno;		/* physical block number of the CHARS block */
{
	BLKNO	locked;		/* BLKNO of various blocks after locking */
	BLKNO	blklist, next;	/* BLKLIST blocks */
	BLK	*blk;

	assert(bufinfo != 0 && blkno != 0);
#ifdef DEBUG_SCAN
	scan__nobuf();
#endif

	/* step 1: lock the chars block.  If its BLKNO remains unchanged,
	 * then we're done.
	 */
	locked = seslock(blkno, ElvTrue, SES_CHARS);
	if (locked == blkno)
	{
		return blkno;
	}

	/* step 2: fetch the bufinfo block. */
	next = seslock(bufinfo, ElvFalse, SES_BUFINFO);
	assert(next == bufinfo);
	blk = sesblk(bufinfo);

	/* step 3: lock the blklist block which contains the requested
	 * chars block.  Since blklist blocks are never shared, this
	 * shouldn't require copy-on-write.
	 */
	blklist = seslock(blk->bufinfo.first, ElvTrue, SES_BLKLIST);
	blk = sesblk(blklist);
	while (lblkno >= SES_MAXBLKLIST)
	{
		next = seslock(blk->blklist.next, ElvTrue, SES_BLKLIST);
		assert(next != 0);
		sesunlock(blklist, ElvFalse);
		blklist = next;
		lblkno -= SES_MAXBLKLIST;
		blk = sesblk(blklist);
	}
	assert(blk->blklist.blk[lblkno].blkno == blkno);

	/* step 4: replace the blkno in the blklist */
	blk->blklist.blk[lblkno].blkno = locked;
	sesunlock(blklist, ElvTrue);
	sesunlock(bufinfo, ElvFalse);
	return locked;
}

/* This function unlocks a CHARS block which has been locked by lockchars(),
 * and then updates the nchars and nlines statistics in the blklist block.
 * It is assumed that no copy-on-write will be necessary for the blklist block;
 * if it was, lockchars() would have done it.
 */
static void unlockchars(bufinfo, lblkno, blkno, chgchars, chglines)
	_BLKNO_	bufinfo;	/* a BUFINFO block */
	_LBLKNO_ lblkno;	/* index into BLKLIST of the CHARS block */
	_BLKNO_	blkno;		/* physical block number of CHARS block */
	int	chgchars;	/* change in the number of characters */
	int	chglines;	/* change in the number of lines */
{
	BLKNO	blklist, next;
	BLK	*blk;

	assert(bufinfo != 0 && blkno != 0);

	/* update block statistics, if necessary */
	if (chgchars != 0 || chglines != 0)
	{
		/* locate the BLKLIST block that refers to this CHARS block */
		(void)seslock(bufinfo, ElvFalse, SES_BUFINFO);
		blklist = sesblk(bufinfo)->bufinfo.first;
		assert(blklist != 0);
		sesunlock(bufinfo, ElvFalse);
		for (; lblkno >= SES_MAXBLKLIST; lblkno -= SES_MAXBLKLIST)
		{
			(void)seslock(blklist, ElvFalse, SES_BLKLIST);
			next = sesblk(blklist)->blklist.next;
			sesunlock(blklist, ElvFalse);
			blklist = next;
			assert(blklist != 0);
		}

		/* update the nchars and nlines counts */
		next = seslock(blklist, ElvTrue, SES_BLKLIST);
		assert(next == blklist);
		blk = sesblk(blklist);
		blk->blklist.blk[lblkno].nchars += chgchars;
		assert(blk->blklist.blk[lblkno].nchars != 0
			&& blk->blklist.blk[lblkno].nchars < SES_MAXCHARS);
		blk->blklist.blk[lblkno].nlines += chglines;
		assert(blk->blklist.blk[lblkno].nlines < SES_MAXCHARS);
		assert(blk->blklist.blk[lblkno].blkno == blkno);
		sesunlock(blklist, ElvTrue);
	}

	/* unlock the chars block for writing */
	sesunlock(blkno, ElvTrue);
}

/******************************************************************************/
/* session restarting function                                                */

#ifdef FEATURE_MISC
/* This function helps lowinit(), by initializing a single buffer from the
 * session file.
 */
static void helpinit(bufinfo, bufproc)
	_BLKNO_	bufinfo;	/* a BUFINFO block */
	void	(*bufproc)P_((_BLKNO_ bufinfo, long nchars, long nlines, long changes, long prevloc, CHAR *name));
{
	BLKNO	blklist, next;
	BLK	*binfo, *blist;
	long	nchars = 0, nlines = 0;
	int	i;

	/* mark the bufinfo block as being "allocated", and lock it in memory */
	next = sesalloc(bufinfo, SES_BUFINFO);
	assert(next == bufinfo);
	(void)seslock(bufinfo, ElvFalse, SES_BUFINFO);
	binfo = sesblk(bufinfo);

	/* if it is damaged, skip it */
	if (checksum(binfo) != binfo->bufinfo.checksum)
	{
		fprintf(stderr, "found a bad version of \"%s\"\n", binfo->bufinfo.name);
		sesunlock(bufinfo, ElvFalse);
		return;
	}

	/* for each blklist block... */
	for (blklist = binfo->bufinfo.first; blklist; blklist = next)
	{
		/* mark the blklist block as being "allocated", & lock it */
		(void)seslock(sesalloc(blklist, SES_BLKLIST), ElvFalse, SES_BLKLIST);
		blist = sesblk(blklist);

		/* For each chars block mentioned in the BLKLIST... */
		for (i = 0; (unsigned)i < SES_MAXBLKLIST && blist->blklist.blk[i].blkno; i++)
		{
			assert(blist->blklist.blk[i].nchars < SES_MAXCHARS);
			assert(blist->blklist.blk[i].nlines <= blist->blklist.blk[i].nchars);

			/* mark the CHARS block as being "allocated" */
			next = sesalloc(blist->blklist.blk[i].blkno, SES_CHARS);
			assert(next == blist->blklist.blk[i].blkno);

			/* count lines & chars in block */
			nchars += blist->blklist.blk[i].nchars;
			nlines += blist->blklist.blk[i].nlines;
		}

		/* find the next blklist, and then unlock this one */
		next = blist->blklist.next;
		sesunlock(blklist, ElvFalse);
	}

	/* let the BUFFER module initialize itself */
	(*bufproc)(bufinfo, nchars, nlines, binfo->bufinfo.changes, binfo->bufinfo.prevloc, toCHAR(binfo->bufinfo.name));

	/* unlock the bufinfo block */
	sesunlock(bufinfo, ElvFalse);
}
#endif /* FEATURE_MISC */


/* This function uses sesopen() to open or create a session file.  Then it
 * looks for any existing buffers, and marks their blocks as being "allocated".
 * It also calls a function (supplied as an argument) so that the BUFFER module
 * can also do its own bookkeeping for the found buffer.
 */
void lowinit(bufproc)
	void	(*bufproc) P_((_BLKNO_ bufinfo, long nchars, long nlines, long changes, long prevloc, CHAR *name));
{
#ifdef FEATURE_MISC
	BLK	*blk;
	BLKNO	blkno, next;
	int	i;
#endif

	/* create or open the session file */
	sesopen(o_recovering);

#ifdef FEATURE_MISC
	/* lock the superblock */
	(void)seslock(0, ElvFalse, SES_SUPER);
	blk = sesblk(0);

	/* for each buffer mentioned in the superblock... */
	for (i = 0; (unsigned)i < SES_MAXSUPER; i++)
	{
		/* ... skipping empty slots... */
		if (blk->super.buf[i])
		{
			/* process the buffer */
			helpinit(blk->super.buf[i], bufproc);
		}
	}

	/* continue on to SUPER2 blocks, if any.  Be sure to mark the SUPER2's
	 * as being "allocated".
	 */
	blkno = blk->super.next;
	sesunlock(0, ElvFalse);
	while (blkno)
	{
		/* mark the block as allocated, and lock it into memory */
		(void)seslock(sesalloc(blkno, SES_SUPER2), ElvFalse, SES_SUPER2);

		/* for each buffer mentioned in the superblock... */
		for (i = 0; (unsigned)i < SES_MAXSUPER2; i++)
		{
			/* ... skipping empty slots... */
			if (blk->super2.buf[i])
			{
				/* process the buffer */
				helpinit(blk->super2.buf[i], bufproc);
			}
		}

		/* find the next block */
		next = blk->super2.next;
		sesunlock(blkno, ElvFalse);
		blkno = next;
	}
#endif /* FEATURE_MISC */
}


/******************************************************************************/
/* buffer creation/destruction functions.                                     */

/* Create a new buffer in the session file */
BLKNO lowalloc(name)
	char	*name;	/* name of the buffer to create */
{
	BLKNO	bufinfo;
	BLKNO	super;
	BLKNO	next;
	BLK	*blk;
	int	i;

	safeinspect();

	/* Allocate a block, and lock it for writing */
	bufinfo = seslock(sesalloc(0, SES_BUFINFO), ElvTrue, SES_BUFINFO);

	/* Fill in the block's info */
	blk = sesblk(bufinfo);
	blk->bufinfo.changes = 0L;
	blk->bufinfo.prevloc = 0L;
	blk->bufinfo.reserved = 0;
	blk->bufinfo.first = 0;
	strncpy(blk->bufinfo.name, name, (size_t)SES_MAXBUFINFO);
	blk->bufinfo.checksum = checksum(blk);

	/* Unlock the block */
	sesunlock(bufinfo, ElvTrue);

	/* Lock the superblock for writing */
	super = seslock(0, ElvTrue, SES_SUPER);
	assert(super == 0);
	blk = sesblk(super);

	/* Find an empty slot in the bufs list, and put this buffer there */
	i = -1;
	do
	{
		/* if we reached the end of this block, start next */
		if ((unsigned)++i >= (super==0 ? SES_MAXSUPER : SES_MAXSUPER2))
		{
			sesunlock(super, ElvFalse);
			next = (super==0 ? blk->super.next : blk->super2.next);
			if (next == 0)
			{
				/* need to allocate a new super2 */
				next = seslock(super, ElvTrue, SES_SUPER);
				assert(next == super);
				next = sesalloc(0, SES_SUPER2);
				if (super == 0)
				{
					blk->super.buf[i] = next;
				}
				else
				{
					blk->super2.buf[i] = next;
				}
				sesunlock(super, ElvTrue);
			}
			super = seslock(next, ElvTrue, SES_SUPER2);
			assert(super == next);
			blk = sesblk(super);
			i = 0;
		}
	} while (((super==0) ? blk->super.buf[i] : blk->super2.buf[i]) != 0);
	if (super == 0)
	{
		blk->super.buf[i] = bufinfo;
	}
	else
	{
		blk->super2.buf[i] = bufinfo;
	}

	/* Unlock the superblock (or its super2 block) */
	sesunlock(super, ElvTrue);
	safeinspect();
	return bufinfo;
}

/* Create a copy of an existing buffer in the session file.  This is used
 * for creating "undo" versions of a buffer before changing it.
 */
BLKNO lowdup(originfo)
	_BLKNO_	originfo;	/* BUFINFO block of original lowbuf */
{
	BLKNO	dupinfo, dupblklist, prevdup = 0;
	BLKNO	scan, next;
	BLK	*blk, *dupblk = NULL, *dupbiblk;
	int	i;

#if LINECACHE
	clobbercache(originfo);
#endif

	/* Lock the originfo block */
	(void)seslock(originfo, ElvFalse, SES_BUFINFO);
	blk = sesblk(originfo);

	/* Create a buffer */
	dupinfo = lowalloc(blk->bufinfo.name);

	/* Lock its bufinfo block for writing */
	scan = seslock(dupinfo, ElvTrue, SES_BUFINFO);
	assert(scan == dupinfo);

	/* Copy the original bufinfo into the new bufinfo */
	dupbiblk = sesblk(dupinfo);
	memcpy(dupbiblk, blk, (size_t)o_blksize);

	/* Unlock the original bufinfo block */
	sesunlock(originfo, ElvFalse);

	/* For each blklist block... */
	scan = blk->bufinfo.first;
	while (scan)
	{
		/* allocate a new blklist block, for the new buffer */
		dupblklist = sesalloc(0, SES_BLKLIST);

		/* link the new blklist block into the new buffer's list */
		if (prevdup != 0)
		{
			dupblk->blklist.next = dupblklist;
			sesunlock(prevdup, ElvTrue);
		}
		else
		{
			dupbiblk->bufinfo.first = dupblklist;
		}

		/* Lock the blklist blocks */
		(void)seslock(scan, ElvFalse, SES_BLKLIST);
		blk = sesblk(scan);
		next = seslock(dupblklist, ElvTrue, SES_BLKLIST);
		assert(next == dupblklist);
		dupblk = sesblk(dupblklist);
		prevdup = dupblklist;

		/* copy the contents from one blklist to the other */
		memcpy(dupblk, blk, (size_t)o_blksize);

		/* For each chars block... */
		for (i = 0; (unsigned)i < SES_MAXBLKLIST && blk->blklist.blk[i].blkno; i++)
		{
			/* Increment allocation count on the chars block */
			(void)sesalloc(blk->blklist.blk[i].blkno, SES_CHARS);
		}
		assert((unsigned)i == SES_MAXBLKLIST || !blk->blklist.next);

		/* unlock the blklist blocks, and procede to next blklist */
		next = blk->blklist.next;
		sesunlock(scan, ElvFalse);
		scan = next;
	}

	/* unlock the final new blklist block (if any) */
	if (dupblk)
	{
		sesunlock(prevdup, ElvTrue);
	}

	/* Unlock the new bufinfo block for writing */
	dupbiblk->bufinfo.checksum = checksum(dupbiblk);
	sesunlock(dupinfo, ElvTrue);
	return dupinfo;
}

/* This function deletes a buffer from the session file. */
void lowfree(bufinfo)
	_BLKNO_	bufinfo;	/* BUFINFO block of lowbuf to destroy */
{
	BLKNO	scan, next;
	BLK	*blk;
	int	i;

#if LINECACHE
	clobbercache(bufinfo);
#endif
	/* Lock the superblock for writing */
	scan = seslock(0, ElvTrue, SES_SUPER);
	assert(scan == 0);
	blk = sesblk(0);

	/* Find this buffer in the bufs list, and replace it with 0 */
	i = -1;
	do
	{
		/* if we reached the end of this block, start next */
		if ((unsigned)++i >= (scan==0 ? SES_MAXSUPER : SES_MAXSUPER2))
		{
			next = (scan==0 ? blk->super.next : blk->super2.next);
			sesunlock(scan, ElvFalse);
			assert(next != 0);
			scan = seslock(next, ElvTrue, SES_BLKLIST);
			assert(scan == next);
			blk = sesblk(scan);
			i = 0;
		}
	} while ((unsigned)((scan==0) ? blk->super.buf[i] : blk->super2.buf[i]) != bufinfo);
	if (scan == 0)
	{
		blk->super.buf[i] = 0;
	}
	else
	{
		blk->super2.buf[i] = 0;
	}
	
	/* Unlock the superblock for writing, and then flush it to disk.
	 * Flushing it is important because we're about to free the blocks
	 * from this buffer, and those blocks are likely to be reallocated
	 * right away for some other purpose.  If elvis crashed at that time
	 * and the super block had never been flushed, then the session file
	 * might be too scrambled for elvis to read it successfully.
	 */
	sesunlock(scan, ElvTrue);
	sesflush(scan);

	/* Lock the bufinfo block */
	(void)seslock(bufinfo, ElvFalse, SES_BUFINFO);

	/* For each blklist block... */
	for (scan = sesblk(bufinfo)->bufinfo.first; scan; scan = next)
	{
		/* Lock the blklist block */
		(void)seslock(scan, ElvFalse, SES_BLKLIST);

		/* For each chars block... */
		blk = sesblk(scan);
		for (i = 0; (unsigned)i < SES_MAXBLKLIST && blk->blklist.blk[i].blkno; i++)
		{
			/* free the chars block */
			sesfree(blk->blklist.blk[i].blkno);
		}
		assert((unsigned)i == SES_MAXBLKLIST || !blk->blklist.next);

		/* Unlock the blklist block */
		next = blk->blklist.next;
		sesunlock(scan, ElvFalse);

		/* Free the blklist block */
		sesfree(scan);
	}

	/* Unlock the bufinfo block */
	sesunlock(bufinfo, ElvFalse);

	/* Free the bufinfo block */
	sesfree(bufinfo);
}

/* Change the title of a buffer */
void lowtitle(bufinfo, title)
	_BLKNO_	bufinfo;	/* bufinfo block whose title is to be changed */
	CHAR	*title;		/* the new title */
{
	BLK	*blk;
#ifdef DEBUG_SESSION
	BLKNO	blkno;

	blkno = seslock(bufinfo, ElvTrue, SES_BUFINFO);
	assert(blkno == bufinfo);
#else
	(void)seslock(bufinfo, ElvTrue, SES_BUFINFO);
#endif

	blk = sesblk(bufinfo);
	assert(blk);
	CHARncpy(blk->bufinfo.name, title, (size_t)SES_MAXBUFINFO);
	sesunlock(bufinfo, ElvTrue);
}

/******************************************************************************/

#if LINECACHE
static struct
{
	BLKNO	bufinfo;/* bufinfo of a known line, or 0 for none */
	long	lineno;	/* line number */
	long	offset;	/* offset of the start of the line */
} linecache[LINECACHE];
static int lineidx;	/* index into linecache[] of slot to recycle */
#endif /* LINECACHE */


#if LINECACHE
static void clobbercache(dst)
	_BLKNO_	dst;	/* bufinfo of a changed buffer */
{
	register int	i;

	for (i = 0; i < LINECACHE; i++)
#if 0
		if (linecache[i].bufinfo == dst)
#endif
			linecache[i].bufinfo = 0;
}
#endif /* LINECACHE */

/* convert a line number to an offset, for a given buffer */
long lowline(bufinfo, lineno)
	_BLKNO_	bufinfo;	/* BUFINFO of the buffer */
	long	lineno;		/* line number (starting with 1) */
{
	BLK	*blk;	/* contents of a bufinfo block or blklist block */
	BLKNO	blklist;/* used for stepping from one blklist to next */
	BLKNO	next;	/* the next blklist block, after this one */
	long	offset;	/* total offset seen */
	int	i;	/* used for scanning through blklist.blk[] */
	int	nlines;	/* number of lines/chars in block */
#if LINECACHE
	long	origline;/* a copy of lineno */

	/* check the cache first */
	for (i = 0; i < LINECACHE; i++)
	{
		if (linecache[i].bufinfo == bufinfo && linecache[i].lineno == lineno)
		{
			return linecache[i].offset;
		}
	}
	origline = lineno;
#endif /* LINECACHE */

	/* outside this function, line numbers start with 1, but in here they start at 0 */
	lineno--;

	/* find the first blklist */
	(void)seslock(bufinfo, ElvFalse, SES_BUFINFO);
	blklist = sesblk(bufinfo)->bufinfo.first;
	sesunlock(bufinfo, ElvFalse);

	/* if empty buffer then return 0 */
	if (!blklist)
	{
		return 0;
	}

	/* for each blklist... */
	for (offset = 0; ; blklist = next)
	{
		assert(blklist != 0);

		/* for each element of blklist->blk[]... */
		seslock(blklist, ElvFalse, SES_BLKLIST);
		blk = sesblk(blklist);
		for (i = 0; (unsigned)i < SES_MAXBLKLIST && blk->blklist.blk[i].blkno; i++)
		{
			/* see if we've found the right chars block yet */
			if ((nlines = blk->blklist.blk[i].nlines) >= lineno)
			{
				/* unlock the blklist, and lock the chars block */
				next = blk->blklist.blk[i].blkno;
				i = blk->blklist.blk[i].nchars;
				sesunlock(blklist, ElvFalse);
				(void)seslock(next, ElvFalse, SES_CHARS);
				blk = sesblk(next);

				/* AT THIS POINT...
				 * lineno = number of newlines to move past
				 *          in this CHARS block.
				 * nlines = total number of newlines in this
				 *          CHARS block
				 * i      = total chars in this CHARS block
				 * offset = offset of start of this block
				 */

				/* locate the line within the chars block */
				if (lineno + lineno <= nlines)
				{
					/* count newlines from front of block */
					for (i = 0; lineno > 0; offset++, i++)
					{
						assert((unsigned)i < SES_MAXCHARS);
						if (blk->chars.chars[i] == '\n')
						{
							lineno--;
						}
					}
				}
				else
				{
					/* count newlines from rear of block */
					for (offset += i, lineno -= nlines; ; )
					{
						assert(i > 0);
						if (blk->chars.chars[--i]=='\n')
						{
							if (lineno++ == 0)
								break;
						}
						offset--;
					}
				}

#if LINECACHE
				/* stuff information into the cache */
				linecache[lineidx].bufinfo = bufinfo;
				linecache[lineidx].offset = offset;
				linecache[lineidx].lineno = origline;
				lineidx = (lineidx + 1) % LINECACHE;
#endif /* LINECACHE */

				/* return the offset */
				sesunlock(next, ElvFalse);
				return offset;
			}
			lineno -= blk->blklist.blk[i].nlines;
			offset += blk->blklist.blk[i].nchars;
		}

		/* find the next blklist */
		next = blk->blklist.next;
		assert(!next || (unsigned)i == SES_MAXBLKLIST);
		sesunlock(blklist, ElvFalse);
		if (!next)
		{
			/* there is no next blklist block -- return final offset */
			return offset;
		}
	}
	/*NOTREACHED*/
}

/******************************************************************************/

/* Convert a byte offset into a BLKNO.  This function scans a buffer's
 * blklist blocks, and uses the character counts there to determine
 * which block contains the character for a given offset.
 *
 * Optionally, it can also determine how many characters are to the left
 * or right of the offset, in that block.  If "left" and "right" aren't NULL,
 * the variables they point to will be set.  (The character at the offset
 * is included * in the "right" count.)  It can similarly set a variable
 * indicating the logical block number of the block containing the offset.
 *
 * There are three special cases:
 *	* If a negative offset is given, it is treated as 0.
 *	* If an offset past the end of the buffer is given, it is treated as
 *	  being a byte in the last block, after the last byte in that block;
 *	  this is *usually* where the next byte would be appended; the "right"
 *	  value is then set to 0 (under all other conditions, it would be at
 *	  least 1).
 *	* If the buffer has no blocks, then all variables and the return
 *	  value will be 0.
 */
BLKNO lowoffset(bufinfo, offset, left, right, lptr, linenum)
	_BLKNO_	bufinfo; /* BUFINFO of the lowbuf */
	long	offset;	 /* offset into the buffer */
	COUNT	*left;	 /* output: number of preceding bytes in CHARS block */
	COUNT	*right;	 /* output: number of following bytes in CHARS block */
	LBLKNO	*lptr;	 /* output: logical block number of CHARS block */
	long	*linenum;/* output: line number */
{
	BLKNO	blklist;/* used for stepping from one blklist to next */
	BLKNO	next;	/* the next blklist block, after this one */
	LBLKNO	lblkno;	/* counts overall LBLKNO of the given offset */
	long	lnum;	/* counts newlines */
	BLK	*blk;	/* contents of a bufinfo or blklist block */
	register int	i;	/* used for scanning through blklist.blk[] */
	int	nlines;	/* lines in current CHARS block */
	int	nchars;	/* chars in current CHARS block */
	register struct blki_s *blki;
	BLKNO	maxblklist = SES_MAXBLKLIST;

	/* treat negative offsets as 0 */
	if (offset < 0)
	{
		offset = 0;
	}

	/* find the first blklist */
	(void)seslock(bufinfo, ElvFalse, SES_BUFINFO);
	blklist = sesblk(bufinfo)->bufinfo.first;
	sesunlock(bufinfo, ElvFalse);

	/* if the buffer has no blocks, then any offset is past the end of
	 * the buffer, so do the "*right = 0" thing.
	 */
	if (!blklist)
	{
		if (left) *left = 0;
		if (right) *right = 0;
		if (lptr) *lptr = 0;
		if (linenum) *linenum = 0;
		return 0;
	}

	/* for each blklist... */
	for (lblkno = 0, lnum = 1; ; lblkno += maxblklist, blklist = next)
	{
		assert(blklist != 0);

		/* for each element of blklist->blk[]... */
		seslock(blklist, ElvFalse, SES_BLKLIST);
		blk = sesblk(blklist);
		for (i = 0, blki = blk->blklist.blk;
		     i < maxblklist && blki->blkno;
		     lnum += blki->nlines, i++, blki++)
		{
			/* see if we've found it yet */
			offset -= blki->nchars;
			if (offset < 0L)
			{
				/* Yes!  Return the info */
				next = blki->blkno;
				nlines = blki->nlines;
				nchars = blki->nchars;
				sesunlock(blklist, ElvFalse);
				if (lptr) *lptr = lblkno + i;
				i = offset + nchars;
				if (left) *left = i;
				if (right) *right = (unsigned short)-offset;
				if (linenum)
				{
					/* AT THIS POINT...
					 * lnum = line# of front of block
					 * i    = index into block
					 * nlines = total \n's in CHARS block
					 * nchars = total chars in CHARS block
					 * next   = BLKNO of the CHARS block
					 */
					if (i + i <= nchars)
					{
						/* count from front of block */
						seslock(next, ElvFalse, SES_CHARS);
						blk = sesblk(next);
						while (--i >= 0)
						{
							if (blk->chars.chars[i] == '\n')
							{
								lnum++;
							}
						}
						sesunlock(next, ElvFalse);
					}
					else
					{
						/* count from rear of block */
						lnum += nlines;
						seslock(next, ElvFalse, SES_CHARS);
						blk = sesblk(next);
						do
						{
							if (blk->chars.chars[--nchars] == '\n')
							{
								lnum--;
							}
						} while (i < nchars);
						sesunlock(next, ElvFalse);
					}
					*linenum = lnum;
				}
				return next;
			}
		}

		/* find the next blklist */
		next = blk->blklist.next;
		assert(!next || i == maxblklist);
		if (!next)
		{
			/* there is no next blklist block -- do the "*right = 0" * thing.  */
			if (left) *left = blk->blklist.blk[i - 1].nchars;
			if (right) *right = 0;
			if (lptr) *lptr = lblkno + i - 1;
			if (linenum) *linenum = lnum;
			next = blk->blklist.blk[i - 1].blkno;
			sesunlock(blklist, ElvFalse);
			return next;
		}
		sesunlock(blklist, ElvFalse);
	}
	/*NOTREACHED*/
}




/* This function inserts a string into a buffer.  It returns the change in
 * the number of lines in the buffer (i.e., the number of newline characters
 * in the "new" string.
 */
long lowinsert(dst, dsttop, newp, newlen)
	_BLKNO_	dst;	/* BUFINFO of lowbuf */
	long	dsttop;	/* offset where insertion should begin */
register CHAR	*newp;	/* new text to be inserted */
	long	newlen;	/* length of new text */
{
	BLKNO	blkno;
	COUNT	left;
	COUNT	right;
	LBLKNO	lblkno;
	BLK	*biblk;
	BLK	*cblk;
	int	nlines;
register BLKNO	newblkno;	/* a new block */
	BLKNO	blklist;
	BLK	*newblk;
	long	totlines = 0;
register int	i;
	int	j;

	safeinspect();

#if LINECACHE
	clobbercache(dst);
#endif

	/* figure out where insertion should begin */
	blkno = lowoffset(dst, dsttop, &left, &right, &lblkno, (long *)0);

	/* will the new text fit in the current block? */
	if (blkno != 0 && (unsigned)(left + right + newlen) < SES_MAXCHARS)
	{
		/* yes!  Do a single-block rewrite. */

		/* lock the chars block for writing (trickier than it sounds!) */
		blkno = lockchars(dst, lblkno, blkno);

		/* twiddle the contents of that block */
		cblk = sesblk(blkno);
		for (i = left + right + newlen - 1; i >= left + newlen; i--)
		{
			cblk->chars.chars[i] = cblk->chars.chars[i - newlen];
		}
		nlines = 0;
		for (i = 0; i < newlen; i++)
		{
			cblk->chars.chars[i + left] = newp[i];
			if (newp[i] == '\n') nlines++;
		}

		/* unlock that block, and update its nchars and nlines */
		unlockchars(dst, lblkno, blkno, (int)newlen, nlines);
		totlines = (long)nlines;
	}
	else
	{
		/* if we're splitting an existing block... */
		if (blkno != 0 && left != 0 && right != 0)
		{
			/* allocate a new block */
			newblkno = seslock(sesalloc(0, SES_CHARS), ElvTrue, SES_CHARS);
			newblk = sesblk(newblkno);

			/* copy the second half of the characters into it */
			blkno = lockchars(dst, lblkno, blkno);
			cblk = sesblk(blkno);
			nlines = 0;
			for (i = 0, j = left; j < left + right; i++, j++)
			{
				newblk->chars.chars[i] = cblk->chars.chars[j];
				if (newblk->chars.chars[i] == '\n')
				{
					nlines++;
				}
			}
			sesunlock(newblkno, ElvTrue);

			/* insert the new block after the first */
			seslock(dst, ElvTrue, SES_BUFINFO);
			biblk = sesblk(dst);
			blklist = insblock(biblk->bufinfo.first, (LBLKNO)(lblkno + 1), newblkno, (COUNT)i, (COUNT)nlines);
			biblk->bufinfo.first = blklist;
			biblk->bufinfo.changes++;
			biblk->bufinfo.prevloc = dsttop;
			biblk->bufinfo.checksum = checksum(biblk);
			sesunlock(dst, ElvTrue);

			/* copy some new text into first half, filling it 90% */
			nlines = -nlines;
			for (i = left, j = -right;
			     i < o_blkfill && newlen > 0;
			     i++, j++, newlen--)
			{
				cblk->chars.chars[i] = *newp;
				if (*newp++ == '\n')
				{
					nlines++;
					totlines++;
				}
			}
			unlockchars(dst, lblkno, blkno, j, nlines);
		}

		/* tweak the value of lblkno, if necessary, so we always insert
		 * before lblkno (never after)
		 */
		if (left > 0)
		{
			lblkno++;
		}

		/* while we have more text to insert */
		seslock(dst, ElvTrue, SES_BUFINFO);
		biblk = sesblk(dst);
		blklist = biblk->bufinfo.first;
		while (newlen > 0)
		{
			/* allocate a new block */
			newblkno = seslock(sesalloc(0, SES_CHARS), ElvTrue, SES_CHARS);
			newblk = sesblk(newblkno);

			/* copy text into it */
			nlines = 0;
			for (i = 0; i < o_blkfill && newlen > 0; i++, newlen--)
			{
				newblk->chars.chars[i] = *newp;
				if (*newp++ == '\n')
				{
					nlines++;
				}
			}
			totlines += (long)nlines;
			sesunlock(newblkno, ElvTrue);

			/* insert the new block */
			blklist = insblock(blklist, lblkno, newblkno, (COUNT)i, nlines);
			lblkno++;
		}
		biblk->bufinfo.changes++;
		biblk->bufinfo.first = blklist;
		biblk->bufinfo.checksum = checksum(biblk);
		sesunlock(dst, ElvTrue);
	}

	safeinspect();

	return totlines;
}

/* This function deletes characters located between two points.  It returns
 * the change in the number of lines (i.e., the negative of the quantity of
 * newlines deleted).
 *	dst: the bufinfo block
 *	dsttop: offset to start of region to delete
 *	dstbottom: offsert to end of region to delete
 */
long lowdelete(dst, dsttop, dstbottom)
	_BLKNO_	dst;		/* BUFINFO of lowbuf */
	long	dsttop;		/* offset where deletion should begin */
	long	dstbottom;	/* offset where deletion should end */
{
	BLK	*blk;
	COUNT	nlines;		/* change in line count for a single block */
	long	totlines = 0;	/* total change in line count */
	BLKNO	first;
	LBLKNO	firstlblk;	/* block containing dsttop */
	COUNT	firstleft;	/* chars in firstlblk to left of dsttop */
	COUNT	firstright;	/* chars in firstlblk to right of dsttop */
	BLKNO	last;
	LBLKNO	lastlblk;	/* block containing dstbottom */
	COUNT	lastleft;	/* chars in firstlblk to left of dstbottom */
	COUNT	lastright;	/* chars in firstlblk to right of dstbottom */
	LBLKNO	lblkno;
	BLK	*biblk;
	int	i, j;

	safeinspect();

#if LINECACHE
	clobbercache(dst);
#endif

	/* lock the bufinfo block, and fetch it */
	(void)seslock(dst, ElvTrue, SES_BUFINFO);
	biblk = sesblk(dst);

	/* If both ends are in the same block */
	first = lowoffset(dst, dsttop, &firstleft, &firstright, &firstlblk, (long *)0);
	last = lowoffset(dst, dstbottom, &lastleft, &lastright, &lastlblk, (long *)0);

	if (first == last || (firstlblk + 1 == lastlblk && lastleft == 0))
	{
		safeinspect();

		/* tweak the special case where lastleft==0 */
		if (first != last)
		{
			lastleft = firstleft + firstright;
			lastright = 0;
		}

		/* deleting one whole block? */
		if (firstleft == 0 && lastleft == firstright)
		{
			/* yes, delete the whole block */
			biblk->bufinfo.first = delblock(biblk->bufinfo.first, firstlblk, (LBLKNO *)0, &nlines);
			safeinspect();
		}
		else
		{
			/* delete part of block by modifying CHARS block */

			/* Lock the chars block for writing; update blklist block */
			first = lockchars(dst, firstlblk, first);
			safeinspect();

			/* count the doomed lines */
			nlines = 0;
			blk = sesblk(first);
			for (i = firstleft; i < lastleft; i++)
			{
				if (blk->chars.chars[i] == '\n')
				{
					nlines++;
				}
			}

			/* Delete the chars from chars block */
			for (i = firstleft, j = lastleft; j < lastleft + lastright; i++, j++)
			{
				blk->chars.chars[i] = blk->chars.chars[j];
			}
			safeinspect();

			/* Unlock the chars block, updating chars & lines counts */
			unlockchars(dst, firstlblk, first, firstleft - lastleft, -(int)nlines);
			safeinspect();
		}
		totlines = -(long)nlines;
	}
	else
	{
		safeinspect();

		/* Delete any whole blocks from the middle portion of the doomed text. */
		lastlblk++;
		for (lblkno = (firstleft == 0 ? firstlblk : firstlblk + 1);
		     lblkno + 1 < (lastright == 0 ? lastlblk + 1 : lastlblk);
		     lastlblk--)
		{
			biblk->bufinfo.first = delblock(biblk->bufinfo.first, lblkno, (LBLKNO *)0, &nlines);
			totlines -= (long)nlines;
		}
		lastlblk--;
		safeinspect();

		/* If there is a first block */
		if (firstleft > 0 && firstright > 0)
		{
			/* Lock the chars block for writing; update blklist block */
			first = lockchars(dst, firstlblk, first);

			/* count the doomed lines, and zero the doomed characters */
			nlines = 0;
			blk = sesblk(first);
			for (i = firstleft; i < firstleft + firstright; i++)
			{
				if (blk->chars.chars[i] == '\n')
				{
					nlines++;
				}
				blk->chars.chars[i] = '\0';
			}

			/* Unlock the chars block, updating chars & lines counts */
			unlockchars(dst, firstlblk, first, -firstright, -nlines);
			totlines -= (long)nlines;
			safeinspect();
		}

		/* If there is a last block */
		if (lastleft > 0 && lastright > 0)
		{
			/* Lock the chars block for writing; update blklist block */
			last = lockchars(dst, lastlblk, last);

			/* count the doomed lines */
			nlines = 0;
			blk = sesblk(last);
			for (i = 0; i < lastleft; i++)
			{
				if (blk->chars.chars[i] == '\n')
				{
					nlines++;
				}
			}

			/* Delete old text from start of block */
			for (i = 0, j = lastleft; j < lastleft + lastright; i++, j++)
			{
				blk->chars.chars[i] = blk->chars.chars[j];
			}
			while (i < lastleft + lastright)
			{
				blk->chars.chars[i++] = '\0';
			}

			/* Unlock the chars block, updating chars & lines counts */
			unlockchars(dst, lastlblk, last,  -lastleft, -nlines);
			totlines -= (long)nlines;
			safeinspect();
		}
	}

	/* Update the bufinfo block's checksum */
	biblk->bufinfo.changes++;
	biblk->bufinfo.prevloc = dsttop;
	biblk->bufinfo.checksum = checksum(biblk);

	/* Unlock the bufinfo block for writing */
	sesunlock(dst, ElvTrue);
	safeinspect();
	return totlines;
}

/* Replace one chunk of text with another, and return the change in the
 * number of lines.
 */
long lowreplace(dst, dsttop, dstbottom, newp, newlen)
	_BLKNO_	dst;		/* BUFINFO of the lowbuf */
	long	dsttop;		/* offset where replacement begins */
	long	dstbottom;	/* offset of end of old text */
	CHAR	*newp;		/* new text */
	long	newlen;		/* length of new text */
{
	long	totlines = 0;	/* total change in line count */

	totlines += lowdelete(dst, dsttop, dstbottom);
	totlines += lowinsert(dst, dsttop, newp, newlen);
	return totlines;
}

/* Insert part of one buffer into another, and return the change in the number
 * of lines.
 */
long lowpaste(dst, dsttop, src, srctop, srcbottom)
	_BLKNO_	dst;		/* BUFINFO of destination lowbuf */
	long	dsttop;		/* offset where text should be copied to */
	_BLKNO_	src;		/* BUFINFO of source lowbuf */
	long	srctop;		/* offset of start of source text */
	long	srcbottom;	/* offset of end of source text */
{
	COUNT	left, right;
	BLKNO	blkno;
	BLK	*blk;
	BLK	*tmpblk = NULL;
	long	totlines = 0;

	safeinspect();

	/* destination can't be in the middle of the source */
	assert(src != dst || dsttop <= srctop || dsttop >= srcbottom);

#if LINECACHE
	clobbercache(dst);
#endif

	/* Small problem: If the destination is in the same buffer as the
	 * source, and happens to be located before the source in the same
	 * block, then when inserting the new text we'll be altering the block
	 * cache which messes up our pointers.
	 *
	 * Solution: If source & dest are in the same buffer, and dest comes
	 * before source, then we need to use a private copy of each source
	 * block.
	 */
	if (dst == src && dsttop <= srctop)
	{
		tmpblk = (BLK *)safealloc((int)o_blksize, 1);
	}

	/* we'll do this in chunks -- transfering text from each block of the
	 * source into the destination, until all text has been transferred.
	 */
	while (srctop < srcbottom)
	{
		/* read the text block, and find the start of source text in it */
		blkno = lowoffset(src, srctop, &left, &right, (LBLKNO *)0, (long *)0);

		seslock(blkno, ElvFalse, SES_CHARS);
		blk = sesblk(blkno);
		if (tmpblk)
		{
			memcpy(tmpblk, blk, (size_t)o_blksize);
			sesunlock(blkno, ElvFalse);
			blk = tmpblk;
		}

		/* Usually we'll be transferring all text from here to the end
		 * of the block.  If this is the last block, we may be
		 * transferring less.
		 */
		if (right > srcbottom - srctop)
		{
			right = (unsigned short)(srcbottom - srctop);
		}

		/* Copy the chunk of text into the destination. */
		totlines += lowinsert(dst, dsttop, &blk->chars.chars[left], (long)right);
		srctop += right;
		dsttop += right;

		/* if we inserted text before the source, then we must tweak the
		 * source offsets.
		 */
		if (tmpblk)
		{
			srctop += right; /* again */
			srcbottom += right;
		}
		else /* different buffers -- the blk was used without copying */
		{
			sesunlock(blkno, ElvFalse);
		}
	}

	/* free the temporary block copy, if we used one */
	if (tmpblk)
	{
		safefree(tmpblk);
	}

	safeinspect();

	return totlines;
}
