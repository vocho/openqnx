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





#include <stdio.h>

#define OK      0
#define FAIL    NULL
#define NOFILE  ((FILE *) 0)
#define MAXLINE 128

#ifndef __QNX__
extern int      errno;
char            *strchr();
#else
#include <errno.h>
#include <string.h>
#endif

static FILE     *defd = NOFILE;  /* defaults file stream */

int     defopen(fname)          /* open | reopen | close defaults file */
	char    *fname;
{
	register FILE   *fd;

	if (!fname) {
		if (defd)
			fclose(defd);
		defd = NOFILE;
		return OK;
	}

	if (!(fd = fopen(fname, "r")))
		return errno;                   /* problems opening file */

	defd = fd;
	return OK;
}

static char     defline[MAXLINE + 1];

char    *defread(pattern)
	register char   *pattern;
{
	register int    sz_patt;
	register char   *cp;

	if (!defd)
		return FAIL;            /* defaults file not opened */

	rewind(defd);
	sz_patt = strlen(pattern);

	while (fgets(defline, MAXLINE, defd)) {
		if (!(cp = strchr(defline, '\n')))
			return FAIL;     /* line too long  */
		if (cp - defline < sz_patt)
			continue;       /* line too short */
		*cp = '\0';
		if (!strncmp(pattern, defline, sz_patt))
			return defline + sz_patt;       /* match found */
	}

	return FAIL;                    /* no matching lines */
}
