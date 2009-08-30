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





/*****************************************************************************
*
*	File:		bk1_eventhandler.c
*
******************************************************************************
*
*   Contents:	This test will do some basic sanity checking on event handlers.
*
*	Date:		Sept 27, 2001
*
*	Author:		Peter Graves
*
*	Notes:		This test must have the tracelogger available to it in it's
*				path.  If this is not available the tests will not be 
*				run.
*
*****************************************************************************/

/*--------------------------------------------------------------------------*
 *								STANDARD HEADERS 							*
 *--------------------------------------------------------------------------*/
#include <process.h>
#include <sys/trace.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <sys/traceparser.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/kercalls.h>
#include <unistd.h>
#include <spawn.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <atomic.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/neutrino.h>
#include <sys/procfs.h>
#include <stdio.h>
#include <fcntl.h>



/*--------------------------------------------------------------------------*
 *									LOCAL HEADERS 							*
 *--------------------------------------------------------------------------*/
#ifndef __QNXLOCALREGRESS__
#include "testpoint.h"
#else
#include <regress/testpoint.h>
#endif


/*--------------------------------------------------------------------------*
 *									DEFINES 								*
 *--------------------------------------------------------------------------*/
#define NUM_ITER  1
/*--------------------------------------------------------------------------*
 *									TYPES									*
 *--------------------------------------------------------------------------*/
typedef struct traceparser_state* tp_state_t;

typedef struct {
	procfs_debuginfo info;
	char             buff[_POSIX_PATH_MAX];
} proc_name;


/*--------------------------------------------------------------------------*
 *									GLOBALS 								*
 *--------------------------------------------------------------------------*/
/* This is a global used by the traceparser callback function to  
 * tell the main thread that the values it got in the events were
 * correct 
 */
static int correct_values;
static int correct_values_eh;

/* This is the value that the event handler will return when called. */
int to_return;

/* These globals are used to pass into the event handlers */
unsigned mypid;
unsigned mytid;
event_data_t my_event_data[70];
unsigned data_array[70][30];

/*--------------------------------------------------------------------------*
 *									ROUTINES								*
 *--------------------------------------------------------------------------*/
/****************************************************************************
*
*						Subroutine kill_tl
*
*	Purpose:	This function will read though /proc/<pid>/as and try to
*				find a copy of tracelogger running. If it is found it
*				will be killed.
*
*	Parameters: none
*
*	Returns:	-1 on failures, 0 if tracelogger is not found, and 1 when 
*				tracelogger has been killed.
*
*****************************************************************************/
int kill_tl()
{
	proc_name name;
	char buf[200];
	int fd,rc, curpid,rval;
	DIR * mydir;
	struct dirent * curent;
	mydir=opendir("/proc/");
	assert(mydir!=NULL);
	rval=0;
	while ((curent=readdir(mydir))!=NULL) {
		curpid=atoi(curent->d_name);
		if (curpid>0) {
			memset(buf,0,sizeof(buf));
			snprintf(buf,sizeof(buf), "/proc/%d/as", curpid);
			fd=open(buf,O_RDONLY);
			if (fd==-1)
				continue;
			rc=devctl(fd, DCMD_PROC_MAPDEBUG_BASE, &name, sizeof name, 0);
			if (rc!=EOK)
				continue;
			if (strstr(name.info.path, "tracelogger")!=NULL) {
				/* This is tracelogger */
				kill(curpid, SIGINT);
				/* We should be able to exit here, but we will continue just to make
			 	 * sure there are no more traceloggers to kill 
				 */
				rval=1;
			}
				
		}
	}
	closedir(mydir);
	return(rval);
	
}
/****************************************************************************
*
*						Subroutine parse_cb
*
*	Purpose: 	This will just count the number of events that we get in the 
*				log.
*
*	Parameters:	header - event header
*				time   - time of the event
*				event_p - pointer to the event array
*				length  - length of the event array
*
*	Returns:	This function will set the value of correct values. It will always
*				return EOK
*
*****************************************************************************/
int parse_cb(tp_state_t  state, void * nothing, unsigned header, unsigned time, unsigned * event_p, unsigned length)
{

	/* This is an exceptionally simple parse callback that will just
	 * count the number of times it was called.
	 */
	correct_values++;
	return(EOK);
	
	
}

