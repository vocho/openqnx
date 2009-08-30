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
char *_strupr( char *str )
{
	return( strupr( str ) );
}
#else
LIBC_WEAK(strupr, _strupr);
#endif

char *strupr( char *str )
{
	char	*p = str;
	char	c;

	for( ;; ) {
		c = *p;
		if( c == '\0' ) break;
		if( c >= 'a' && c <= 'z' ) {
			*p = c + ('A' - 'a');
		}
		++p;
	}
	return( str );
}

__SRCVERSION("strupr.c $Rev: 167420 $");
