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
#ifdef __USAGE
#endif
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#if defined(__MINGW32__)
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#include <ctype.h>
#include <lib/compat.h>

int main( int argc, char **argv )
{
char	*path;
int		i;

	if ( argc != 2 )
		return 1;

	path = strdup( argv[1] );
	if ( path == NULL )
		return 1;

	if ( path[0] == '\\' && path[1] == '\\' ) {
		path++;
		path[0] = path[1];
		path[1] = ':';
	}
	for ( i = 0; path[i]; i++ ) {
		if ( path[i] == '\\' )
			path[i] = '/';
	}
	fprintf( stdout, "%s", path );

	return 0;
}
