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



#include <lib/compat.h>

int eaccess(char *path, int mode) {
#if 1
    return( access( path, mode ) );
#else
    struct stat sb;
    int shift, perm;
    int uid;

    if ((perm = stat(path, &sb)) != 0) return perm;
#if 0
    if ((uid = geteuid()) == 0) {
	/* super-user always gets read-write access
	 * and execute access if any x bit is set
	 */
	sb.st_mode = 6 | ((sb.st_mode & 0111) != 0);
	shift = 0;
    }
    else if (uid == sb.st_uid) shift = 6;		/* owner */
    else if (getegid() == sb.st_gid) shift = 3;		/* group */
    else shift = 0;					/* other */
    perm = (sb.st_mode >> shift) & 7;

    if ((perm | mode) != perm) {
	errno = EACCES;
	return -1;
    }
#endif
    return 0;
#endif
}
