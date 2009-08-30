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



#include "lib/compat.h"

/*
	go to the first separator position
*/

char* __first_dirsep( const char* path )
{
	const char* p;
	
	for ( p = path; *p != '\0'; ++p )
	{
		if ( IS_DIRSEP(*p) && !IS_DIRSEP(*(p+1)) )
			return (char*)p;
	}

	return (char *) 0;
}

/*
	go to the last separator position
*/

char* __last_dirsep( const char* path )
{
	const char* p;
	const char* last_slash = (char *)0;
	
	for ( p = path; *p != '\0'; ++p )
	{
		if ( IS_DIRSEP(*p) && !IS_DIRSEP(*(p+1)) )
			last_slash = p;
	}
	
	return (char *)last_slash;
}
