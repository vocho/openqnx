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




#include <string.h>

#ifndef __WEAK_ALIAS
char *_strlwr( char *str )
{
	return( strlwr( str ) );
}
#else
LIBC_WEAK(strlwr, _strlwr);
#endif

char *strlwr( char *str )
{
	char	*p = str;
	char	c;

	for( ;; ) {
		c = *p;
		if( c == '\0' ) break;
		if( c >= 'A' && c <= 'Z' ) {
			*p = c + ('a' - 'A');
		}
		++p;
	}
	return( str );
}

__SRCVERSION("strlwr.c $Rev: 167420 $");
