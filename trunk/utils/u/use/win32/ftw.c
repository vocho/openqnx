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





/*
sysV filetree walker
Notes for configuring,

The option QNX_EXTENSION is used to disable/enable stat optimization
on reading directories.  When turned on, it checks to stat field in the
dirent struct before attempting a stat on the file.  When turned off,
it ignores the dirent struct and always stats it.

The option USE_ACCESS is used to determine whether or not to use
access() to check permissions on the file(s).  It is presumed that
access is much more expensive than gete[ug]id(), so since it has the
stat structure It just figures it out itself.  A potential problem
lies in an application which switches effective uid/gid in a signal
handler, and even so, the window is tiny.....

Some timings -- a 474 remote filesystem 3 levels deep, starting at
/usr/home/steve, run across a 2.5Mbit arcnet lan.

QNX_EXTENSION   USE_ACCESS   TIME
0               1            22.89s
0               0            18.95s
1               1             7.19s
1               0             3.28s


*/

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <lib/compat.h>
#include <ftw.h>

#define DBGPRT printf

#ifndef DDEBUG
#define	DDEBUG 0
#endif

#ifndef USE_ACCESS
#define	USE_ACCESS 0
#endif

#ifndef QNX_EXTENSION
#define	QNX_EXTENSION 0
#endif

/* avoid calls to access (potentially expensive) by checking UIDS ourselves */
#if !USE_ACCESS

static int xperms(struct stat * p)
{
    uid_t uid = geteuid();

    if (uid == 0) return 7;
    if (uid == p->st_uid) return (p->st_mode >> 6) & 07;
#ifndef __MINGW32__
    if (getegid() == p->st_gid) return (p->st_mode >> 3) & 07;
#endif
    return p->st_mode & 07;
}

#endif


#define dostat(F,S)	(stat((F),(S)) == -1 ? NULL : (S))

/* the QNX get_dir may return the 'stat' structure if the elements are right,
   i.e., right type of file, and appropriate permissions
*/
#if QNX_EXTENSION
#define	get_stat(D,S,F) (((D)->d_stat.st_status & _FILE_USED) ? \
						&(D)->d_stat : dostat((F),(S)))
#else
#define get_stat(D,S,F) dostat((F),(S))
#endif

int ftw(const char *path, int (*fn)(const char *fname, const struct stat *sbuf, int flags), int depth) {
    DIR *dir;
    struct dirent *de;
    char *fname;
    int t;
    int dirs = 0;
    int rc = 0;
    struct stat sbuf;
    struct stat *sp;

    if (depth < 1)
	depth = 1;
    if ((dir = opendir(path)) == NULL) {
#if DDEBUG
	DBGPRT("opendir(%s) failed\n", path);
#endif
	return -1;
    }
    t = strlen(path);
    /* 2-- one for \0, one for '/' separator... */
#ifndef NAME_MAX		// Should be using fpathconf()
#define NAME_MAX _POSIX_PATH_MAX
#endif
    if ((fname = malloc(t + NAME_MAX + 2)) == NULL) {
	closedir(dir);
#if DDEBUG
	DBGPRT("malloc(%d) failed\n", t + NAME_MAX + 2);
#endif
	errno = ENOMEM;
	return -1;
    }
    memcpy(fname, path, t);
    strcpy(fname, path);
    if (t && fname[t - 1] != '/') {
	fname[t++] = '/';
    }
    while ((de = readdir(dir)) != NULL) {
	int arg;

	if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
	    continue;
	strcpy(fname + t, de->d_name);
	if ((sp = get_stat(de, &sbuf, fname)) == NULL) {
	    arg = FTW_NS;
	} else if (S_ISDIR(sp->st_mode)) {
#if ! USE_ACCESS
	    if (xperms(sp) & R_OK) {
#else
	    if (access(fname, R_OK) == 0) {
#endif
		dirs++;
		arg = FTW_D;
	    } else {
		arg = FTW_DNR;
	    }
	} else {
	    arg = FTW_F;
	}
	if (rc = (*fn) (fname, sp, arg)) {
#if DDEBUG
	    DBGPRT("aborted by user function(%d)\n", rc);
#endif
	    break;
	}
    }
    closedir(dir);
    if (rc == 0 && dirs && depth > 1) {
	/* oh no says bill, we don't need a 'rewinddir' */
	if ((dir = opendir(path)) == NULL) {
	    free(fname);
#if DDEBUG
	    DBGPRT("opendir2(%s) failed\n", path);
#endif
	    return -1;
	}
	while ((de = readdir(dir)) != NULL) {
	    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
		continue;
	    strcpy(fname + t, de->d_name);
	    if ((sp = get_stat(de, &sbuf, fname)) == NULL)
		continue;
	    if (S_ISDIR(sp->st_mode)) {
#if ! USE_ACCESS
		if ((xperms(sp) & X_OK) == 0)
#else
		if (access(fname, X_OK) != 0)
#endif
		    continue;
		if ((rc = ftw(fname, fn, depth - 1)) != 0) {
#if DDEBUG
		    DBGPRT("ftw failed (%d)\n", rc);
#endif
		    break;
		}
	    }
	}
	closedir(dir);
    }
    free(fname);
    return rc;
}


__SRCVERSION("win32/ftw.c $Rev: 153052 $");

