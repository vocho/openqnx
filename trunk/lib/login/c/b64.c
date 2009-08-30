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




#include <sys/types.h>
#include <sys/timeb.h>
#include "login.h"

int
base_64(short x)
{
	if (x <= 9)	return	x + '0';
	x -= 9;
	if (x <= 26) return x + 'A';
	x -= 26;
	return x + 'a';
}

int
new_salt(char *salt)
{
int		salt_time;
struct	timeb	tms;

	ftime(&tms);
	salt_time = (tms.millitm/100 + tms.time * 10) & 07777;
	salt[0] = base_64(salt_time & 037);
	salt[1] = base_64((salt_time >> 6) & 037);
	salt[2] = '\0';
	return 0;
}
