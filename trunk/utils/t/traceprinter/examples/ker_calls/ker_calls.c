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





#ifdef __USAGE
%C - instrumentation example

%C	- [-n num]

%C	- example that illustrates the very basic use of
	  the TraceEvent() kernel call and the instrumentation
	  module with tracelogger in a daemon mode.

	  All thread states and all/one (specified) kernel
	  call number are intercepted. The kernel call(s)
	  is(are) intercepted in wide emitting mode.

Options:
	-n <num> kernel call number to be intercepted
	   (defult is all)

	  In order to use this example, start the tracelogger
	  in the deamon mode as:

	  tracelogger -n iter_number -d

	  with iter_number = your choice of 1 through 10

	  After executing the example, the tracelogger (daemon)
	  will log the specified number of iterations and then
	  terminate. There are no messages printed uppon successful
	  completion of the example. You can view the intercepted
	  events with the traceprinter utility.

	  See accompanied documentation and comments within
	  the example source code for more explanations.
#endif

#include <sys/trace.h>
#include <unistd.h>
#include <stdlib.h>

#include "instrex.h"

int main(int argc, char **argv)
{
	int arg_var;       /* input arguments parsing support      */
	int call_num=(-1); /* kernel call number to be intercepted */

	/* Parse command line arguments
	 *
	 * - get optional kernel call number
	 */
	while((arg_var=getopt(argc, argv,"n:"))!=(-1)) {
		switch(arg_var) 
		{
			case 'n': /* get kernel call number */
				call_num = strtoul(optarg, NULL, 10);
				break;
			default:  /* unknown */
				TRACE_ERROR_MSG
				(
				 argv[0],
				 "error parsing command-line arguments - exitting\n"
				);

				return (-1);
		}
	}			

	/*
	 * Just in case, turn off all filters, since we
	 * don't know their present state - go to the
	 * known state of the filters.
	 */
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_DELALLCLASSES));
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_CLRCLASSPID, _NTO_TRACE_KERCALL));
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_CLRCLASSTID, _NTO_TRACE_KERCALL));
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_CLRCLASSPID, _NTO_TRACE_THREAD));
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_CLRCLASSTID, _NTO_TRACE_THREAD));

	/*
	 * Set wide emitting mode for all classes and
	 * their events. 
	 */
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE));

	/*
	 * Intercept _NTO_TRACE_THREAD class
	 * We need it to know the state of the active thread.
	 */
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_ADDCLASS, _NTO_TRACE_THREAD));

	/*
	 * Add all/one kernel call
	 */
	if(call_num != (-1)) {
		TRACE_EVENT
		(
		 argv[0],
		 TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_KERCALL, call_num)
		);
	} else {
		TRACE_EVENT
		(
		 argv[0],
		 TraceEvent(_NTO_TRACE_ADDCLASS, _NTO_TRACE_KERCALL)
		);
	}

	/*
	 * Start tracing process
	 *
	 * During the tracing process, the tracelogger (which
	 * is being executed in a daemon mode) will log all events.
	 * The number of full logged iterations is user specified.
	 */
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_START));

	/*
	 * The main() of this execution flow returns.
	 * However, the main() function of the tracelogger
	 * will return after registering the specified number
	 * of events.
	 */
	return (0);
}

