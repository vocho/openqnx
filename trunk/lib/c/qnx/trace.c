/*
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
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
 *
 */
/* 
 * trace.c
 *
 * User land convenience functions for injecting events.  At some point a more
 * efficient method of inject user generated into the kernel tracelog might be 
 * dreamed up, but for now let's get users away from using the straight kernel call.
 */

#include <alloca.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/trace.h>

#define _TRACE_ROUND_UP_TO_INTS(n)          (((n)&3)?(((n)>>2)+1):((n)>>2))

int trace_vnlogf( int code, int max, const char *fmt, va_list arglist ) {
	char	*buf;
	if ( max == 0 ) {
		va_list			va_new;

		va_copy(va_new, arglist);
		max = vsnprintf(NULL, 0, fmt, va_new);
		va_end(va_new);
		if ( max < 0 ) {
			return -1;
		}
		max += 1; /* allow for null terminator */
	}
	/* RUSH: is alloca going to be aligned - I think so */
	if((buf = alloca(max)) == NULL) {
		errno = ENOMEM;
		return -1;
	}
	if ( vsnprintf(buf, max, fmt, arglist) == -1 ) {
		return -1;
	}
	if ( TraceEvent( _NTO_TRACE_INSERTUSRSTREVENT, code, buf ) == -1 ) {
		return -1;
	}
	return max;
}

int trace_logf( int code, const char *fmt, ... ) {
	va_list arglist;
	int		ret;

	va_start(arglist, fmt);
	ret = trace_vnlogf( code, 0, fmt, arglist );
	va_end(arglist);

	return ret;
}

int trace_nlogf( int code, int max, const char *fmt, ... ) {
	va_list arglist;
	int		ret;

	va_start(arglist, fmt);
	ret = trace_vnlogf( code, max, fmt, arglist );
	va_end(arglist);

	return ret;
}

int trace_logi( int code, unsigned d1, unsigned d2 ) {
	return TraceEvent( _NTO_TRACE_INSERTSUSEREVENT, code, d1, d2 );
}

int trace_logbc( int class, int event, const void *data, size_t nbytes )
{
	unsigned	len;
	void		*temp;

	/* TraceEvent takes the number of 32bit values, not bytes */
	len = _TRACE_ROUND_UP_TO_INTS(nbytes);
	/* If nbytes is not an even multiple of 32bit words, or the data address is
	 * not 32bit aligned, then we need to do a copy
	 */
	if ( (nbytes & 3) || ( ((unsigned)data) & 0x3) ) {
		temp = alloca(len * sizeof(unsigned));
		if ( temp == NULL ) {
			return -1;
		}
		memcpy(temp, data, nbytes);
		data = temp;
	}
	return TraceEvent( _NTO_TRACE_INSERTCCLASSEVENT, class>>10, event, data, len );
}

int trace_logb( int code, const void *buf, size_t nbytes ) {
	return trace_logbc( _TRACE_USER_C, code, buf, nbytes );
}

int trace_func_enter( void *this_fn, void *call_site ) {
	return TraceEvent( _NTO_TRACE_INSERTSCLASSEVENT, _NTO_TRACE_SYSTEM, _NTO_TRACE_SYS_FUNC_ENTER, (unsigned)this_fn, (unsigned)call_site );
}

int trace_func_exit( void *this_fn, void *call_site ) {
	return TraceEvent( _NTO_TRACE_INSERTSCLASSEVENT, _NTO_TRACE_SYSTEM, _NTO_TRACE_SYS_FUNC_EXIT, (unsigned)this_fn, (unsigned)call_site );
}

__SRCVERSION("trace.c $Rev$");
