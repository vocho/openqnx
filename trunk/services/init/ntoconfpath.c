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



#include <limits.h>
#include <stdio.h>
#include <sys/utsname.h>
#include <unistd.h>

static struct utsname u;

char *nto_conf_path( char *path, char *nodename, int executable )
{
static char buf[_POSIX_PATH_MAX];

	if ( nodename == NULL && u.nodename[0] == '\0' ) {
		uname( &u );
	}
	if ( nodename == NULL )
		nodename = u.nodename;

	sprintf( buf, "%s.%s", path, nodename );
	if ( access( buf, executable ? X_OK:F_OK ) != -1 )
		return buf;
	return path;
}
