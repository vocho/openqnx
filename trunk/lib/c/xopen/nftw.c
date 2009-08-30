/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */





#define _FILE_OFFSET_BITS	32
#define _LARGEFILE64_SOURCE	1
#include <alloca.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/iomsg.h>
#include <sys/neutrino.h>
#include <sys/stat.h>
#include <unistd.h>
#include "connect.h"

/*
 *  As the POSIX specification allows the user to limit the number
 *  of file descriptors used in the tree walk, this code implements
 *  two alternative mechanisms for doing this.  One involves using
 *  telldir()/seekdir() to reposition the directory stream after
 *  closing it to release an fd; the other involves allocating a
 *  memory buffer to cache the remainder of the directory before
 *  closing it to release an fd.  Note that under QNX, pathname
 *  resolution and union directories mean it is more expensive to
 *  use the re-positioning method, so the caching is the default.
 *  Also note that due to the micro-kernel architecture, some
 *  operations (such as stat()) actually require a file descriptor;
 *  also with the union filesystem one directory may require
 *  multiple file descriptors.  So we can't actually exactly
 *  implement the fd limit anyway, but best-effort is made (and in
 *  some cases, non-union directory with embedded stat, is correct).
 */
#undef FTW_USE_SEEKDIR

/*
 *  As the implementation is a recursive-descent, it is possible
 *  for deep directory hierarchies to need some stack space, so
 *  this can be optionally checked at each level.  Although the
 *  internal requirements are low (200-300 bytes), support routines,
 *  in particular the _connect_*() messaging and malloc() bands, have
 *  heavy stack requirements.  An undocumented implementation detail
 *  of _connect_ctrl() determines if malloc/alloca is used; attempt
 *  to second-guess what stack requirements this will need.
 */
#define FTW_CHECK_STACK
#ifdef FTW_CHECK_STACK
#define FTW_STACK_REQUIREMENTS()	(_connect_malloc ? 1792 : 4096)
#endif

#define FTW_NFTW	0x01		/* nftwXXX() variant (nftw,nftw64) */
#define FTW_64BIT	0x02		/* XXX64() variant (ftw64,nftw64)  */

struct object {
	ino64_t				ino;
	dev_t				dev;
	struct object		*link;
};

struct directory {
	DIR					*dir;
#ifdef FTW_USE_SEEKDIR
	long				posn;
#else
	struct dirent64		*contents;
	int					size;
#endif
	struct directory	*link;
};

struct nftw {
	unsigned			flags;
	int					limit;
	int					depth;
	dev_t				fsys;
	char				*pathname;
	int					size;
	int					stem;
	char				*cwd;
	int					(*invoke)(struct nftw *, struct stat64 *, int);
	int					(*callback)();
	int					(*statfn)(), (*lstatfn)();
	void				*(*readdirfn)();
	struct object		*known;
	struct directory	*dirs;
	int					action;
	int					error;
};

/* Recursive directory processing */
static int directory(struct nftw *, struct stat64 *);
static int process(struct nftw *, struct dirent64 *);

static __inline int invokeftw(struct nftw *state, struct stat64 *st, int type)
{
/* nftw() values:	          FTW_F, FTW_D, FTW_DNR, FTW_DP, FTW_NS, FTW_SL, FTW_SLN */
static const int	map[] = { FTW_F, FTW_D, FTW_DNR, FTW_D,  FTW_NS, FTW_F,  FTW_NS };
int					result;

	if ((result = (*state->callback)(state->pathname, st, map[type])) != 0)
		state->error = errno;
	return(result);
}
static __inline int invokenftw(struct nftw *state, struct stat64 *st, int type)
{
struct FTW		args;
int				result;

	args.base = state->stem, args.level = state->depth, args.quit = -1;
	if ((result = (*state->callback)(state->pathname, st, type, &args)) != 0)
		state->error = errno;
	else
		state->action = args.quit;
	return(result);
}

static __inline int cd(struct nftw *state, char *truncatep)
{
int		status;
char	ch;

	ch = *truncatep, *truncatep = '\0';
	if ((status = chdir(state->pathname)) == -1)
		state->error = errno;
	*truncatep = ch;
	return(status);
}

static __inline int direntry(struct nftw *state, struct directory *dir, struct dirent64 **dp)
{
	errno = EOK;
	if ((*dp = (*state->readdirfn)(dir->dir)) != NULL || errno == EOK)
		return(0);
	state->error = errno;
	return(-1);
}

