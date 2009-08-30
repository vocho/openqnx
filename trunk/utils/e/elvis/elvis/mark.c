/* mark.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_mark[] = "$Id: mark.c,v 2.18 2002/11/01 19:29:49 steve Exp $";
#endif


MARK namedmark[26];

/* Allocate a mark, pointing to a specific location in a specific buffer.
 * As changes are make to the buffer, the mark's offset into that buffer
 * will automatically be updated.
 */
#ifndef DEBUG_ALLOC
MARK markalloc(buffer, offset)
	BUFFER	buffer;	/* the buffer that the new mark will refer to */
	long	offset;	/* position of the mark within buffer */
{
	MARK	newp;

	newp = (MARK)safealloc(1, sizeof(MARKBUF));
#else
MARK _markalloc(file, line, buffer, offset)
	char	*file;	/* source file where allocating */
	int	line;	/* source line where allocating */
	BUFFER	buffer;	/* the buffer that the new mark will refer to */
	long	offset;	/* position of the mark within buffer */
{
	MARK	newp;

	assert(buffer != NULL && offset >= 0 && offset <= o_bufchars(buffer));

	newp = (MARK)_safealloc(file, line, False, 1, sizeof(MARKBUF));
	/*fprintf(stderr, "markalloc(0x%lx, %ld) called from %s(%d), returning 0x%lx\n", (long)buffer, offset, file, line, (long)newp);*/
#endif
	newp->buffer = buffer;
	newp->offset = offset;
	newp->next = bufmarks(buffer);
	bufsetmarks(buffer, newp);
	return newp;
}

/* Free a mark which was created by markalloc().  This also adjusts the
 * namedmarks[] array, if necessary.
 */
#ifndef DEBUG_ALLOC
void markfree(mark)
	MARK	mark;	/* the mark to destroy */
{
#else
void _markfree(file, line, mark)
	char	*file;
	int	line;
	MARK	mark;
{
#endif
	MARK	lag;
	int	i;

#ifdef DEBUG_ALLOC
	/*fprintf(stderr, "markfree(0x%lx) called from %s(%d)\n", (long)mark, file, line);*/
#endif
	/* remove from buffer's list of marks */
	if (mark == bufmarks(mark->buffer))
	{
		bufsetmarks(mark->buffer, mark->next);
	}
	else
	{
		for (lag = bufmarks(mark->buffer); lag->next != mark; lag = lag->next)
		{
			assert(lag->next);
		}
		lag->next = mark->next;
	}

	/* if in namedmarks, then unset the namedmarks variable */
	for (i = 0; i < QTY(namedmark); i++)
	{
		if (namedmark[i] == mark)
		{
			namedmark[i] = NULL;
			break;
		}
	}

	/* free the mark's resources */
#ifndef DEBUG_ALLOC
	safefree(mark);
#else
	_safefree(file, line, mark);
#endif
}

/* Adjust the offset of every who's offset is greater than "from".  If at "to"
 * or later, add "delta" to it.  Between "from" and "to", add some fraction
 * of "delta" to it.
 *
 * This function is meant to be called only from bufreplace().
 */
void markadjust(from, to, delta)
	MARK	from;	/* old start of text */
	MARK	to;	/* old end of text */
	long	delta;	/* difference between old "to" and new "to" offsets */
{
	MARK	mark;	/* used for scanning the buffer's mark list */
	long	dist;	/* original distance between "from" and "to" */
	long	tooff;	/* original offset of "to" */
	long	fromoff;/* original offset of "from" */

	/* trivial case */
	if (delta == 0)
	{
		return;
	}

	/* Compute some stuff that depends on the "to" mark.  Note that we
	 * must do this before the loop, because during the loop we may adjust
	 * the value of the "to" mark itself!
	 */
	tooff = to->offset;
	fromoff = from->offset;
	dist = tooff - fromoff;
	assert(from->buffer == to->buffer && -delta <= dist && dist >= 0);

	/* for every mark... */
	for (mark = bufmarks(from->buffer); mark; mark = mark->next)
	{
		/* adjust, if affected by mod */
		if (mark->offset > tooff)
		{
			mark->offset += delta;
		}
		else if (mark->offset > fromoff)
		{
			/* NOTE: At this point we know that dist!=0 because
			 * that could only happen if fromoff==tooff, and we
			 * can only get here if this mark's offset <= tooff
			 * but >fromoff.  When tooff==fromoff, that is
			 * impossible!
			 */
			mark->offset += delta * (mark->offset - from->offset) / dist;
		}
	}
}

/* Find the line number of a mark.  This is defined as being the number of
 * newlines preceding the mark, plus 1.  Note that this is not necessarily
 * how the display mode defines lines.
 */
long markline(mark)
	MARK	mark;	/* mark to be converted */
{
#if 1
 static long	lnum;
 static	MARKBUF	prevmark;
 static long	prevchanges;

	/* try to avoid calling lowoffset(), since that function is slow */
	if (markbuffer(mark) == markbuffer(&prevmark)
	 && markoffset(mark) == markoffset(&prevmark)
	 && markbuffer(mark)->changes == prevchanges)
		return lnum;

	/* remember info so we can maybe optimize the next call */
	prevmark = *mark;
	prevchanges = markbuffer(mark)->changes;
#else
	long	lnum;
#endif
	(void)lowoffset(bufbufinfo(markbuffer(mark)), markoffset(mark), NULL, NULL, NULL, &lnum);
	return lnum;
}

/* Adjust a mark so that it points to a specific line, and then return mark.
 * If the requested line doesn't exist, return NULL.
 */
MARK marksetline(mark, linenum)
	MARK	mark;	/* the mark to be adjusted */
	long	linenum;/* the desired line number */
{
	/* if bogus line number, return NULL */
	if (linenum < 1 || linenum > o_buflines(markbuffer(mark)))
	{
		return NULL;
	}

	/* else change the mark's offset & return it */
	mark->offset = lowline(bufbufinfo(markbuffer(mark)), linenum);
	assert(mark->offset < o_bufchars(markbuffer(mark)));
	return mark;
}

/* Change the buffer of a mark.  This involves deleting the mark from the mark
 * list of the old buffer, and then adding it to the list of the new buffer.
 */
#ifdef DEBUG_MARK
void _marksetbuffer(file, line, mark, buffer)
	char	*file;
	int	line;
#else
void marksetbuffer(mark, buffer)
#endif
	MARK	mark;	/* the mark to be moved */
	BUFFER	buffer;	/* the new buffer that the mark should refer to */
{
	MARK	lag;

	/* if no change, then do nothing */
	if (markbuffer(mark) == buffer)
		return;

#ifdef DEBUG_MARK
	{
		WINDOW win = NULL;
		while (win = winofbuf(win, NULL))
			if (win->windowid.value.number == 1
			 && win->cursor == mark)
				fprintf(stderr, "%s:%d: was \"%s\", now \"%s\"\n",
				    file, line,
				    win->cursor->buffer->bufname.value.string,
				    buffer->bufname.value.string);
	}
#endif
	/* remove from old buffer's list of marks */
	if (mark == bufmarks(mark->buffer))
	{
		bufsetmarks(mark->buffer, mark->next);
	}
	else
	{
		assert(bufmarks(mark->buffer));
		for (lag = bufmarks(mark->buffer); lag->next != mark; lag = lag->next)
		{
			assert(lag->next->next);
		}
		lag->next = mark->next;
	}

	/* insert it into the new buffer's list */
	mark->buffer = buffer;
	mark->next = bufmarks(buffer);
	bufsetmarks(buffer, mark);
}
