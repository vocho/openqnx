/* mark.h */
/* Copyright 1995 by Steve Kirkendall */


typedef struct mark_s
{
	struct mark_s	*next;		/* another mark in the same buffer */
	BUFFER		buffer;		/* the buffer that the mark refers to */
	long		offset;		/* the offset of the char within that buffer */
} *MARK, MARKBUF;

#define markbuffer(mark)	((mark)->buffer)
#define markoffset(mark)	((mark)->offset)
#define marksetoffset(mark,o)	((mark)->offset = (o))
#define markaddoffset(mark,o)	((mark)->offset += (o))
#define markdup(mark)		markalloc(markbuffer(mark), markoffset(mark))
#define marktmp(mbuf, buf, off)	((mbuf).buffer = (buf), (mbuf).offset = (off), &(mbuf))
#define markset(mark,newmark)	(marksetbuffer(mark, markbuffer(newmark)), marksetoffset(mark, markoffset(newmark)))

extern MARK namedmark[26];

BEGIN_EXTERNC
extern void markadjust P_((MARK from, MARK to, long delta));
extern long markline P_((MARK mark));
extern MARK marksetline P_((MARK mark, long linenum));
#ifdef DEBUG_MARK
#define marksetbuffer(m,b)	_marksetbuffer(__FILE__, __LINE__, (m), (b))
extern void _marksetbuffer P_((char *file, int line, MARK mark, BUFFER buffer));
#else
extern void marksetbuffer P_((MARK mark, BUFFER buffer));
#endif
END_EXTERNC

#ifndef DEBUG_ALLOC
BEGIN_EXTERNC
extern MARK markalloc P_((BUFFER buffer, long offset));
extern void markfree P_((MARK mark));
END_EXTERNC
#else
BEGIN_EXTERNC
extern MARK _markalloc P_((char *file, int line, BUFFER buffer, long offset));
extern void _markfree P_((char *file, int line, MARK mark));
END_EXTERNC
#define markalloc(b,o)	_markalloc(__FILE__, __LINE__, b, o)
#define markfree(m)	_markfree(__FILE__, __LINE__, m)
#endif
