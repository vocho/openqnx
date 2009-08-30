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
 * Just like read, but disable echo around it.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#ifdef __MINGW32__
#include <lib/compat.h>
#else
#include <pwd.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <util/qnx4dev.h>
#include <sys/stat.h>
/* #include "passwd.h" */

int
read_noecho(int fd, char *pwbuf,int len)
{
int	mode;
	mode = dev_mode(fd,0,_DEV_ECHO);
	len = read(fd,pwbuf,len);
	dev_mode(fd,mode,_DEV_ECHO);
	return len;
}
