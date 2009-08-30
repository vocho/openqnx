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

	  Two events from two classes are included and monitored
	  interchangeably. The flow control of monitoring the
	  specified events is controlled with attached event
	  handlers.

	  In order to use this example, start the tracelogger
	  in the deamon mode as:

	  tracelogger -n 1 -d

	  After executing the example, the tracelogger (daemon)
	  will log the specified number of iterations and then
	  terminate. There are no messages printed uppon successful
	  completion of the example. You can view the intercepted
	  events with the traceprinter utility.

	  See accompanied documentation and comments within
	  the example source code for more explanations.
#endif

#include <unistd.h>
#include <sys/trace.h>
#include <sys/kercalls.h>

#include "instrex.h"

/*
 * Prepare event structure where the event data will be
 * stored and passed to an event handler.
 */
event_data_t e_d_1;
_Uint32t     data_array_1[20]; /* 20 elements for potential args. */

event_data_t e_d_2;
_Uint32t     data_array_2[20]; /* 20 elements for potential args. */

/*
 * Global state variable that controls the
 * event flow between two events
 */
int g_state;

/*
 * Event handler attached to the event "ring0" 
 * from the _NTO_TRACE_KERCALL class.
 */
int call_ring0_eh(event_data_t* e_d)
{
	if(g_state) {
		g_state = !g_state;
		return (1);
	}

	return (0);
}

/*
 * Event handler attached to the event _NTO_TRACE_THRUNNING
 * from the _NTO_TRACE_THREAD (thread) class.
 */
int thread_run_eh(event_data_t* e_d)
{
	if(!g_state) {
		g_state = !g_state;
		return (1);
	}

	return (0);
}


int main(int argc, char **argv)
{
	/*
	 * First fill arrays inside event data structures
	 */
	e_d_1.data_array = data_array_1;
	e_d_2.data_array = data_array_2;

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
	 * Set fast emitting mode
	 */
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_SETALLCLASSESFAST));

	/*
	 * Obtain I/O privity before adding event handlers
	 */
	if (ThreadCtl(_NTO_TCTL_IO, 0)!=EOK) { /* obtain I/O privity         */
		(void) fprintf(stderr, "argv[0]: Fail to obtain I/O privity - root privileges\n");

		return (-1);
	}

	/*
	 * Intercept one event from class _NTO_TRACE_KERCALL,
	 * event __KER_MSG_READV.
	 */
	TRACE_EVENT
	(
	 argv[0],
	 TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_KERCALLENTER, __KER_RING0)
	);

	/*
	 * Add event handler to the event "ring0"
	 * from _NTO_TRACE_KERCALL class.
	 */
	TRACE_EVENT
	(
	 argv[0],
	 TraceEvent(_NTO_TRACE_ADDEVENTHANDLER, _NTO_TRACE_KERCALLENTER, __KER_RING0, call_ring0_eh, &e_d_1)
	);

	/*
	 * Intercept one event from class _NTO_TRACE_THREAD
	 */
	TRACE_EVENT
	(
	 argv[0],
	 TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_THREAD, _NTO_TRACE_THRUNNING)
	);


	/*
	 * Add event event handler to the _NTO_TRACE_THRUNNING event
	 * from the _NTO_TRACE_THREAD (thread) class.
	 */
	TRACE_EVENT
	(
	 argv[0],
	 TraceEvent(_NTO_TRACE_ADDEVENTHANDLER, _NTO_TRACE_THREAD, _NTO_TRACE_THRUNNING, thread_run_eh, &e_d_2)
	);

	/*
	 * Start tracing process
	 *
	 * During the tracing process, the tracelogger (which
	 * is being executed in a daemon mode) will log all events.
	 * The number of full logged iterations has been specified
	 * to be 1.
	 */
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_START));

	/*
	 * During one second collect all events
	 */
	(void) sleep(1);

	/*
	 * Stop tracing process by closing the event stream.
	 */
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_STOP));

	/*
	 * Flush the internal buffer since the number
	 * of stored events could be less than
	 * "high water mark" of one buffer (715 events).
	 *
	 * The tracelogger will probably terminate at
	 * this point, since it has been executed with
	 * one iterration (-n 1 "option").
	 */
	TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_FLUSHBUFFER));

	/*
	 * Delete event handlers before exiting to avoid execution
	 * in the missing address space.
	 */
	TRACE_EVENT
	(
	 argv[0],
	 TraceEvent(_NTO_TRACE_DELEVENTHANDLER, _NTO_TRACE_KERCALLENTER, __KER_RING0)
	);
	TRACE_EVENT
	(
	 argv[0],
	 TraceEvent(_NTO_TRACE_DELEVENTHANDLER, _NTO_TRACE_THREAD, _NTO_TRACE_THRUNNING)
	);

	/*
	 * Wait one second before terminating to hold the address space
	 * of the event handlers.
	 */
	(void) sleep(1);

	return (0);
}

