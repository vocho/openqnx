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

Since qnx doesn't regularly have a getty, this function both
checks that login is being run on a tty, and ensures the tty is
in a somewhat sane mode.



$Log$
Revision 1.8  2005/06/03 01:22:46  adanko
Replace existing QNX copyright licence headers with macros as specified by
the QNX Coding Standard. This is a change to source files in the head branch
only.

Note: only comments were changed.

PR25328

Revision 1.7  2003/10/12 06:46:51  jgarvey
Probable QNX4->6 porting issue (again found from compiler warning).
There is no "tcsetct()" under QNX6; despite comments in mig4nto I
think that TIOCSCTTY is a potential implementation, so use that.
Can't tell/test, the routine isn't actually used by this library ;-)

Revision 1.6  1999/04/02 20:15:18  bstecher
Clean up some warnings

Revision 1.5  1999/03/04 15:12:22  peterv
Fixed syntax error

Revision 1.4  1999/02/24 18:43:34  peterv
Cleaned up termio to match posix.

Revision 1.3  1998/10/14 21:20:04  eric
Modified for nto, now uses some support/util includes.

Revision 1.2  1998/09/26 16:23:00  eric
*** empty log message ***

Revision 1.1  1997/02/18 16:50:07  kirk
Initial revision

*/

#include <libc.h>
#include <unistd.h>
#include <termios.h>
#include <util/qnx4dev.h>
#ifdef __QNXNTO__
#include <sys/ioctl.h>
#else
#include <unistd.h>
#endif

#include "login.h"

int
getty(char *ttyname)
{
	struct	termios	tios;

        if (ctermid(ttyname) == 0) {
        	fputs("getty: no controlling terminal!\n", stderr);
        }
/* only allow login if I am the session leader. */
	if (getspid(getsid(0)) != getpid()) {
		fputs("getty: must be session leader!\n", stderr);
		exit(EXIT_FAILURE);
	}

	if (! isatty(0) || ! isatty(1) || ! isatty(2)) {
		return -1;
	}

#ifndef WOGIN
	tcflush(0,TCIFLUSH);
#endif

	tcgetattr(0,&tios);
	tios.c_iflag |= BRKINT | ICRNL | IXON;
#ifdef ONLCR
	tios.c_oflag |= OPOST | ONLCR;
#else
	tios.c_oflag |= OPOST;
#endif
	tios.c_cflag |= CREAD;
	tios.c_lflag |= ECHO | ECHOE | ECHOK | ISIG | ICANON;
	tcsetattr(0,TCSANOW, &tios);
#ifdef __QNXNTO__
	ioctl(0, TIOCSCTTY);
#else
	tcsetct(0,getpid());
#endif
	return 0;

}
