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
unixlog: unix-style error and accounting logs.



$Log$
Revision 1.5  2005/06/03 01:22:46  adanko
Replace existing QNX copyright licence headers with macros as specified by
the QNX Coding Standard. This is a change to source files in the head branch
only.

Note: only comments were changed.

PR25328

Revision 1.4  2003/10/14 23:16:46  jgarvey
Tidy up the output formatting to remove GCC warnings.  A time_t
is an int not a long.  And "% 10.u" makes the same as "%10u"
(according to some testcode which formats/compares both for every
int value :-) without the warnings, so go with that.  Reasonably
academic, as I don't think QNX6 login tools do such accounting.

Revision 1.3  1999/06/21 12:49:39  thomasf
Removed call to getnid() for neutrino

Revision 1.2  1998/09/26 16:23:00  eric
*** empty log message ***

Revision 1.1  1997/02/18 16:50:10  kirk
Initial revision

*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <utmp.h>

#include "login.h"

void 
log_failure(char *tty_name, char **lname, time_t * ltimes, int retries)
{
	int             fd;
	int             qd;
	int             i;
	int             t;
	char            loginbuf[256];

	/* only mung buffer if it exists */
	fd = open(LOGIN_LOG, O_WRONLY | O_APPEND);
	qd = open(QNX_LOG, O_WRONLY | O_APPEND);
	for (i = 0; i < retries; i++) {
		if (fd != -1) {
			t = sprintf(loginbuf, "%s:%s:%d\n",
				lname[i], tty_name, ltimes[i]);
			write(fd, loginbuf, t);
		}
		if (qd != -1) {
			t = sprintf(loginbuf, "%10u LF %s %s\n",
				ltimes[i], tty_name, lname[i]);
			write(qd, loginbuf, t);
		}
	}
	if (fd != -1)
		close(fd);
	if (qd != -1)
		close(qd);
}


void 
unix_logging(struct passwd * pw, char *tty_name)
{
	struct utmp     utstr;
	struct utmp    *p;
	char           *s;
	memset(&utstr, 0, sizeof utstr);
	strcpy(utstr.ut_user, pw->pw_name);
	#ifdef __QNXNTO__
		utstr.ut_id[0]=0;  /* there are only 4 bytes available in ut_id */
	#else
		itoa(getnid(), utstr.ut_id, 10);
	#endif

	if ((s = strrchr(tty_name, '/')) == NULL)
		s = tty_name;
	else
		s++;
	strcpy(utstr.ut_line, s);

	utstr.ut_pid = getpid();
	utstr.ut_type = LOGIN_PROCESS;
	if ((p = getutline(&utstr)) == NULL) {
		endutent();
	}
	utstr.ut_time = time(0);
	pututline(&utstr);
	endutent();
}


void 
bsd_lastlog(struct passwd * pw, char *ttyname)
{
	int             fd;
	char           *p;
	struct lastlog  llbuf;
	time(&llbuf.ll_time);
	if ((p=strchr(ttyname, '/')) == 0)
		p = ttyname;
	else
		p++;
	strncpy(llbuf.ll_line, p, sizeof llbuf.ll_line ); 
	/* itoa(getnid(), llbuf.ll_line, 10); */
#ifdef __QNXNTO__ 
	confstr(_CS_HOSTNAME, llbuf.ll_host, 16);	//hostname is 16 chars long 
#else
	sprintf(llbuf.ll_host, "%ld", getnid());
#endif
	if ((fd = open(LASTLOG_FILE, O_WRONLY)) == -1)
		return;
	lseek(fd, (unsigned long)pw->pw_uid * sizeof llbuf, SEEK_SET);
	write(fd, &llbuf, sizeof llbuf);
	close(fd);
}
