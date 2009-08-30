/* session.h */
/* Copyright 1995 by Steve Kirkendall */


/* This file contains macros, tyepdefs, and extern declarations pertaining to
 * session files.  This includes the session file format, block cacheing,
 * and block allocation.
 */

/*----------------------------------------------------------------------------*/
/* compile-time configurable constants                                        */

#ifndef BLKSIZE
# define BLKSIZE	2048	/* default block size */
#endif

#ifndef BLKHASH
# define BLKHASH	61	/* default hash factor (a prime number) */
#endif

#ifndef BLKCACHE
# define BLKCACHE	20	/* default size of block cache */
#endif

#ifndef BLKGROW
# define BLKGROW	8	/* default amount by which block table grows */
#endif

/*----------------------------------------------------------------------------*/
/* session file format                                                        */

#define SESSION_MAGIC			0x0200DEADL
#define SESSION_MAGIC_BYTESWAPPED	0xADDE0002L

/* These data types are used to represent physical and logical block numbers.
 * BLKNO is a physical block number; it is used to compute an offset into the
 * session file.  LBLKNO is a logical block number; it is an index into a
 * blklist block for a given buffer.  DON'T CONFUSE ONE FOR THE OTHER.
 */
typedef COUNT BLKNO;
typedef COUNT LBLKNO;
typedef _COUNT_ _BLKNO_;
typedef _COUNT_ _LBLKNO_;
typedef enum { SES_NEW, SES_SUPER, SES_SUPER2, SES_BUFINFO, SES_BLKLIST, SES_CHARS } BLKTYPE;

typedef union
{
	struct
	{
		long	magic;		/* file type code: 0x0200DEAD */
		long	inuse;		/* in-use flag */
		COUNT	blksize;	/* bytes per block */
		BLKNO	next;		/* super2 block that continues buf[] */
		BLKNO	buf[1];		/* where each buffer is described */
	} super;

	struct
	{
		BLKNO	next;		/* super2 block that continues buf[] */
		BLKNO	buf[1];		/* continuation of super.buf[] */
	} super2;

	struct
	{
		long	changes;	/* change counter */
		long	prevloc;	/* where last change was made */
		long	reserved;	/* nothing yet; always 0 */
		BLKNO	first;		/* first text block list */
		short	checksum;	/* checksum of this block */
		char	name[1];	/* buffer name */
	} bufinfo;

	struct
	{
		BLKNO	next;		/* ref to next block of BLKNO values */
		struct blki_s
		{
			BLKNO	blkno;	/* which block stores next CHARs */
			COUNT	nchars;	/* number of CHARs used in the block */
			COUNT	nlines;	/* number of '\n' CHARs in the block */
		}	blk[1];		/* list of blocks in this buffer */
	} blklist;

	struct
	{
		CHAR	chars[1];	/* the bytes themselves */
	} chars;

	short	sumshorts[BLKSIZE / sizeof(short)]; /* for calculating checksum */

	char	sizetester[BLKSIZE];	/* forces BLK to be prefered size */
} BLK;

#define SES_MAXSUPER	((o_blksize - (int)(((BLK *)0)->super.buf)) / sizeof(BLKNO))
#define SES_MAXSUPER2	((o_blksize - (int)(((BLK *)0)->super2.buf)) / sizeof(BLKNO))
#define SES_MAXBUFINFO	(o_blksize - (int)(((BLK *)0)->bufinfo.name))
#define SES_MAXBLKLIST	((o_blksize - (int)(((BLK *)0)->blklist.blk)) / sizeof(((BLK *)0)->blklist.blk[0]))
#define SES_MAXCHARS	(o_blksize / sizeof(CHAR))


/*----------------------------------------------------------------------------*/
/* low-level session file access functions                                    */

BEGIN_EXTERNC
extern void	sesopen P_((ELVBOOL force));
extern void	sesclose P_((void));
extern BLK	*sesblk P_((_BLKNO_));
extern void	sesunlock P_((_BLKNO_ blkno, ELVBOOL forwrite));
extern void	sesflush P_((_BLKNO_ blkno));
extern void	sessync P_((void));
END_EXTERNC
#ifdef DEBUG_SESSION
# define seslock(b,f,t)	_seslock(__FILE__, __LINE__, b, f, t)
# define sesalloc(b,t)	_sesalloc(__FILE__, __LINE__, b, t)
# define sesfree(b)	_sesfree(__FILE__, __LINE__, b)
BEGIN_EXTERNC
extern BLKNO	_seslock P_((char *file, int line, _BLKNO_ blkno, ELVBOOL forwrite, BLKTYPE blktype));
extern BLKNO	_sesalloc P_((char *file, int line, _BLKNO_ blkno, BLKTYPE blktype));
extern void	_sesfree P_((char *file, int line, _BLKNO_ blkno));
END_EXTERNC
#else
BEGIN_EXTERNC
extern BLKNO	seslock P_((_BLKNO_ blkno, ELVBOOL forwrite, BLKTYPE blktype));
extern BLKNO	sesalloc P_((_BLKNO_ blkno, BLKTYPE blktype));
extern void	sesfree P_((_BLKNO_ blkno));
END_EXTERNC
#endif