/****************************************************************************
*
*						Subroutine event_handler
*
*	Purpose: 	This is a simple event handler. It will just return
*				the value in to_return
*
*	Parameters:	event_data - this is a pointer to all the event data
*				
*
*	Returns:	This function returns the value in to_return
*			
*
*****************************************************************************/
int event_handler(event_data_t * event_data)
{
	atomic_add(&correct_values_eh,1);
	return(to_return);
}
/****************************************************************************
*
*						Subroutine start_logger
*
*	Purpose: 	This function will start up the tracelogger, and
*				clear any filters that may be in place.
*
*	Returns: 	Pid of the tracelogger
*
*****************************************************************************/
int start_logger(void) 
{
	int tlpid,rc;
	char buf[100];
	snprintf(buf, sizeof(buf), "-n%d", NUM_ITER);

	tlpid=spawnlp(P_NOWAIT, "tracelogger", "tracelogger",buf, "-d", NULL);
	if (tlpid==-1) {
		snprintf(buf,sizeof(buf), "Unable to start tracelogger (%s)", strerror(errno));
		testerror(buf);
		teststop("Bad");
		exit(0);
	}
	rc=TraceEvent(_NTO_TRACE_DELALLCLASSES);
	assert(rc!=-1);
	rc=TraceEvent(_NTO_TRACE_CLRCLASSPID, _NTO_TRACE_KERCALL);
	assert(rc!=-1);
	rc=TraceEvent(_NTO_TRACE_CLRCLASSTID, _NTO_TRACE_KERCALL);
	assert(rc!=-1);
	rc=TraceEvent(_NTO_TRACE_CLRCLASSPID, _NTO_TRACE_THREAD);
	assert(rc!=-1);
	rc=TraceEvent(_NTO_TRACE_CLRCLASSTID, _NTO_TRACE_THREAD);
	assert(rc!=-1);
	rc=TraceEvent(_NTO_TRACE_CLRCLASSPID, _NTO_TRACE_VTHREAD);
	assert(rc!=-1);
	rc=TraceEvent(_NTO_TRACE_CLRCLASSTID, _NTO_TRACE_VTHREAD);
	assert(rc!=-1);
	return(tlpid);
}
/****************************************************************************
*
*						Subroutine main
*
*****************************************************************************/
int main(int argc, char *argv[])
{
	int tlkilled,tlpid, rc,status,x,firstfail,child;  /*tlpid=trace logger pid*/
	struct traceparser_state * tp_state;
	struct stat statbuf;
	char buf[100];
	int events[] = {__KER_MSG_SENDV,__KER_MSG_SENDVNC,__KER_MSG_ERROR,__KER_MSG_RECEIVEV,__KER_MSG_REPLYV, 
	__KER_MSG_READV,__KER_MSG_WRITEV,__KER_MSG_READWRITEV,__KER_MSG_INFO,__KER_MSG_SEND_PULSE,__KER_MSG_DELIVER_EVENT, 
	__KER_MSG_KEYDATA,__KER_MSG_READIOV,__KER_MSG_RECEIVEPULSEV,__KER_MSG_VERIFY_EVENT,__KER_SIGNAL_KILL, __KER_SIGNAL_RETURN,
	__KER_SIGNAL_FAULT, __KER_SIGNAL_ACTION, __KER_SIGNAL_PROCMASK,__KER_SIGNAL_SUSPEND,__KER_SIGNAL_WAITINFO,__KER_CHANNEL_CREATE,
	__KER_CHANNEL_DESTROY,__KER_CONNECT_ATTACH,__KER_CONNECT_DETACH,__KER_CONNECT_SERVER_INFO,__KER_CONNECT_CLIENT_INFO, 
	__KER_CONNECT_FLAGS,__KER_THREAD_CREATE,__KER_THREAD_DESTROY,__KER_THREAD_DESTROYALL, __KER_THREAD_DETACH,  
	__KER_THREAD_JOIN,__KER_THREAD_CANCEL,__KER_THREAD_CTL,__KER_SYNC_CREATE,__KER_SYNC_DESTROY,__KER_SYNC_MUTEX_LOCK, 
	__KER_SYNC_MUTEX_UNLOCK,__KER_SYNC_CONDVAR_WAIT,__KER_SYNC_CONDVAR_SIGNAL,__KER_SYNC_SEM_POST, 
	__KER_SYNC_SEM_WAIT,__KER_SYNC_CTL,__KER_SYNC_MUTEX_REVIVE, /* 46 kernel calls*/
	_NTO_TRACE_THCONDVAR,_NTO_TRACE_THCREATE,_NTO_TRACE_THDEAD,_NTO_TRACE_THDESTROY,_NTO_TRACE_THINTR,
	_NTO_TRACE_THJOIN,_NTO_TRACE_THMUTEX,_NTO_TRACE_THNANOSLEEP,_NTO_TRACE_THNET_REPLY,_NTO_TRACE_THNET_SEND,
	_NTO_TRACE_THREADY,_NTO_TRACE_THRECEIVE,_NTO_TRACE_THREPLY,_NTO_TRACE_THRUNNING,_NTO_TRACE_THSEM,
	_NTO_TRACE_THSEND,_NTO_TRACE_THSIGSUSPEND,_NTO_TRACE_THSIGWAITINFO,_NTO_TRACE_THSTACK,_NTO_TRACE_THSTOPPED,
	_NTO_TRACE_THWAITCTX,_NTO_TRACE_THWAITPAGE,_NTO_TRACE_THWAITTHREAD, /* 23 thread states */
	-1};

	/*
	 * Start the tests.
	 */
	teststart(argv[0]);
	/* Get rid of tracelogger if it is running  */
	tlkilled=kill_tl();
	/***********************************************************************/

	/***********************************************************************/
	/*
	 * This is a simple test to make sure we need to have io privity to 
	 * add event handlers.
	 */
 	testpntbegin("need io privity to add event handler");

	errno=0;
	my_event_data[0].data_array=data_array[0];
	rc=TraceEvent(_NTO_TRACE_ADDEVENTHANDLER, _NTO_TRACE_THREAD,1<<STATE_READY, event_handler, &my_event_data[0]);
	if (rc!=-1) {
		testpntfail("Was able to attach an event handler");
		TraceEvent(_NTO_TRACE_DELEVENTHANDLER, _NTO_TRACE_THREAD,1<<STATE_READY);
	}else {
		if (errno==EPERM) 
			testpntpass("Good");
		else
			testpntfail(strerror(errno));
	}
 	testpntend();

	/* We should need io privity to attach event handlers */
	rc=ThreadCtl( _NTO_TCTL_IO, 0 );
	assert(rc!=-1);

	/***********************************************************************/

	/***********************************************************************/
	/*
	 * Make sure that if the event_handler returns 1 that the events get
	 * logged.
	 */
 	testpntbegin("Event handler returns 1");
	to_return=1;
	correct_values_eh=0;
		
	/* We need to start up the tracelogger in daemon mode, 1 itteration.
	 * we will filter out everything other then our thread state, then 
	 * start logging. 
	 */
	tlpid=start_logger();
	sleep(1);
	/* Set the logger to wide emmiting mode */
	rc=TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE);
	assert(rc!=-1);
	/* Add the classes we want to look at */
	rc=TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_THREAD, 1<<STATE_READY);
	assert(rc!=-1);
	/* Setup the event handler */
	memset(&my_event_data, 0, sizeof(my_event_data));
	memset(data_array, 0, sizeof(data_array));
	my_event_data[0].data_array=data_array[0];
	TraceEvent(_NTO_TRACE_ADDEVENTHANDLER, _NTO_TRACE_THREAD,1<<STATE_READY, event_handler, &my_event_data[0]);
	
	rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
	assert(rc!=-1);
	/* Make sure there is some system activity going on */
	while(waitpid(tlpid, &status, WNOHANG)!=tlpid ) {
		stat("/proc/", &statbuf);

	}
	/* Now, setup the traceparser lib to pull out the thread state events
	 * and make sure our event shows up 
	 */
	tp_state=traceparser_init(NULL);
	assert(tp_state!=NULL);
	traceparser_cs_range(tp_state, NULL, parse_cb,_NTO_TRACE_THREAD, _NTO_TRACE_THDEAD, _NTO_TRACE_THDESTROY);

	/* Since we don't want a bunch of output being displayed in the 
	 * middle of the tests, turn off verbose output.
	 */
	traceparser_debug(tp_state, stdout, _TRACEPARSER_DEBUG_NONE);
	/* Set correct_values to 0, so we can see if the callback actually
	 * got called. 
	 */
	correct_values=0;
	/* And parse the tracebuffer */
	traceparser(tp_state, NULL, "/dev/shmem/tracebuffer");
	
	if ((correct_values<=correct_values_eh)  && (correct_values>0))
		testpntpass("Good");
	else if (correct_values==0) 
		testpntfail("No events were logged");
	else if (correct_values_eh<correct_values)
		testpntfail("Different number of events in eventhandler and tracebuffer");

	TraceEvent(_NTO_TRACE_DELEVENTHANDLER, _NTO_TRACE_THREAD,1<<STATE_READY);
	traceparser_destroy(&tp_state);
 	testpntend();
	/***********************************************************************/

	/***********************************************************************/
	/*
	 * Make sure that if the event_handler returns 1 that the events get
	 * logged.
	 */
 	testpntbegin("Event handler returns 0");
	to_return=0;
	correct_values_eh=0;
		
	/* We need to start up the tracelogger in daemon mode, 1 itteration.
	 * we will filter out everything other then our thread state, then 
	 * start logging. 
	 */
	tlpid=start_logger();
	sleep(1);
	/* Set the logger to wide emmiting mode */
	rc=TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE);
	assert(rc!=-1);
	/* Add the classes we want to look at */
	rc=TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_THREAD, 1<<STATE_READY);
	assert(rc!=-1);
	/* Setup the event handler */
	memset(&my_event_data, 0, sizeof(my_event_data));
	memset(data_array, 0, sizeof(data_array));
	my_event_data[0].data_array=data_array[0];
	TraceEvent(_NTO_TRACE_ADDEVENTHANDLER, _NTO_TRACE_THREAD,1<<STATE_READY, event_handler, &my_event_data[0]);
	
	rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
	assert(rc!=-1);
	/* Make sure there is some system activity going on */
	x=0;
	while(waitpid(tlpid, &status, WNOHANG)!=tlpid ) {
		stat("/proc/", &statbuf);
		x++;
		if (x>100)
			TraceEvent(_NTO_TRACE_FLUSHBUFFER);

	}
	/* Now, setup the traceparser lib to pull out the thread state events
	 * and make sure our event shows up 
	 */
	tp_state=traceparser_init(NULL);
	assert(tp_state!=NULL);
	traceparser_cs_range(tp_state, NULL, parse_cb,_NTO_TRACE_THREAD, _NTO_TRACE_THDEAD, _NTO_TRACE_THDESTROY);

	/* Since we don't want a bunch of output being displayed in the 
	 * middle of the tests, turn off verbose output.
	 */
	traceparser_debug(tp_state, stdout, _TRACEPARSER_DEBUG_NONE);
	/* Set correct_values to 0, so we can see if the callback actually
	 * got called. 
	 */
	correct_values=0;
	/* And parse the tracebuffer */
	traceparser(tp_state, NULL, "/dev/shmem/tracebuffer");
	
	if ((correct_values_eh>0) && (correct_values==0)) {
		testpntpass("Good");
	} else if (correct_values_eh==0) {
		testpntfail("Event handler was not called");
	} else
		testpntfail("Events were logged when event handler returned 0");

	TraceEvent(_NTO_TRACE_DELEVENTHANDLER, _NTO_TRACE_THREAD,1<<STATE_READY);
	traceparser_destroy(&tp_state);
 	testpntend();
	/***********************************************************************/

	/***********************************************************************/
	/*
	 * Make sure that if the event_handler returns a large number (other 
	 * then 1) that all events are still logged.
	 */
 	testpntbegin("Event handler returns a large number");
	to_return=9999999;
	correct_values_eh=0;
		
	/* We need to start up the tracelogger in daemon mode, 1 itteration.
	 * we will filter out everything other then our thread state, then 
	 * start logging. 
	 */
	tlpid=start_logger();
	sleep(1);
	/* Set the logger to wide emmiting mode */
	rc=TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE);
	assert(rc!=-1);
	/* Add the classes we want to look at */
	rc=TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_THREAD, 1<<STATE_READY);
	assert(rc!=-1);
	/* Setup the event handler */
	memset(&my_event_data, 0, sizeof(my_event_data));
	memset(data_array, 0, sizeof(data_array));
	my_event_data[0].data_array=data_array[0];
	TraceEvent(_NTO_TRACE_ADDEVENTHANDLER, _NTO_TRACE_THREAD,1<<STATE_READY, event_handler, &my_event_data[0]);
	
	errno=0;
	rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
	if (rc==-1) {
		testnote(strerror(errno));
	}
	assert(rc!=-1);
	/* Make sure there is some system activity going on */
	while(waitpid(tlpid, &status, WNOHANG)!=tlpid ) {
		stat("/proc/", &statbuf);

	}
	/* Now, setup the traceparser lib to pull out the thread state events
	 * and make sure our event shows up 
	 */
	tp_state=traceparser_init(NULL);
	assert(tp_state!=NULL);
	traceparser_cs_range(tp_state, NULL, parse_cb,_NTO_TRACE_THREAD, _NTO_TRACE_THDEAD, _NTO_TRACE_THDESTROY);

	/* Since we don't want a bunch of output being displayed in the 
	 * middle of the tests, turn off verbose output.
	 */
	traceparser_debug(tp_state, stdout, _TRACEPARSER_DEBUG_NONE);
	/* Set correct_values to 0, so we can see if the callback actually
	 * got called. 
	 */
	correct_values=0;
	/* And parse the tracebuffer */
	traceparser(tp_state, NULL, "/dev/shmem/tracebuffer");
	
	if ((correct_values<=correct_values_eh)  && (correct_values>0))
		testpntpass("Good");
	else if (correct_values==0) 
		testpntfail("No events were logged");
	else if (correct_values_eh<correct_values)
		testpntfail("Different number of events in eventhandler and tracebuffer");

	traceparser_destroy(&tp_state);
	TraceEvent(_NTO_TRACE_DELEVENTHANDLER, _NTO_TRACE_THREAD,1<<STATE_READY);
 	testpntend();
	/***********************************************************************/

	/***********************************************************************/
	/*
	 * Make sure that if the event_handler returns a negitive number that
	 * that all events are still logged.
	 */
 	testpntbegin("Event handler returns a negitive");
	to_return=-2;
	correct_values_eh=0;
		
	/* We need to start up the tracelogger in daemon mode, 1 itteration.
	 * we will filter out everything other then our thread state, then 
	 * start logging. 
	 */
	tlpid=start_logger();
	sleep(1);
	/* Set the logger to wide emmiting mode */
	rc=TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE);
	assert(rc!=-1);
	/* Add the classes we want to look at */
	rc=TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_THREAD, 1<<STATE_READY);
	assert(rc!=-1);
	/* Setup the event handler */
	memset(&my_event_data, 0, sizeof(my_event_data));
	memset(data_array, 0, sizeof(data_array));
	my_event_data[0].data_array=data_array[0];
	TraceEvent(_NTO_TRACE_ADDEVENTHANDLER, _NTO_TRACE_THREAD,1<<STATE_READY, event_handler, &my_event_data[0]);
	
	errno=0;
	rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
	if (rc==-1) {
		testnote(strerror(errno));
	}
	assert(rc!=-1);
	/* Make sure there is some system activity going on */
	while(waitpid(tlpid, &status, WNOHANG)!=tlpid ) {
		stat("/proc/", &statbuf);

	}
	/* Now, setup the traceparser lib to pull out the thread state events
	 * and make sure our event shows up 
	 */
	tp_state=traceparser_init(NULL);
	assert(tp_state!=NULL);
	traceparser_cs_range(tp_state, NULL, parse_cb,_NTO_TRACE_THREAD, _NTO_TRACE_THDEAD, _NTO_TRACE_THDESTROY);

	/* Since we don't want a bunch of output being displayed in the 
	 * middle of the tests, turn off verbose output.
	 */
	traceparser_debug(tp_state, stdout, _TRACEPARSER_DEBUG_NONE);
	/* Set correct_values to 0, so we can see if the callback actually
	 * got called. 
	 */
	correct_values=0;
	/* And parse the tracebuffer */
	traceparser(tp_state, NULL, "/dev/shmem/tracebuffer");
	
	if ((correct_values<=correct_values_eh)  && (correct_values>0))
		testpntpass("Good");
	else if (correct_values==0) 
		testpntfail("No events were logged");
	else if (correct_values_eh<correct_values)
		testpntfail("Different number of events in eventhandler and tracebuffer");

	traceparser_destroy(&tp_state);
	TraceEvent(_NTO_TRACE_DELEVENTHANDLER, _NTO_TRACE_THREAD,1<<STATE_READY);
 	testpntend();
	/***********************************************************************/

	/***********************************************************************/
	/*
	 * At the time of writing, there is a system limit of 64 event handlers 
	 * that can be attached at one time. Try to attach 64 to make sure it 
	 * works as expected.
	 */
 	testpntbegin("64 event handlers attached");
	to_return=1;
	correct_values_eh=0;
		
	/* We need to start up the tracelogger in daemon mode, 1 itteration.
	 * we will filter out everything other then our thread state, then 
	 * start logging. 
	 */
	tlpid=start_logger();
	sleep(1);
	/* Set the logger to wide emmiting mode */
	rc=TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE);
	assert(rc!=-1);
	/* Add the classes we want to look at */
	rc=TraceEvent(_NTO_TRACE_ADDCLASS, _NTO_TRACE_KERCALLENTER);
	assert(rc!=-1);
	/* Setup the event handlers */
	memset(&my_event_data, 0, sizeof(my_event_data));
	memset(data_array, 0, sizeof(data_array));
	for (x=0;x<64;x++) {
		my_event_data[x].data_array=data_array[x];
		rc=TraceEvent(_NTO_TRACE_ADDEVENTHANDLER, _NTO_TRACE_KERCALLENTER,x, event_handler, &my_event_data[x]);
		if (rc==-1)
			break;
	}
	errno=0;
	rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
	if (rc==-1) {
		testnote(strerror(errno));
	}
	assert(rc!=-1);
	/* Make sure there is some system activity going on */
	while(waitpid(tlpid, &status, WNOHANG)!=tlpid ) {
		stat("/proc/", &statbuf);

	}
	/* Now, setup the traceparser lib to pull out the thread state events
	 * and make sure our event shows up 
	 */
	tp_state=traceparser_init(NULL);
	assert(tp_state!=NULL);
	traceparser_cs_range(tp_state, NULL, parse_cb,_NTO_TRACE_KERCALLENTER, 0, 64);

	/* Since we don't want a bunch of output being displayed in the 
	 * middle of the tests, turn off verbose output.
	 */
	traceparser_debug(tp_state, stdout, _TRACEPARSER_DEBUG_NONE);
	/* Set correct_values to 0, so we can see if the callback actually
	 * got called. 
	 */
	correct_values=0;
	/* And parse the tracebuffer */
	traceparser(tp_state, NULL, "/dev/shmem/tracebuffer");
	
	if (x<64) {
		snprintf(buf,sizeof(buf), "Could only attach %d event handler\n", x);
		testpntfail(buf);
	} else if ((correct_values<=correct_values_eh)  && (correct_values>0))
		testpntpass("Good");
	else if (correct_values==0) 
		testpntfail("No events were logged");
	else if (correct_values_eh<correct_values)
		testpntfail("Different number of events in eventhandler and tracebuffer");

	traceparser_destroy(&tp_state);
	for (x=0;x<64;x++) {
		TraceEvent(_NTO_TRACE_DELEVENTHANDLER, _NTO_TRACE_KERCALLENTER,x);
	}
 	testpntend();
	/***********************************************************************/

	/***********************************************************************/
	/*
	 * At the time of writing, there is a system limit of 64 event handlers 
	 * that can be attached at one time. Try to attach more then 64 to make
	 * sure it works as expected.
	 */
 	testpntbegin("More then 64 event handlers attached");
	to_return=1;
	correct_values_eh=0;
		
	/* We need to start up the tracelogger in daemon mode, 1 itteration.
	 * we will filter out everything other then our thread state, then 
	 * start logging. 
	 */
	tlpid=start_logger();
	sleep(1);
	/* Set the logger to wide emmiting mode */
	rc=TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE);
	assert(rc!=-1);
	/* Add the classes we want to look at */
	rc=TraceEvent(_NTO_TRACE_ADDCLASS, _NTO_TRACE_KERCALLENTER);
	assert(rc!=-1);
	rc=TraceEvent(_NTO_TRACE_ADDCLASS, _NTO_TRACE_THREAD);
	assert(rc!=-1);
	rc=TraceEvent(_NTO_TRACE_ADDCLASS, _NTO_TRACE_VTHREAD);
	assert(rc!=-1);
	rc=TraceEvent(_NTO_TRACE_ADDCLASS, _NTO_TRACE_PROCESS);
	assert(rc!=-1);
	/* Setup the event handlers */
	memset(&my_event_data, 0, sizeof(my_event_data));
	memset(data_array, 0, sizeof(data_array));
	firstfail=-1;
	for (x=0;x<69;x++) {
		my_event_data[x].data_array=data_array[x];
		rc=TraceEvent(_NTO_TRACE_ADDEVENTHANDLER,(x>45)?_NTO_TRACE_THREAD:_NTO_TRACE_KERCALLENTER,events[x], event_handler, &my_event_data[x]);
		if ((rc==-1) &&(firstfail==-1))
			firstfail=x;

	}
	errno=0;
	rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
	if (rc==-1) {
		testnote(strerror(errno));
	}
	assert(rc!=-1);
	/* Make sure there is some system activity going on */
	while(waitpid(tlpid, &status, WNOHANG)!=tlpid ) {
		stat("/proc/", &statbuf);

	}
	/* Now, setup the traceparser lib to pull out the thread state events
	 * and make sure our event shows up 
	 */
	tp_state=traceparser_init(NULL);
	assert(tp_state!=NULL);
	traceparser_cs_range(tp_state, NULL, parse_cb,_NTO_TRACE_KERCALLENTER, 0, 64);

	/* Since we don't want a bunch of output being displayed in the 
	 * middle of the tests, turn off verbose output.
	 */
	traceparser_debug(tp_state, stdout, _TRACEPARSER_DEBUG_NONE);
	/* Set correct_values to 0, so we can see if the callback actually
	 * got called. 
	 */
	correct_values=0;
	/* And parse the tracebuffer */
	traceparser(tp_state, NULL, "/dev/shmem/tracebuffer");
	
	if (firstfail<64) {
		snprintf(buf,sizeof(buf), "Could only attach %d event handler\n", firstfail);
		testpntfail(buf);
	} else if (firstfail>64) {
		snprintf(buf,sizeof(buf), "Was allowed to attach %d handler, should only be 64\n", firstfail);
		testpntfail(buf);
	} else if ((correct_values<=correct_values_eh)  && (correct_values>0))
		testpntpass("Good");
	else if (correct_values==0) 
		testpntfail("No events were logged");
	else if (correct_values_eh<correct_values)
		testpntfail("Different number of events in eventhandler and tracebuffer");

	traceparser_destroy(&tp_state);
	for (x=0;x<69;x++) {
		TraceEvent(_NTO_TRACE_DELEVENTHANDLER,(x>45)?_NTO_TRACE_THREAD:_NTO_TRACE_KERCALLENTER,events[x]);
	}
 	testpntend();
	/***********************************************************************/

	/***********************************************************************/
	/*
	 * At the time of writing, there is a system limit of 64 event handlers 
	 * that can be attached at one time. Try to attach more then 64 to make
	 * sure it works as expected.
	 * This test will do the same as above, but it will fork a child to add
	 * the event handlers. The child will then exit without starting 
	 * tracelogger or removing the event handlers. This is just to make
	 * sure it does not cause any problems.
	 */
 	testpntbegin("64+ event handlers - not cleaned up ");
	to_return=1;
	correct_values_eh=0;
		
	child=fork();
	assert(child!=-1);
	if (child==0) {
		/* Set the logger to wide emmiting mode */
		rc=TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE);
		assert(rc!=-1);
		/* Add the classes we want to look at */
		rc=TraceEvent(_NTO_TRACE_ADDCLASS, _NTO_TRACE_KERCALL);
		assert(rc!=-1);
		rc=TraceEvent(_NTO_TRACE_ADDCLASS, _NTO_TRACE_THREAD);
		assert(rc!=-1);
		rc=TraceEvent(_NTO_TRACE_ADDCLASS, _NTO_TRACE_VTHREAD);
		assert(rc!=-1);
		rc=TraceEvent(_NTO_TRACE_ADDCLASS, _NTO_TRACE_PROCESS);
		assert(rc!=-1);
		/* Setup the event handlers */
		memset(&my_event_data, 0, sizeof(my_event_data));
		memset(data_array, 0, sizeof(data_array));
		firstfail=-1;
		for (x=0;x<69;x++) {
			my_event_data[x].data_array=data_array[x];
			rc=TraceEvent(_NTO_TRACE_ADDEVENTHANDLER,(x>45)?_NTO_TRACE_THREAD:_NTO_TRACE_KERCALLENTER,events[x], event_handler, &my_event_data[x]);
			if ((rc==-1) &&(firstfail==-1))
				firstfail=x;
	
		}
		exit(0);
	}
	rc=waitpid(child, &x, 0);
	assert(rc==child);
	child=fork();
	assert(child!=-1);
	/* Make a request to proc to generate a bit of activity */
	if (child==0) {
		exit(0);
	}
	rc=waitpid(child, &x, 0);
	assert(rc==child);
	testpntpass("OK");
 	testpntend();
	/***********************************************************************/
	/* If the tracelogger was running when we started, we should restart it again */
	if (tlkilled==1) 
		system("reopen /dev/null ; tracelogger -n 0 -f /dev/null &");

	teststop(argv[0]);
	return 0;
}