static char *tidypath(const char *path, int bufferlen)
{
char					*buffer;
struct _connect_ctrl	ctrl;
struct _io_connect		msg;		
int						fd, error;

	if ((buffer = malloc(bufferlen)) != NULL) {
		memset(&ctrl, 0x00, sizeof(struct _connect_ctrl));
		ctrl.base = _NTO_SIDE_CHANNEL;
		ctrl.path = buffer, ctrl.pathsize = bufferlen;
		ctrl.send = MsgSendvnc;
		ctrl.msg = memset(&msg, 0x00, sizeof(struct _io_connect));
		msg.ioflag = O_LARGEFILE | O_NOCTTY, msg.mode = S_IFLNK;
		msg.subtype = _IO_CONNECT_COMBINE_CLOSE;
		if ((fd = _connect_ctrl(&ctrl, path, 0, NULL)) != -1) {
			ConnectDetach(fd);
			if (*buffer != '\0')
				return(buffer);
			errno = ENAMETOOLONG;
		}
		error = errno, free(buffer), errno = error;
	}
	else {
		errno = ENOMEM;
	}
	return(NULL);
}

static int setup(struct nftw *state, const char *path, void *fn, int depth, int flags, short isnftw, short is64)
{
char	*cp;

	state->error = errno, state->action = -1;
	state->callback = fn;
	state->flags = flags;
	state->limit = (depth >= 1) ? depth : 1;
	state->depth = 0, state->dirs = NULL;
	state->invoke = isnftw ? invokenftw : invokeftw;
	state->lstatfn = is64 ? (void *)lstat64 : (void *)lstat;
	state->statfn = (flags & FTW_PHYS) ? state->lstatfn : is64 ? (void *)stat64 : (void *)stat;
	state->readdirfn = is64 ? (void *)readdir64 : (void *)readdir;
	state->known = NULL, state->cwd = NULL;
	if (!(flags & FTW_CHDIR) || (state->cwd = getcwd(NULL, PATH_MAX)) != NULL) {
		if ((state->pathname = tidypath(path, state->size = PATH_MAX)) != NULL) {
			state->stem = ((cp = strrchr(state->pathname, '/') + 1) == &state->pathname[1] && *cp == '\0') ? 0 : cp - state->pathname;
			if (!(flags & FTW_CHDIR) || cd(state, cp) == 0) {
				return(EOK);
			}
			free(state->pathname);
		}
		else {
			state->error = errno;
		}
		free(state->cwd);
	}
	else {
		state->error = errno;
	}
	return(-1);
}
static void cleanup(struct nftw *state)
{
struct object	*o;

	while ((o = state->known) != NULL) {
		state->known = o->link;
		free(o);
	}
	if (state->cwd != NULL) {
		(void)chdir(state->cwd);
		free(state->cwd);
	}
	free(state->pathname);
}

static int unique(struct nftw *state, dev_t dev, ino64_t ino)
{
struct object	*o;

	if (!(state->flags & FTW_PHYS)) {
		for (o = state->known; o != NULL; o = o->link)
			if (dev == o->dev && ino == o->ino)
				return(0);
	}
	return(!0);
}
static int visit(struct nftw *state, dev_t dev, ino64_t ino)
{
struct object	*o;

	if (!(state->flags & FTW_PHYS)) {
		if ((o = malloc(sizeof(struct object))) == NULL)
			return(0);
		o->dev = dev, o->ino = ino;
		o->link = state->known, state->known = o;
	}
	return(!0);
}

