/* lowbuf.h */
/* Copyright 1995 by Steve Kirkendall */


/* Create a new buffer in the session file */
BEGIN_EXTERNC
extern void	lowinit P_((void (*bufproc)(_BLKNO_ bufinfo, long nchars, long nlines, long changes, long prevloc, CHAR *name)));
extern BLKNO	lowalloc P_((char *name));
extern BLKNO	lowdup P_((_BLKNO_ originfo));
extern void	lowfree P_((_BLKNO_ bufinfo));
extern void	lowtitle P_((_BLKNO_ bufinfo, CHAR *title));
extern long	lowline P_((_BLKNO_ bufinfo, long lineno));
extern BLKNO	lowoffset P_((_BLKNO_ bufinfo, long offset, COUNT *left, COUNT *right, LBLKNO *lptr, long *linenum));
extern long	lowdelete P_((_BLKNO_ dst, long dsttop, long dstbottom));
extern long	lowinsert P_((_BLKNO_ dst, long dsttop, CHAR *newp, long newlen));
extern long	lowreplace P_((_BLKNO_ dst, long dsttop, long dstbottom, CHAR *newp, long newlen));
extern long	lowpaste P_((_BLKNO_ dst, long dsttop, _BLKNO_ src, long srctop, long srcbottom));
extern void	lowflush P_((_BLKNO_ bufinfo));
END_EXTERNC
