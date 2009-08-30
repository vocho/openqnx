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
 qnxlog -- update qnx-specific accoutning files with relevant information



 $Log$
 Revision 1.3  2005/06/03 01:22:46  adanko
 Replace existing QNX copyright licence headers with macros as specified by
 the QNX Coding Standard. This is a change to source files in the head branch
 only.

 Note: only comments were changed.

 PR25328

 Revision 1.2  2003/10/14 23:16:46  jgarvey
 Tidy up the output formatting to remove GCC warnings.  A time_t
 is an int not a long.  And "% 10.u" makes the same as "%10u"
 (according to some testcode which formats/compares both for every
 int value :-) without the warnings, so go with that.  Reasonably
 academic, as I don't think QNX6 login tools do such accounting.

 Revision 1.1  1997/02/18 16:50:09  kirk
 Initial revision

*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <time.h>

#include "login.h"

void 
qnx_pwlog(struct passwd * pw, char *pwlog, char *ttyname)
{
	int             fd;
	int             n;
	char            buffer[100];

	if ((fd = open(QNX_LOG, O_WRONLY | O_APPEND)) != -1) {
		n = sprintf(buffer, "%10u %2.2s %s %u %u %s\n",
			    time(NULL),
			    pwlog,
			    ttyname,
			    pw->pw_uid,
			    pw->pw_gid,
			    pw->pw_name);
		write(fd, buffer, n);
		close(fd);
	}
}
