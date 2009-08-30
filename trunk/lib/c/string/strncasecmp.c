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

int
strncasecmp( const char *s1, const char *s2, size_t n ) {
	unsigned char	c1;
	unsigned char	c2;

	for( ;; ) {
		if(n == 0) return( 0 );
		c1 = *s1;
		c2 = *s2;
		if(c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
		if(c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
		if(c1 != c2) {
			/*
				Subtlely different behaviour from strnicmp, but it's
				mandated by Unix 98.
			*/
			return(c1 - c2);
		}
		if(c1 == '\0') return(0);
		++s1;
		++s2;
		--n;
	}
}

__SRCVERSION("strncasecmp.c $Rev: 153052 $");
