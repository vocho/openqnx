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



#include <utmp.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdio.h> 

int openutmp(int oflag, int mode)
{
#ifndef __QNXNTO__
	static char *u= "/usr/adm/utmp";
#else
	static char *u= _PATH_UTMP;
#endif
	char buffer[MAXHOSTNAMELEN + 15];
	int fd, err;

#if	!defined(__QNXNTO__)
	sprintf(buffer, "%s.%d", u, getnid());
#else
	/* Login didn't add hostname as of 000712.  Should it? */
	sprintf(buffer, "%s.", u);
	gethostname(&buffer[strlen(u) + 1], MAXHOSTNAMELEN);
#endif
	if ((fd= open(buffer, oflag, mode)) >= 0)
		return fd;
	err= errno;
	
	if ((fd= open(u, oflag, mode)) >= 0)
		return fd;
	err= errno;
	
	return -1;
}

int openwtmp(int oflag, int mode)
{
#if	!defined(__QNXNTO__)
	static char *u= "/usr/adm/wtmp";
#else
	static char *u= _PATH_WTMP;
#endif
	char buffer[MAXHOSTNAMELEN + 16];
	int fd;

#if	!defined(__QNXNTO__)
	sprintf(buffer, "%s.%d\n", u, getnid());
#else
	/* Login didn't add hostname as of 000712.  Should it? */
	sprintf(buffer, "%s.", u);
	gethostname(&buffer[strlen(u) + 1], MAXHOSTNAMELEN);
#endif
	
	if ((fd= open(buffer, oflag, mode)) >= 0)
		return fd;
	
	if ((fd= open(u, oflag, mode)) >= 0)
		return fd;
		
	return -1;
}

