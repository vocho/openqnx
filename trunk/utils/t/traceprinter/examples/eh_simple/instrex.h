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





/*
 *  instrex.h instrumentation examples - public definitions
 *
 */

#ifndef __INSTREX_H_INCLUDED

#include <errno.h>
#include <stdio.h>
#include <string.h>

/*
 * Supporting macro that intercepts and prints a possible
 * error states during calling TraceEvent(...)
 *
 * Call TRACE_EVENT(TraceEvent(...))  <=>  TraceEvent(...)
 *
 */
#define TRACE_EVENT(prog_name, trace_event) \
if((int)((trace_event))==(-1)) \
{ \
	(void) fprintf \
	( \
	 stderr, \
	 "%s: function call TraceEvent() failed, errno(%d): %s\n", \
	 prog_name, \
	 errno, \
	 strerror(errno) \
	); \
 \
	return (-1); \
}

/*
 * Prints error message
 */
#define TRACE_ERROR_MSG(prog_name, msg) \
	(void) fprintf(stderr,"%s: %s\n", prog_name, msg)

#define  __INSTREX_H_INCLUDED
#endif

