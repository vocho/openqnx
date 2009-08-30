/* unix/osblock.c */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#ifndef S_ISDIR
# define S_ISDIR(mode)	((mode & 0170000) == 0040000)
#endif
#include "elvis.h"
#ifdef FEATURE_RCSID
char id_osblock[] = "$Id: osblock.c,v 2.30 2003/10/17 17:41:23 steve Exp $";
#endif
#ifndef DEFAULT_SESSION
# define DEFAULT_SESSION "%s/elvis%d.ses"
#endif
#ifndef F_OK
# define F_OK	0
#endif
#ifndef W_OK
# define W_OK	2
#endif
#ifndef O_BINARY
# ifdef _O_BINARY
#  define O_BINARY _O_BINARY
# else
#  define O_BINARY 0
# endif
#endif


static int fd = -1; /* file descriptor of the session file */
#ifdef FEATURE_RAM
static BLK **blklist;
static int nblks;
#endif

/* This function creates a new block file, and returns ElvTrue if successful,
 * or ElvFalse if failed because the file was already busy.
 */
ELVBOOL blkopen(force, buf)
	ELVBOOL	force;		/* if ElvTrue, open even if "in use" flag set */
	BLK	*buf;		/* buffer, holds SUPER block */
{
 static char	dfltname[100];
	char	dir[80];
	struct stat st;
	int	i, j;
	long	oldcount;

#ifdef FEATURE_RAM
	if (o_session && !CHARcmp(o_session, toCHAR("ram")))
	{
		nblks = 1024;
		blklist = (BLK **)calloc(nblks, sizeof(BLK *));
		blklist[0] = (BLK *)malloc(o_blksize);
		memcpy(blklist[0], buf, o_blksize);
		return ElvTrue;
	}
#endif

	/* If no session file was explicitly requested, try successive
	 * defaults until we find an existing file (if we're trying to
	 * recover) or a non-existent file (if we're not trying to recover).
	 */
	if (!o_session)
	{
		/* search through sessionpath for a writable directory */
		if (!o_sessionpath)
			o_sessionpath = toCHAR("~:.");
		for (i = 0, *dir = '\0'; o_sessionpath[i] && !*dir; )
		{
			/* copy next name from o_sessionpath to dfltname */
			j = 0;
			if (o_sessionpath[i] == '~' && !elvalnum(o_sessionpath[i + 1]))
			{
				strcpy(dir, tochar8(o_home));
				j = strlen(dir);
				i++;
			}
			while (o_sessionpath[i] && o_sessionpath[i] != ':')
			{
				dir[j++] = o_sessionpath[i++];
			}
			dir[j] = '\0';
			if (j == 0)
				strcpy(dir, ".");
			if (o_sessionpath[i] == ':')
				i++;

			/* If not writable directory, forget it */
			if (stat(dir, &st) != 0
			 || !S_ISDIR(st.st_mode)
			 || !(st.st_uid == geteuid()
			 	? (st.st_mode & S_IRWXU) == S_IRWXU
			 	: st.st_gid == getegid()
			 		? (st.st_mode & S_IRWXG) == S_IRWXG
			 		: (st.st_mode & S_IRWXO) == S_IRWXO))
			{
				*dir = '\0';
			}
		}
		if (!*dir)
		{
			msg(MSG_FATAL, "set SESSIONPATH to a writable directory");
		}
		optpreset(o_directory, CHARkdup(toCHAR(dir)), OPT_FREE);

		/* choose the name of a session file */
		i = 1;
		oldcount = 0;
		do
		{
			/* protect against trying a ridiculous number of names */
			if (i >= 1000)
			{
				msg(MSG_FATAL, o_recovering
					? "[s]no session file found in $1"
					: "[s]too many session files in $1", dir);
			}
			sprintf(dfltname, DEFAULT_SESSION, dir, i++);

			/* if the file exists and is writable by this user,
			 * and we aren't recovering, then remember it so we
			 * can print a warning later, so the user will know
			 * he should delete it or recover it eventually.
			 */
			if (!o_recovering && access(dfltname, W_OK) == 0)
			{
				oldcount++;
			}

			/* if user wants to cancel, then fail */
			if (chosengui->poll && (*chosengui->poll)(ElvFalse))
			{
				return ElvFalse;
			}
		} while (o_recovering ? (access(dfltname, F_OK) != 0)
				      : ((fd = open(dfltname, O_RDWR|O_CREAT|O_EXCL|O_BINARY, 0600)) < 0));
		o_session = toCHAR(dfltname);
		o_tempsession = ElvTrue;
		if (oldcount > 0)
			msg(MSG_WARNING, "[d]skipping $1 old session file($1!=1?\"s\")", oldcount);
	}

	/* Try to open the session file (if not opened in the above loop) */
	if (fd < 0 && (fd = open(tochar8(o_session), O_RDWR|O_BINARY)) >= 0)
	{
		/* we're opening an existing session -- definitely not temporary */
		o_tempsession = ElvFalse;
	}
	else
	{
		/* either we're about to open an existing session that was
		 * explicitly named via "-f session", or we have already
		 * created a temporary session and just need to initialize it.
		 */

		/* if we don't have a temp session already open, then we must
		 * want to create the session file now.
		 */
		if (fd < 0 && errno == ENOENT)
			fd = open(tochar8(o_session), O_RDWR|O_CREAT|O_EXCL|O_BINARY, 0600);
		if (fd < 0)
			msg(MSG_FATAL, "no such session");

		/* either way, we now have an open session.  Initialize it! */
		o_newsession = ElvTrue;
		if (write(fd, (char *)buf, (unsigned)o_blksize) < o_blksize)
		{
			close(fd);
			unlink(tochar8(o_session));
			fd = -1;
			errno = ENOSPC;
		}
		else
		{
			lseek(fd, 0L, 0);
		}
	}

	/* if elvis runs other programs, they shouldn't inherit this fd */
	fcntl(fd, F_SETFL, 1);

	/* Read the first block & mark the session file as being "in use".
	 * If already marked as "in use" and !force, then fail.
	 */
	/* lockf(fd, LOCK, sizeof buf->super); */
	if (read(fd, (char *)buf, sizeof buf->super) != sizeof buf->super)
	{
		msg(MSG_FATAL, "blkopen's read failed");
	}
	if (buf->super.inuse && !force)
	{
		/* lockf(fd, ULOCK, o_blksize); */
		return ElvFalse;
	}
	buf->super.inuse = getpid();
	lseek(fd, 0L, 0);
	(void)write(fd, (char *)buf, sizeof buf->super);
	/* lockf(fd, ULOCK, o_blksize); */

	/* done! */
	return ElvTrue;
}


