/* opsys.h */
/* Copyright 1995 by Steve Kirkendall */



typedef enum
{
	DIR_INVALID,	/* malformed filename (can't happen with UNIX) */
	DIR_BADPATH,	/* unable to check file */
	DIR_NOTFILE,	/* file exists but is neither normal nor directory */
	DIR_DIRECTORY,	/* file is a directory */
	DIR_NEW,	/* file doesn't exist yet */
	DIR_UNREADABLE,	/* file exists but is unreadable */
	DIR_READONLY,	/* file is readable but not writable */
	DIR_READWRITE	/* file is readable and writable. */
} DIRPERM;

BEGIN_EXTERNC
extern ELVBOOL	blkopen P_((ELVBOOL force, BLK *buf));
extern void	blkclose P_((BLK *buf));
extern void	blkwrite P_((BLK *buf, _BLKNO_ blkno));
extern void	blkread P_((BLK *buf, _BLKNO_ blkno));
extern void	blksync P_((void));

extern char	*dirfirst P_((char *wildexpr, ELVBOOL ispartial));
extern char	*dirnext P_((void));
extern ELVBOOL	diriswild P_((char *wildexpr));
extern ELVBOOL	dirwildcmp P_((char *fname, char *wild));
extern DIRPERM	dirperm P_((char *filename));
extern char	*dirdir P_((char *pathname));
extern char	*dirfile P_((char *pathname));
extern char	*dirtime P_((char *filename));
extern char	*dirpath P_((char *dir, char *file));
extern char	*dircwd P_((void));
extern ELVBOOL	dirchdir P_((char *pathname));

extern ELVBOOL	prgopen P_((char *command, ELVBOOL willwrite, ELVBOOL willread));
extern int	prgwrite P_((CHAR *buf, int nbytes));
extern ELVBOOL	prggo P_((void));
extern int	prgread P_((CHAR *buf, int nbytes));
extern int	prgclose P_((void));

extern int	txtopen P_((char *filename, _char_ rwa, ELVBOOL binary));
extern void	txtclose P_((void));
extern int	txtwrite P_((CHAR *buf, int nbytes));
extern int	txtread P_((CHAR *buf, int nbytes));

#if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)
typedef struct
{
	int	fd;		/* file descriptor of a socket to read from */
	int	left;		/* number of chars used from buf */
	int	right;		/* total number of chars in buf */
	char	buf[4096];	/* buffer */
} sockbuf_t;

#define netbuffer(sb)		((sb)->buf + (sb)->left)
#define netbytes(sb)		((sb)->right - (sb)->left)
#define netconsume(sb, n)	((sb)->left += (n))

sockbuf_t *netconnect P_((char *site_port, unsigned int defport));
void	  netdisconnect P_((sockbuf_t *sb));
ELVBOOL	  netread P_((sockbuf_t *sb));
char	  *netgetline P_((sockbuf_t *sb));
ELVBOOL	  netwrite P_((sockbuf_t *sb, char *data, int len));
ELVBOOL	  netputline P_((sockbuf_t *sb, char *command, char *arg1, char *arg2));
char	  *netself P_((void));
#endif

#ifdef OSINIT
extern void	osinit P_((char *argv0));
#endif

#if ANY_UNIX
extern char *expanduserhome P_((char *pathname, char *dest));
#endif

END_EXTERNC
