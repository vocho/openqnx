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
 * sulog: put an entry into the sulog



 $Log$
 Revision 1.3  2005/06/03 01:22:46  adanko
 Replace existing QNX copyright licence headers with macros as specified by
 the QNX Coding Standard. This is a change to source files in the head branch
 only.

 Note: only comments were changed.

 PR25328

 Revision 1.2  1999/04/02 20:15:19  bstecher
 Clean up some warnings

 Revision 1.1  1997/02/18 16:50:10  kirk
 Initial revision

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <pwd.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>


#include "login.h"

int
sulog(int failed, char *to, char *logfile)
{
	char            buf[512];
	struct passwd  *pw;
	int             len;
	time_t          clock;
	int             fd;
	char           *s;
	strcpy(buf, "SU ");
	len = 3;

	time(&clock);
	len += strftime(buf + len, 100, "%m/%d %H:%M ", localtime(&clock));
	if (!(s = ctermid(0))) {
		s = "???";
	}
	len += sprintf(buf + len, "%c %s ", failed ? '-' : '+', basename(ctermid(0)));
	if (!(pw = getpwuid(getuid()))) {
		len += sprintf(buf + len, "unknown-");
	} else {
		len += sprintf(buf + len, "%s-", pw->pw_name);
	}
	len += sprintf(buf + len, "%s\n", to);
	if ((fd = open(logfile, O_WRONLY | O_APPEND)) == -1) {
		return -1;
	}
	write(fd, buf, len);
	close(fd);
	return 0;
}
