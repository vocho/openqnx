/* scan.h */
/* Copyright 1995 by Steve Kirkendall */



/* The following linked list of structs is used to store scan contexts.
 * It is declared publicly only for the sake of the macros; no other modules
 * are expected to access it directly.
 */
extern struct scan_s
{
	struct scan_s	*next;
	BUFFER		buffer;		/* the buffer being scanned */
	_BLKNO_		bufinfo;	/* the blkinfo block of the buffer being scanned */
	_BLKNO_		blkno;		/* block number of currently locked CHARS block, or 0 */
	CHAR		*leftedge;	/* pointer to leftmost locked char */
	CHAR		*rightedge;	/* pointer to char after rightmost locked char */
	CHAR		**ptr;		/* the pointer associated with this scan context */
	long		leoffset;	/* offset of left edge */
#ifdef DEBUG_SCAN
	char		*file;
	int		line;
#endif
} *scan__top;
extern MARKBUF scan__markbuf;



#ifndef DEBUG_SCAN
BEGIN_EXTERNC
extern CHAR	*scanalloc P_((CHAR **cp, MARK start));
extern CHAR	*scanstring P_((CHAR **cp, CHAR *str));
extern CHAR	*scandup P_((CHAR **cp, CHAR **oldp));
END_EXTERNC
#else
# define scanalloc(cp, start)	_scanalloc(__FILE__, __LINE__, (cp), (start))
# define scanstring(cp, str)	_scanstring(__FILE__, __LINE__, (cp), (str))
# define scandup(cp, oldp)	_scandup(__FILE__, __LINE__, (cp), (oldp))
BEGIN_EXTERNC
extern CHAR	*_scanalloc P_((char *file, int line, CHAR **cp, MARK start));
extern CHAR	*_scanstring P_((char *file, int line, CHAR **cp, CHAR *str));
extern CHAR	*_scandup P_((char *file, int line, CHAR **cp, CHAR **oldp));
END_EXTERNC
#endif



#ifndef DEBUG_SCAN

# define scanprev(cp)	(scan__top->leftedge < *(cp) ? --*(cp) : scan__prev(cp))
# define scannext(cp)	(scan__top->rightedge > *(cp) + 1 ? ++*(cp) : scan__next(cp))
# define scanleft(cp)	((COUNT)(*(cp) - scan__top->leftedge))
# define scanright(cp)	((COUNT)(scan__top->rightedge - *(cp)))
# define scanmark(cp)	marktmp(scan__markbuf, scan__top->buffer, scan__top->leoffset + (int)((*cp) - scan__top->leftedge))

#else

# define scanprev(cp)	(scan__top->ptr == (cp) && scan__top->leftedge < *(cp) ? --*(cp) : scan__prev(cp))
# define scannext(cp)	(scan__top->ptr == (cp) && scan__top->rightedge > *(cp) + 1 && *(cp) ? ++*(cp) : scan__next(cp))
# define scanleft(cp)	(scan__top->ptr == (cp) ? (COUNT)(*(cp) - scan__top->leftedge) : scan__left(cp))
# define scanright(cp)	(scan__top->ptr == (cp) ? (COUNT)(scan__top->rightedge - *(cp)) : scan__right(cp))
extern MARK	scanmark P_((CHAR **cp));
extern COUNT	scan__left P_((CHAR **cp));
extern COUNT	scan__right P_((CHAR **cp));
extern void	scan__nobuf P_((void));

#endif



extern void	scanfree P_((CHAR **cp));
extern CHAR	*scanseek P_((CHAR **cp, MARK restart));
extern CHAR	*scan__next P_((CHAR **cp));
extern CHAR	*scan__prev P_((CHAR **cp));
extern CHAR	scanchar P_((MARK mark));
