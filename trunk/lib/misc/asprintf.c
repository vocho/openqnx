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





#include <util.h>
#include <stdarg.h>
#include <malloc.h>

int
asprintf(char **bufp, const char *fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);
	ret = vasprintf(bufp, fmt, va);
	va_end(va);

	return (ret);
}


int
vasprintf(char **bufp, const char * fmt, va_list va)
{
	va_list va_new;
	int required;

	va_copy(va_new, va);
	required = vsnprintf(NULL, 0, fmt, va_new);
	va_end(va_new);

	required++; /* for terminating '\0' */

	if ((*bufp = malloc(required)) == NULL)
		return -1;

	return (vsnprintf(*bufp, required, fmt, va));
}
