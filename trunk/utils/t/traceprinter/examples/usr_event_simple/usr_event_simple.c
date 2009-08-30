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

%C	- example that illustrates the very basic use of
	  the TraceEvent() kernel call and the instrumentation
	  module with tracelogger in a daemon mode.

	  All classes and their events are included and monitored.
	  Additionally, four user generated simple events and
	  one string event are intercepted.

	  In order to use this example, start the tracelogger
	  in the deamon mode as:

	  tracelogger -n iter_number -d

	  with iter_number = your choice of 1 through 10

	  After executing the example, the tracelogger (daemon)
	  will log the specified number of iterations and then
	  terminate. There are no messages printed uppon successful
	  completion of the example. You can view the intercepted
	  events with the traceprinter utility. The intercepted
	  user events (class USREVENT) have event id(s)
	  (EVENT) equal to: 111, 222, 333, 444 and 555.

	  See accompanied documentation and comments within
	  the example source code for more explanations.
#endif

#include <sys/trace.h>
#include <unistd.h>

#include "instrex.h"

int main(int argc, char **argv)
{
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
	 * Set fast emitting mode for all classes and
	 * their events. 
	 */
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_SETALLCLASSESFAST));

	/*
	 * Intercept all event classes
	 */
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_ADDALLCLASSES));

	/*
	 * Start tracing process
	 *
	 * During the tracing process, the tracelogger (which
	 * is being executed in a daemon mode) will log all events.
	 * The number of full logged iterations is user specified.
	 */
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_START));

	/*
	 * Insert four user defined simple events and one string
	 * event into the event stream. The user events have
	 * arbitrary event id(s): 111, 222, 333, 444 and 555
	 * (possible values are in the range 0...1023).
	 * Every user event with id=(111, ..., 555) has attached
	 * two numerical data (simple event): ({1,11}, ..., {4,44})
	 * and string (string event id=555) "Hello world".
	 */
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, 111, 1, 11));
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, 222, 2, 22));
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, 333, 3, 33));
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, 444, 4, 44));
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT,555, "Hello world" ));

	/*
	 * The main() of this execution flow returns.
	 * However, the main() function of the tracelogger
	 * will return after registering the specified number
	 * of events.
	 */
	return (0);
}

