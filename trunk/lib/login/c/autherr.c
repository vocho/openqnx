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
    Map error codes into appropriate strings for messages.




$Log$
Revision 1.3  2005/06/03 01:22:46  adanko
Replace existing QNX copyright licence headers with macros as specified by
the QNX Coding Standard. This is a change to source files in the head branch
only.

Note: only comments were changed.

PR25328

Revision 1.2  1998/09/26 16:24:11  steve
*** empty log message ***

Revision 1.1  1997/02/18 16:50:05  kirk
Initial revision

*/

#include <string.h>
#include "login.h"



static struct look {
	enum pwdbstat_e ecode;
	const char *estr;
} ltab[] = {
	{ PdbOk, "" },
	{ NoPasswd, "Password file does not exist" },
	{ NoShadow, "Shadow file does not exist" },
	{ NotSameDevice, "Password and Shadow files on different devices" },
	{ PasswdBadType, "Password is not a regular file" },
	{ ShadowBadType, "Shadow is not a regular file" },
	{ InvalidOwner,  "Password and Shadow files must be owned by userid 0" },
	{ BusyPasswd, "Password file update in progress" }
};
static int ltabmax = sizeof ltab/sizeof *ltab;

const char *
pwdb_errstr(enum pwdbstat_e err)
{
	int i;
	for (i=0; i < ltabmax; i++)
		if (ltab[i].ecode == err) return ltab[i].estr;
	return "Unknown password state";
}