static int spillfds(struct nftw *state)
{
struct directory	*d;
#ifndef FTW_USE_SEEKDIR
struct dirent64		*dp, *grow;
int					len, result;
#endif

	for (d = state->dirs; d->dir == NULL; d = d->link)
		;
#ifdef FTW_USE_SEEKDIR
	if ((d->posn = telldir(d->dir)) == -1)
		return(-1);
#else
	(void)dircntl(d->dir, D_SETFLAG, dircntl(d->dir, D_GETFLAG) & ~D_FLAG_STAT);
	for (len = 0; (result = direntry(state, d, &dp)) == 0 && dp != NULL; len += dp->d_reclen) {
		if (len + dp->d_reclen > d->size) {
			if ((grow = realloc(d->contents, d->size += PATH_MAX)) == NULL) {
				free(d->contents), d->contents = NULL, d->size = 0;
				return(-1);
			}
			d->contents = grow;
		}
		memcpy((char *)d->contents + len, dp, dp->d_reclen);
	}
	if (result != 0) {
		free(d->contents), d->contents = NULL, d->size = 0;
		return(-1);
	}
	d->contents = (len != 0) ? realloc(d->contents, d->size = len) : NULL;
#endif
	closedir(d->dir), d->dir = NULL;
	++state->limit;
	return(0);
}
#ifdef FTW_USE_SEEKDIR
static int unspillfds(struct nftw *state, char *truncatep)
{
struct directory	*d;
char				ch;

	ch = *truncatep, *truncatep = '\0';
	(d = state->dirs)->dir = opendir(state->pathname);
	*truncatep = ch;
	if (d->dir != NULL) {
		--state->limit;
		seekdir(d->dir, d->posn);
		dircntl(d->dir, D_SETFLAG, dircntl(d->dir, D_GETFLAG) | D_FLAG_STAT);
		return(0);
	}
	state->error = errno;
	return(-1);
}
#endif
static int enterdir(struct nftw *state)
{
struct directory	*d;

	if ((d = malloc(sizeof(struct directory))) != NULL) {
		if (state->limit > 0 || spillfds(state) == 0) {
			if ((d->dir = opendir(state->pathname)) != NULL) {
				--state->limit;
				(void)dircntl(d->dir, D_SETFLAG, dircntl(d->dir, D_GETFLAG) | D_FLAG_STAT);
#ifdef FTW_USE_SEEKDIR
				d->posn = 0;
#else
				d->contents = NULL, d->size = 0;
#endif
				d->link = state->dirs, state->dirs = d;
				return(0);
			}
			else {
				state->error = errno;
			}
		}
		else {
			state->error = EMFILE;
		}
		free(d);
	}
	else {
		state->error = ENOMEM;
	}
	return(-1);
}
static void exitdir(struct nftw *state)
{
struct directory	*d;

	d = state->dirs, state->dirs = d->link;
	if (d->dir != NULL) {
		closedir(d->dir);
		++state->limit;
	}
#ifndef FTW_USE_SEEKDIR
	free(d->contents);
#endif
	free(d);
}

static __inline int gettype(struct nftw *state, struct dirent64 *dp, struct stat64 *st)
{
struct dirent_extra_stat	*extra;
int							embed;

	for (embed = 0, extra = (struct dirent_extra_stat *)_DEXTRA_FIRST(dp); _DEXTRA_VALID(extra, dp); 
			extra = (struct dirent_extra_stat *)_DEXTRA_NEXT(extra)) {
		if ((embed = ((extra->d_type == _DTYPE_STAT && !(state->flags & FTW_PHYS)) || (extra->d_type == _DTYPE_LSTAT && ((state->flags & FTW_PHYS) || !S_ISLNK(extra->d_stat.st_mode)))) != 0))
			break;
	}
	if (embed)
		memcpy(st, &extra->d_stat, sizeof(struct stat64));
	else if ((*state->statfn)(state->pathname, st) == -1)
		return((errno != EACCES && errno != ENOENT) ? -1 : (!(state->flags & FTW_PHYS) && (*state->lstatfn)(state->pathname, st) == 0 && S_ISLNK(st->st_mode)) ? FTW_SLN : FTW_NS);
	return(S_ISDIR(st->st_mode) ? FTW_D : S_ISLNK(st->st_mode) ? FTW_SL : FTW_F);
}

