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



#ifdef __MINGW32__
#include <lib/compat.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#ifdef __STDC__
int (saccess)( struct stat *statbuf, int amode )
#else 
int saccess(statbuf,amode)
struct stat *statbuf;
int amode;
#endif
    {
    int         lookat;

    if (amode & ~(R_OK|W_OK|X_OK)) {
        errno=EINVAL;
        return(-1);
    }

    if (amode & R_OK) {
        lookat = S_IROTH;
        if (statbuf->st_uid == getuid())        lookat = S_IRUSR;
        else if (statbuf->st_gid == getgid())   lookat = S_IRGRP;
        if ((statbuf->st_mode & lookat) == 0) {
            errno=EACCES;
            return(-1);
        }
    }

    if (amode & X_OK) {
        lookat = S_IXOTH;
        if (statbuf->st_uid == getuid())        lookat = S_IXUSR;
        else if (statbuf->st_gid == getgid())   lookat = S_IXGRP;
        if ((statbuf->st_mode & lookat) == 0) {
            errno=EACCES;
            return(-1);
        }
    }

    if (amode & W_OK) {
        lookat = S_IWOTH;
        if (statbuf->st_uid == getuid())       lookat = S_IWUSR;
        else if (statbuf->st_gid == getgid())  lookat = S_IWGRP;

        if ((statbuf->st_mode & lookat) == 0) {
            errno=EACCES;
            return(-1);
        }

        /* if looks like we are ok for write, but wait, this
           might be a read-only file system. Unfortunately,
           there is no call available to determine if this is
           a read-only file system. This situation requires a
           new call to the file system to fix. */

    }

    return(0);
}
