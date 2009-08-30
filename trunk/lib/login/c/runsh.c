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
run_shell -- transform into the login shell for this pw



$Log$
Revision 1.6  2005/06/03 01:22:46  adanko
Replace existing QNX copyright licence headers with macros as specified by
the QNX Coding Standard. This is a change to source files in the head branch
only.

Note: only comments were changed.

PR25328

Revision 1.5  2003/10/15 21:22:40  jgarvey
Tidy up lib/login.  There were two almost-identical header files,
"h/login.h" (used during build because it came earlier in -I list)
and "public/login.h" (which was the one installed for clients to
compile against).  This duplication could led to coherence/
consistency problems.  So take all duplication out of "h/login.h",
and what is left is very private/internal use only, put that in
a new "h/liblogin.h" which can be included by the (3) source files
in this library that use it.  Now builds clean QNX4 and QNX6.

Revision 1.4  1999/04/02 20:15:19  bstecher
Clean up some warnings

Revision 1.3  1998/10/14 21:20:04  eric
Modified for nto, now uses some support/util includes.

Revision 1.2  1998/09/26 16:23:00  eric
*** empty log message ***

Revision 1.1  1997/02/18 16:50:09  kirk
Initial revision

 * Revision 1.1  1995/03/10  14:41:18  steve
 * Initial revision
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <process.h>
#include <pwd.h>
#include <malloc.h>
#include <limits.h>
#include <libgen.h>
#include <util/stdutil.h>

#include "liblogin.h"
#include "login.h"

/*-
 * count the number of tokens in a buffer
 */
static int
count_tokens(char *bufp, char *delims)
{
	int       count;
	for (count=0; bufp; count++, bufp=strpbrk(bufp, delims)) {
		while (*bufp && strchr(delims, *bufp))
			bufp++;
		if (!*bufp) /* don't count leading or trailing delims */
			break;
	}
	return count;
}

int
run_shell(struct passwd * pw)
{
	char            shname[UTIL_NAME_MAX + 2];
	char          **argv;
	int             i;
	char           *delims = " \t\n";
	char           *cmd;

	if ((i = count_tokens(pw->pw_shell, delims)) == 0) {
		execle("/bin/sh", "-sh", NULL, environ);
		return -1;
	}
	if ((cmd=strdup(pw->pw_shell)) == 0) {
		errno = ENOMEM;
		return -1;
	}
	if ((argv = calloc(i + 2, sizeof(char *))) == NULL) {
		errno = ENOMEM;
		return -1;
	}
	strcpy(shname + 1, basename(strtok(cmd, delims)));
	*(argv[0] = shname) = '-';
	for (i = 1; argv[i++] = strtok(NULL, delims););
	argv[i] = NULL;
	execve(cmd, argv, environ);
	return -1;
}