static int process(struct nftw *state, struct dirent64 *dp)
{
char			*grow;
struct stat64	st;
int				type;

	if ((dp->d_namelen == 1 && dp->d_name[0] == '.') || (dp->d_namelen == 2 && dp->d_name[0] == '.' && dp->d_name[1] == '.'))
		return(0);
	if (state->stem + 1 + dp->d_namelen + 1 > state->size) {
		if ((grow = realloc(state->pathname, state->size + PATH_MAX)) == NULL)
			return(state->error = ENAMETOOLONG, -1);
		state->pathname = grow, state->size += PATH_MAX;
	}
	sprintf(&state->pathname[state->stem - 1], "/%.*s", dp->d_namelen, dp->d_name);
	if ((type = gettype(state, dp, &st)) == -1)
		return(state->error = errno, -1);
	if (type != FTW_NS && (state->flags & FTW_MOUNT) && st.st_dev != state->fsys)
		return(0);
	if (type != FTW_D)
		return((*state->invoke)(state, &st, type));
	if (!unique(state, st.st_dev, st.st_ino))
		return(0);
#ifdef FTW_CHECK_STACK
	if (__stackavail() < FTW_STACK_REQUIREMENTS())
		return(state->error = ENOMEM, -1);
#endif
	if (!visit(state, st.st_dev, st.st_ino))
		return(state->error = ENOMEM, -1);
	return(directory(state, &st));
}
static int directory(struct nftw *state, struct stat64 *st)
{
struct dirent64	*dp;
#ifndef FTW_USE_SEEKDIR
struct dirent64	*cached;
#endif
int				prefix, result;

	if ((result = enterdir(state)) == 0) {
		if ((state->flags & FTW_DEPTH) || ((result = (*state->invoke)(state, st, (!(state->flags & FTW_CHDIR) || eaccess(state->pathname, X_OK) != -1) ? FTW_D : FTW_DNR)) == 0 && state->action != FTW_SKD)) {
			if (!(state->flags & FTW_CHDIR) || (result = chdir(state->pathname)) == 0) {
				state->stem += strlen(&state->pathname[prefix = state->stem]) + (state->stem != 0 ? 1 : 0);
				++state->depth;
#ifdef FTW_USE_SEEKDIR
				while ((result = direntry(state, state->dirs, &dp)) == 0 && dp != NULL) {
					if ((result = process(state, dp)) != 0 || state->action == FTW_SKR)
						break;
					if (state->dirs->dir == NULL && (result = unspillfds(state, &state->pathname[(state->stem > 1) ? state->stem - 1 : 1])) != 0)
						break;
				}
#else
				while ((result = direntry(state, state->dirs, &dp)) == 0 && dp != NULL) {
					if ((result = process(state, dp)) != 0 || state->action == FTW_SKR)
						break;
					if (state->dirs->dir == NULL) {
						if ((cached = state->dirs->contents) != NULL) {
							do {
								if ((result = process(state, cached)) != 0 || state->action == FTW_SKR)
									break;
							} while ((cached = (struct dirent64 *)((char *)cached + cached->d_reclen)) < (struct dirent64 *)((char *)state->dirs->contents + state->dirs->size));
						}
						break;
					}
				}
#endif
				--state->depth;
				state->pathname[(state->stem > 1) ? state->stem - 1 : 1] = '\0', state->stem = prefix;
				if (result == 0 && (!(state->flags & FTW_CHDIR) || (result = cd(state, &state->pathname[(prefix > 1) ? prefix - 1 : 1])) == 0)) {
					if (state->flags & FTW_DEPTH)
						result = (*state->invoke)(state, st, FTW_DP);
				}
			}
			else if (errno == EACCES) {
				result = (state->flags & FTW_DEPTH) ? (*state->invoke)(state, st, FTW_DNR) : 0;
			}
			else {
				state->error = errno;
			}
		}
		exitdir(state);
	}
	else if (state->error == EACCES) {
		result = (*state->invoke)(state, st, FTW_DNR);
	}
	return(result);
}

static int filetreewalk(int behaviour, const char *path, void *fn, int depth, int flags)
{
struct nftw		state;
struct stat64	st;
int				result;

	if (path == NULL || *path == '\0') {
		state.error = ENOENT, result = -1;
	}
	else if ((result = setup(&state, path, fn, depth, flags, behaviour & FTW_NFTW, behaviour & FTW_64BIT)) == 0) {
		if ((*(state.statfn))(state.pathname, &st) == -1) {
			if (!(flags & FTW_PHYS) && errno == ENOENT && (*state.lstatfn)(state.pathname, &st) != -1 && S_ISLNK(st.st_mode)) {
				result = (*state.invoke)(&state, &st, FTW_SLN);
			}
			else {
				state.error = errno, result = -1;
			}
		}
		else {
			if (!S_ISDIR(st.st_mode)) {
				result = (*state.invoke)(&state, &st, S_ISLNK(st.st_mode) ? FTW_SL : FTW_F);
			}
#ifdef FTW_CHECK_STACK
			else if (__stackavail() < FTW_STACK_REQUIREMENTS()) {
				state.error = ENOMEM, result = -1;
			}
#endif
			else if (visit(&state, state.fsys = st.st_dev, st.st_ino)) {
				result = directory(&state, &st);
			}
			else {
				state.error = ENOMEM, result = -1;
			}
		}
		cleanup(&state);
	}
	return(errno = state.error, result);
}

int ftw(const char *path, int (*fn)(const char *, const struct stat *, int), int ndirs)
{
	return(filetreewalk(0, path, fn, ndirs, 0));
}
int ftw64(const char *path, int (*fn)(const char *, const struct stat64 *, int), int ndirs)
{
	return(filetreewalk(FTW_64BIT, path, fn, ndirs, 0));
}
int nftw(const char *path, int (*fn)(const char *, const struct stat *, int, struct FTW *), int depth, int flags)
{
	return(filetreewalk(FTW_NFTW, path, fn, depth, flags));
}
int nftw64(const char *path, int (*fn)(const char *, const struct stat64 *, int, struct FTW *), int depth, int flags)
{
	return(filetreewalk(FTW_NFTW | FTW_64BIT, path, fn, depth, flags));
}

__SRCVERSION("nftw.c $Rev: 156761 $");
