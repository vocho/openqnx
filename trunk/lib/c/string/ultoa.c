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




#include <stdlib.h>
#include <errno.h>

#if defined(__WATCOMC__) && __WATCOMC__ <= 1100
#define FIXCONST	__based(__segname("CONST2"))
#else
#define FIXCONST
#endif

#if 0
char *ultoa(unsigned long int value, char *buf, int radix) {
	char			tmp[32 + 1];		/* Lowest radix is 2, so 32-bits plus a null */
	char			*p1 = tmp, *p2;
	static const char FIXCONST xlat[] = "0123456789abcdefghijklmnopqrstuvwxyz";

	if(radix < 2 || radix > 36) {
		errno = EINVAL;
		return 0;
	}

	do {
		*p1++ = xlat[value % radix];
	} while(value /= radix);

	for(p2 = buf; p1 != tmp; *p2++ = *--p1);
	*p2 = '\0';

	return buf;
}
#else

char *ultoa(unsigned long int value, char *buf, int radix) {
	return ulltoa(value, buf, radix);
}

#endif

__SRCVERSION("ultoa.c $Rev: 153052 $");
