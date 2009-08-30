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



#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>

char * getcwd( char * buffer, size_t size )
{
	char * path = "/";

	if( ! buffer )
	{
		size = strlen( path );
		buffer = malloc( size + 1 );
		if( ! buffer )
		{
			errno = ENOMEM;
			return 0;
		}
	}
	if( size < strlen( path ) )
	{
		errno = ERANGE;
		return 0;
	}
	return strncpy( buffer, path, size );
}
