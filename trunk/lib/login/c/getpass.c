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
  getpass.c: prompt user and retrieve a string with no echo to tty



$Log$
Revision 1.5  2005/06/03 01:22:46  adanko
Replace existing QNX copyright licence headers with macros as specified by
the QNX Coding Standard. This is a change to source files in the head branch
only.

Note: only comments were changed.

PR25328

Revision 1.4  1999/05/20 12:10:21  thomasf
Added modifications to _not_ include the code that is already present
in the NTO libc, tested locally on my configuration.

Revision 1.3  1999/01/06 20:53:08  bstecher
Unix98 compatability

Revision 1.2  1998/10/14 21:20:04  eric
Modified for nto, now uses some support/util includes.

Revision 1.1  1997/02/18 16:50:07  kirk
Initial revision

*/

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <util/qnx4dev.h>

#include "login.h"

#if !defined(__QNXNTO__)
char *
getpass(const char *prompt) {
    static char pw[32];
    unsigned mode;
    int tty, i;
    char c;

    if ((tty = open("/dev/tty", O_RDWR)) < 0)
	return 0;
    write(tty, prompt, strlen(prompt));
    mode = dev_mode(tty, 0, _DEV_ECHO);
    for (i = 0; read(tty, &c, sizeof c) > 0 && c != '\n';)
	if (i < sizeof pw) pw[i++] = c;
    pw[i] = 0;
    dev_mode(tty, mode, _DEV_ECHO);
    c = '\n'; write(tty, &c, sizeof c);
    close(tty);
    return pw;
}
#endif
