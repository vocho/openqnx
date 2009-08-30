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



/*-
 * Simple scheme for locking the password database.



 * $Log$
 * Revision 1.4  2005/06/03 01:22:46  adanko
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.3  1999/04/02 20:15:19  bstecher
 * Clean up some warnings
 *
 * Revision 1.2  1998/09/26 16:23:00  eric
 * *** empty log message ***
 *
 * Revision 1.1  1997/02/18 16:50:09  kirk
 * Initial revision
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "login.h"

static int
acquire(const char *pname, unsigned timeout)
{
	int             fd;

	do {
		if ((fd=open(pname, O_CREAT | O_EXCL, 0600)) == -1) {
			if (errno!=EEXIST) return 0;
				
			if (timeout) {
				if (timeout != -1) timeout--;
				sleep(1);
			}
		}
	} while (fd == -1 && timeout);
	if (fd == -1) {
		errno = EBUSY;
		return 0;
	}
	close(fd);
	return 1;
}

static int
liberate(const char *pname)
{
	return unlink(pname) == 0;
}


int
lock_passwd()
{
	return acquire(PW_LOCK,LOCK_TIMEOUT);
}


int
unlock_passwd()
{
	return liberate(PW_LOCK);
}


#include <sys/stat.h>
int
lchk_passwd(void)
{
	struct stat st;
	return stat(PW_LOCK, &st) == 0;
}
