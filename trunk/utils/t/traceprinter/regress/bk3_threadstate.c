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
*	File:		bk3_threadstate.c
*
******************************************************************************
*
*   Contents:	Tests to make sure the instrumentation properly intercepts
*				thread state changes. This test will generate some system
*				activity, then parse through the log and make sure the 
*				thread state information looks basicly sane (the running 
*				thread goes blocked before another thread starts running, 
*				etc)
*
*	Date:		Oct. 11, 2001
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
#include <sys/syspage.h>
#include <sys/stat.h>
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
 *									TYPES									*
 *--------------------------------------------------------------------------*/
typedef struct {
	procfs_debuginfo info;
	char             buff[_POSIX_PATH_MAX];
} proc_name;
typedef struct traceparser_state* tp_state_t;

typedef struct last_state_info {
	int pid;
	int tid;
	int state;
} last_state_info_t;


/*--------------------------------------------------------------------------*
 *									GLOBALS 								*
 *--------------------------------------------------------------------------*/
/* This is a global used by the traceparser callback function to  
 * tell the main thread that the values it got in the events were
 * correct 
 */
static int correct_values;
last_state_info_t * last_state;
const char * const task_state[] = {
    "THDEAD",
    "THRUNNING",
    "THREADY",
    "THSTOPPED",
    "THSEND",
    "THRECEIVE",
    "THREPLY",
    "THSTACK",
    "THWAITTHREAD",
    "THWAITPAGE",
    "THSIGSUSPEND",
    "THSIGWAITINFO",
    "THNANOSLEEP",
    "THMUTEX",
    "THCONDVAR",
    "THJOIN",
    "THINTR",
    "THSEM",
    "THWAITCTX",
    "THNET_SEND",
    "THNET_REPLY"
};

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
*	Purpose: 	This is a traceparcer callback. It will check that the 
*				thread state changes come in in the order we expect them to.				
*
*	Parameters:	header - event header
*				time   - time of the event
*				event_p - pointer to the event array
*				length  - length of the event array
*
*	Returns:	This function will set the value of correct values. It 
*				will be set to -1 on failure, and 1 on success.
*
*****************************************************************************/
int parse_cb(tp_state_t  state, void * nothing, unsigned header, unsigned time, unsigned * event_p, unsigned length)
{
	char buff[200];
	static int nextready[3] = {0,0,0};
	int cpu=_NTO_TRACE_GETCPU(header);
	if (correct_values==0)
		correct_values=1;

	if ((nextready[0]!=0)&&(_NTO_TRACE_GETEVENT(header)!=STATE_READY)) {
		snprintf(buff, sizeof(buff), "Expected a STATE_READY, got %s", task_state[_NTO_TRACE_GETEVENT(header)]);
		testnote(buff);
		correct_values=-1;
	}

	if (last_state[cpu].pid==0) {
		/* If this is the first thread state for this cpu, and it's 
		 * a thread ready event, keep track of it.
		 */
		if (_NTO_TRACE_GETEVENT(header)==STATE_RUNNING) {
			last_state[cpu].pid=event_p[0];
			last_state[cpu].tid=event_p[1];
			last_state[cpu].state=STATE_RUNNING;
		}
		return(EOK);
			
	} else {
		/* If this is not the first event for this cpu, make sure things come in in the right order. */
		switch(_NTO_TRACE_GETEVENT(header)) {
			case STATE_READY:
				if (nextready[0]!=0) {
					if ((event_p[0]!=nextready[0])||(event_p[1]!=nextready[1])) {
						snprintf(buff,sizeof(buff), "Went from running 1 thread, to running another without a block t:%x T1: %d T2:%d  P1:%d P2:%d",
							nextready[2],nextready[0], event_p[0], nextready[1],event_p[1]);
						testnote(buff);
						correct_values=-1;
					}
					nextready[0]=0;
				}
				/* If this ready event is for the currenly running thread, record that.. */
				if ((last_state[cpu].pid==event_p[0])&&(last_state[cpu].tid==event_p[1])) {
					last_state[cpu].state=STATE_READY;
				}
				return(EOK);
				break;
			case STATE_RUNNING:
				/* If a thread is running, make sure the last state we got was the blocking of another
				 * thread 
				 */
				if ((last_state[cpu].pid==event_p[0])&&(last_state[cpu].tid==event_p[1]) &&
					(last_state[cpu].state==STATE_RUNNING)) {
					testnote("We seem to have got 2 runnings in a row for the same process");
					correct_values=-1;
				} else if (last_state[cpu].state==STATE_RUNNING) {
					nextready[0]=last_state[cpu].pid;
					nextready[1]=last_state[cpu].tid;
					nextready[2]=time;
					last_state[cpu].pid=event_p[0];
					last_state[cpu].tid=event_p[1];
				} else  {
					last_state[cpu].pid=event_p[0];
					last_state[cpu].tid=event_p[1];
					last_state[cpu].state=STATE_RUNNING;
				}
				break;
			case STATE_SEND:			
			case STATE_RECEIVE:			
			case STATE_STACK:			
			case STATE_NANOSLEEP:			
			case STATE_CONDVAR:
			case STATE_MUTEX:			
			case STATE_JOIN:			
			case STATE_SEM:			
				/* These states should only be triggerable from a RUNNING thread... */
				if ((last_state[cpu].pid!=event_p[0])||(last_state[cpu].tid!=event_p[1]) ||
					(last_state[cpu].state!=STATE_RUNNING)) {
					snprintf(buff, sizeof(buff),"Got to: %s from %s at t: %x",
						task_state[_NTO_TRACE_GETEVENT(header)], task_state[last_state[cpu].state],time);
					testnote(buff);
					correct_values=-1;
				}
			/* FALL THROUGH */
			default:
				/* For any blocking state, if the block is on a currently running thread remember it */
				if ((last_state[cpu].pid==event_p[0])&&(last_state[cpu].tid==event_p[1])) {
					last_state[cpu].pid=event_p[0];
					last_state[cpu].tid=event_p[1];
					last_state[cpu].state=_NTO_TRACE_GETEVENT(header);
				}


		}
	}
	return(EOK);
	
	
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

	tlpid=spawnlp(P_NOWAIT, "tracelogger", "tracelogger", "-n10", "-d", NULL);
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
	return(tlpid);
}
/****************************************************************************
*
*						Subroutine main
*
*****************************************************************************/
int main(int argc, char *argv[])
{
	int tlkilled,tlpid, rc,status;  //tlpid=trace logger pid
	struct traceparser_state * tp_state;
	struct stat mystat;


	/*
	 * Start the tests.
	 */
	teststart(argv[0]);
	/* Get rid of tracelogger if it is running  */
	tlkilled=kill_tl();
	/* Allocate enough state info structures to keep track of the states
	 * of all cpus
	 */
	last_state=malloc(sizeof(last_state_info_t) * _syspage_ptr->num_cpu);
	assert(last_state!=NULL);
	memset(last_state, 0, sizeof(last_state_info_t) * _syspage_ptr->num_cpu);
	/***********************************************************************/

	/***********************************************************************/
	/*
	 * Make sure the thread state changes are logged properly
	 */
 	testpntbegin("Thread state change ordering");
		
	/* We need to start up the tracelogger in daemon mode, 1 itteration.
	 * we will filter out everything other then thread state changes, and
	 * start logging. 
	 */
	tlpid=start_logger();
	sleep(1);
	/* Set the logger to wide emmiting mode */
	rc=TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE);
	assert(rc!=-1);
	/* Add thread states  */
	rc=TraceEvent(_NTO_TRACE_ADDCLASS, _NTO_TRACE_THREAD);
	assert(rc!=-1);
	
	rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
	assert(rc!=-1);
	/* Generate some system activity */
	while (waitpid(tlpid, &status, WNOHANG)!=tlpid) {
		stat("/proc/", &mystat);
		stat("/dev/pipe", &mystat);
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
	
	if (correct_values==0) 
		testpntfail("Our callback never got called, no events?");
	else if (correct_values==-1)
		testpntfail("Bad");
	else if (correct_values==1)
		testpntpass("Good");
	else 
		testpntfail("This should not happen");

	traceparser_destroy(&tp_state);
 	testpntend();
	/***********************************************************************/
	/* If the tracelogger was running when we started, we should restart it again */
	if (tlkilled==1) 
		system("reopen /dev/null ; tracelogger -n 0 -f /dev/null &");

	teststop(argv[0]);
	return 0;
}


