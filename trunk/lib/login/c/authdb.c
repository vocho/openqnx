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

 authpdb.c- authenticate the password database.

 This module tests the sanity of the password database via the following
 assertions-
  1.  The password and shadow files must exist.
  2.  The password and shadow files must be on the same device.
  3.  The password and shadow files must be "regular" files.
  4.  The password and shadow files must be owned by uid=0.






$Log$
Revision 1.5  2005/06/03 01:22:46  adanko
Replace existing QNX copyright licence headers with macros as specified by
the QNX Coding Standard. This is a change to source files in the head branch
only.

Note: only comments were changed.

PR25328

Revision 1.4  2003/10/21 19:28:12  bstecher
Don't need to include <sys/disk.h>

Revision 1.3  2003/10/15 21:22:40  jgarvey
Tidy up lib/login.  There were two almost-identical header files,
"h/login.h" (used during build because it came earlier in -I list)
and "public/login.h" (which was the one installed for clients to
compile against).  This duplication could led to coherence/
consistency problems.  So take all duplication out of "h/login.h",
and what is left is very private/internal use only, put that in
a new "h/liblogin.h" which can be included by the (3) source files
in this library that use it.  Now builds clean QNX4 and QNX6.

Revision 1.2  1998/09/26 16:24:11  steve
*** empty log message ***

Revision 1.1  1997/02/18 16:50:05  kirk
Initial revision

*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/disk.h>

#include "liblogin.h"
#include "login.h"


enum pwdbstat_e
auth_pwdb(void)
{
	struct stat     pw_st, sh_st;

	if (lchk_passwd() != 0) {
		return BusyPasswd;
	}
	if (lstat(PASSWD, &pw_st) != 0) {
		return NoPasswd;
	}
	if (lstat(SHADOW, &sh_st) != 0) {
		return NoShadow;
	} else if (pw_st.st_dev != sh_st.st_dev) {
		return NotSameDevice;
	}
	if (!S_ISREG(pw_st.st_mode)) {
		return PasswdBadType;
	}
	if (!S_ISREG(pw_st.st_mode)) {
		return ShadowBadType;
	}
	if (pw_st.st_uid != 0 || sh_st.st_uid != 0) {
		return InvalidOwner;
	}
	return PdbOk;
}

