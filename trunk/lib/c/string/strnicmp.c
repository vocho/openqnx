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




#include <strings.h>

#ifndef __WEAK_ALIAS
int
_strnicmp( const char *s1, const char *s2, size_t n ) {
	return( strnicmp( s1, s2, n ) );
}
#else
LIBC_WEAK(strnicmp, _strnicmp);
#endif

int
strnicmp( const char *s1, const char *s2, size_t n ) {
	unsigned char	c1;
	unsigned char	c2;
	unsigned char	cc1;
	unsigned char	cc2;

	for( ;; ) {
		if(n == 0) return( 0 );
		cc1 = c1 = *s1;
		cc2 = c2 = *s2;
		if(cc1 >= 'A' && cc1 <= 'Z') cc1 += 'a' - 'A';
		if(cc2 >= 'A' && cc2 <= 'Z') cc2 += 'a' - 'A';
		if(cc1 != cc2) {
			/*
				So the ordering is right for characters between the end
				of the upper case character set and the begining of the
				lower case ones (ASCII dependent code).
			*/
			if(cc1 > 'Z' && cc1 < 'a') cc2 = c2;
			if(cc2 > 'Z' && cc2 < 'a') cc1 = c1;
			return(cc1 - cc2);
		}
		if(cc1 == '\0') return(0);
		++s1;
		++s2;
		--n;
	}
}

__SRCVERSION("strnicmp.c $Rev: 167420 $");
