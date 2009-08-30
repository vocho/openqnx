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
int _memicmp( const void *_s1, const void *_s2, size_t len )
{
	return( memicmp( _s1, _s2, len ) );
}
#else
LIBC_WEAK(memicmp, _memicmp);
#endif

int memicmp( const void *_s1, const void *_s2, size_t len )
{
	const unsigned char	*s1 = _s1;
	const unsigned char	*s2 = _s2;
	unsigned char		c1;
	unsigned char		c2;

	for( ;; ) {
		if( len == 0 ) return( 0 );
		c1 = *s1;
		c2 = *s2;
		if( c1 >= 'A' && c1 <= 'Z' ) c1 += 'a' - 'A';
		if( c2 >= 'A' && c2 <= 'Z' ) c2 += 'a' - 'A';
		if( c1 != c2 ) return( *s1 - *s2 );
		++s1;
		++s2;
		--len;
	}
}

__SRCVERSION("memicmp.c $Rev: 167420 $");
