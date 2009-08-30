/*
 * $QNXLicenseC:
 * Copyright 2007,2008, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable
 * license fees to QNX Software Systems before you may reproduce,
 * modify or distribute this software, or any work that includes
 * all or part of this software.   Free development licenses are
 * available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email
 * licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review
 * this entire file for other proprietary rights or license notices,
 * as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

//NOTE: those won't be needed once all the code uses the new MEM_ macros
#ifdef _BT_LIGHT

#include "common.h"

#include "utils.h"


void *_BT(memcpy) (void *dest, const void *src, size_t n)
{
	char *d=dest;
	const char *s=src;
	size_t i;
	for (i=0; i<n; i++) {
		d[i]=s[i];
	}
	return d;
}


void *_BT(memmove) (void *dest, const void *src, size_t n)
{
	int i;
	char *d=dest;
	const char *s=src;
	if (d<s) {
		for (i=0; i<n; i++)
			d[i] = s[i];
	} else if (d>s) {
		for (i=n-1; i>=0; i--)
			d[i] = s[i];
	}
	return dest;
}

#endif
