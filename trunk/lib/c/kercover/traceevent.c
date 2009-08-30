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




#include <errno.h>
#include <stdarg.h>
#include <sys/trace.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <stdio.h>
#include "cpucfg.h"

extern int __traceevent(int *);

#if defined(TRACE_NOT_SUPPORTED)
 #undef TRACE_NOT_SUPPORTED // So, it is supported :-)
#endif

#if defined(TRACE_NOT_SUPPORTED)

int TraceEvent(int code, ...) {
	return (-1);
}

#elif defined(__X86__)

// This implementation works on x86 where taking the address of the
// first parameter gives you access to the rest as a linear array.
int TraceEvent(int code, ...) {
	if(in_interrupt()) {
		int r_v;

		if((r_v = SYSPAGE_ENTRY(callin)->trace_event(&code))) {
			errno = r_v;
			return (-1);
		}

		return (0);
	} else {
		return (__traceevent(&code));
	}
}


#elif (defined(__PPC__)||defined(__MIPS__)|| \
       defined(__ARM__)||defined(__SH__))

int TraceEvent(int code, ...) {
	int      data[10];           // Can't have more than 1+7+2 elements
	va_list  arg;
	unsigned i;
	unsigned count=_TRACE_GET_COUNT((unsigned)code);

	data[0] = code;
	va_start(arg, code);
	for(i=1; i<count; ++i) {
		data[i] = va_arg(arg, int);
	}
	va_end(arg);

	if(in_interrupt()) {
		int r_v;

		if((r_v = SYSPAGE_ENTRY(callin)->trace_event(data))) {
			errno = r_v;
			return (-1);
		}

		return (0);
	} else {
		return (__traceevent(data));
	}
}

#else
 #error instrumentation not supported
#endif

__SRCVERSION("traceevent.c $Rev: 153052 $");