/* This function closes the session file, given its handle */
void blkclose(buf)
	BLK	*buf;	/* buffer, holds superblock */
{
	if (fd < 0)
		return;
	blkread(buf, 0);
	buf->super.inuse = 0L;
	blkwrite(buf, 0);
	close(fd);
	fd = -1;
	if (o_tempsession)
	{
		unlink(tochar8(o_session));
	}
}

/* Write the contents of buf into record # blkno, for the block file
 * identified by blkhandle.  Blocks are numbered starting at 0.  The
 * requested block may be past the end of the file, in which case
 * this function is expected to extend the file.
 */
void blkwrite(buf, blkno)
	BLK	*buf;	/* buffer, holds contents of block */
	_BLKNO_	blkno;	/* where to write it */
{
#ifdef FEATURE_RAM
	/* store it in RAM */
	if (nblks > 0)
	{
		if (blkno >= nblks)
		{
			blklist = (BLK **)realloc(blklist,
						(nblks + 1024) * sizeof(BLK *));
			memset(&blklist[nblks], 0, 1024 * sizeof(BLK *));
			nblks += 1024;
		}
		if (!blklist[blkno])
			blklist[blkno] = malloc(o_blksize);
		memcpy(blklist[blkno], buf, o_blksize);
		return;
	}
#endif

	/* write the block */
	lseek(fd, (off_t)blkno * (off_t)o_blksize, 0);
	if (write(fd, (char *)buf, (size_t)o_blksize) != o_blksize)
	{
		msg(MSG_FATAL, "blkwrite failed");
	}
}

/* Read the contends of record # blkno into buf, for the block file
 * identified by blkhandle.  The request block will always exist;
 * it will never be beyond the end of the file.
 */
void blkread(buf, blkno)
	BLK	*buf;	/* buffer, where buffer should be read into */
	_BLKNO_	blkno;	/* where to read from */
{
#ifdef FEATURE_RAM
	if (nblks > 0)
	{
		memcpy(buf, blklist[blkno], o_blksize);
		return;
	}
#endif

	/* read the block */
	lseek(fd, (off_t)blkno * o_blksize, 0);
	if (read(fd, (char *)buf, (size_t)o_blksize) != o_blksize)
	{
		msg(MSG_FATAL, "[d]blkread($1) failed", (int)blkno);
	}
}

/* Force changes out to disk.  Ideally we would only force the session file's
 * blocks out to the disk, but UNIX doesn't offer a way to do that, so we
 * force them all out.  Major bummer.
 */
void blksync()
{
#ifdef FEATURE_RAM
	if (nblks > 0)
		return;
#endif

	sync();
}
