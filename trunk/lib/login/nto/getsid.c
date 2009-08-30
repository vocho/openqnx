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




/*

In the QNX 4 version, this contains the fns:

pid_t getspid(int id)         //get the pid of the session leader of sid.

getsid(int pid)

sid_name(int sid, char *name)


*/


#include <sys/types.h>
#include "login.h"


/*
	getspid:	get the pid of the session leader of sid.
*/

pid_t getspid(int id)
{
	extern pid_t hack_getspid(int id);

	return hack_getspid(id);
}

