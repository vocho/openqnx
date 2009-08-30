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
*	File:		bk2_kercalls.c
*
******************************************************************************
*
*   Contents:	Tests to make sure the instrumentation properly intercepts
*				kerel calls, and their arguments on kernel exit 
*
*	Date:		August 2, 2001
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
typedef struct traceparser_state* tp_state_t;
typedef struct {
	procfs_debuginfo info;
	char             buff[_POSIX_PATH_MAX];
} proc_name;


/*--------------------------------------------------------------------------*
 *									FUNCTION DEFINITIONS					*
 *--------------------------------------------------------------------------*/
int trigger_kercall_event(unsigned * event_p);
/* The folloing 2 defines are for kernel calls that are not 
 * included in any public headers, these are just to quiet 
 * compile time warnings 
 */
void __kernop(void);
unsigned __SysCpupageGet(unsigned);


/*--------------------------------------------------------------------------*
 *									MACROS		 							*
 *--------------------------------------------------------------------------*/
/* Macros for the mode to start the traceparser in */
#define FAST 1
#define WIDE 0

/* These are the two text strings to send back and forth
 * They are both a total of 4 bytes (the size of an unsigned)
 * so that they should all fit in a single event, without
 * any problems.
 */
#define SEND "123"
#define REPLY "456"

/* This is the value that we should get from a ConnectAttach for
 * the first side channel connection... The test will assert
 * out if this is not true 
 */
#define COID 1073741826


/* Nanoseconds in a second */
#define NS_IN_SEC (1000000000L)
/*--------------------------------------------------------------------------*
 *									GLOBALS 								*
 *--------------------------------------------------------------------------*/
/* This is a global used by the traceparser callback function to  
 * tell the main thread that the values it got in the events were
 * correct 
 */
static int correct_values;

/* This is a global that is set to tell us if we are in fast mode (1)
 * or wide mode (0)
 */
static int fast_mode;

/* 
 * This is a pointer to the current test being preformed as defined
 * in the event array in main.
 */
unsigned * cur_event;

/* This is the channel that will be setup in main, and used by trigger_event
 * to trigger the various message related kernel calls.
 */
int chid;
 
/* This global array will be used to pass the expected results from the 
 * trigger_kercall_event function to the parseing callback. 
 */
unsigned global_vals[24];


/* Buffer to send and receive messages in and the iovs we will use */
char reply_buf[100];
iov_t send, reply;


/* These are used to let us spawn copies of this program
 * if needed 
 */
char * path;
char *token[] = { "childproc", NULL, NULL };


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
*						Subroutine sig_hand 
*
*	Purpose: 	This is a simple signal handler that does nothing at all
*
*
*****************************************************************************/
void sig_hand(int signo, siginfo_t *info, void *other)
{
	return;
}

/****************************************************************************
*
*						Subroutine can_stub
*
*	Purpose: 	This is a simple cancelation stub which will exit the 
*				current thread
*
*
*****************************************************************************/
static void can_stub( void )
{
        pthread_exit( PTHREAD_CANCELED );
}
/****************************************************************************
*
*						Subroutine nothing_thread 
*
*	Purpose: 	This is a do nothing thread that will just sleep for a couple
*				seconds then exit. This is used to test the various thread
*				related kernel calls against.	
*
*	Parameters:	None
*
*	Returns:	Nothing
*			
*
*****************************************************************************/
void * nothing_thread(void * arg)
{
	sleep(4);
	return((void *)0xabcdef12);
}
/****************************************************************************
*
*						Subroutine msg_thread
*
*	Purpose: 	This is a thread that will be used as a target for triggering
*				all of the message related kernel calls. It will sit recieved 
*				blocked on a channel created in main, and will just receive 
*				messages, and depending on the contents of the message it will:
*				reply, send another message, send a pulse, or signal a condvar
*
*	Parameters:	None
*
*	Returns:	Nothing
*			
*
*****************************************************************************/
void * msg_thread(void * arg)
{
	int rcvid,rc;
	char buf[100];
	while(1) {
		rcvid=MsgReceive(chid, buf,sizeof(buf), NULL);
		//Ignore pulses
		if (rcvid==0)
			continue;
		rc=MsgReply(rcvid, EOK,  REPLY, 3);
		if (buf[0]=='S') {
			/* If their message starts with an S they want us to send
			 * them back a message
	 		 */
			MsgSend(COID, "MESSAGE", 8, buf, sizeof(buf));
		} else if (buf[0]=='P') {
			/* If the message starts with a P we should send a Pulse */
			MsgSendPulse(COID, 10,10,10);
		} else if (buf[0]=='C') {
			/* If the message starts with a C we should signal a condvar
			 * who's sync object pointer should be in global_val1, in	
			 * 4 seconds (gives the sender a chance to get condvar blocked.
			 */
			sleep(4);
			SyncCondvarSignal((sync_t *)global_vals[0], 1);
		} else if (buf[0]=='K') {
			/* If the message starts with kill, we should 
			 * SignalKill ourselves with a SIGUSR2
			 */	
			sleep(1);
			kill(getpid(), SIGUSR1);
		}
		delay(100);
	}
	
}
/****************************************************************************
*
*						Subroutine parse_cb
*
*	Purpose: 	This is a traceparcer callback for all events. It will check 
*				the given event length, and parameter values against those
*				that are currenly pointed to in the global cur_event
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
	int x;
	char buf[100];

	/* This is to handle the case when we may see multiple events in the 
	 * trace logger.  If we have seen the correct event, then we can just
	 * ignore this one.  This was put in mostly to deal the the TraceEvent
	 * call which will force us to deal with multiple events
	 */
	if (correct_values==1) 
		return(EOK);
	/*
	 * Now check that the values that we recieved in the callback are the expected values.
	 * The values we expecte are 
	 */
	if (fast_mode==WIDE) {
		if (length!=cur_event[0]) {
 			snprintf(buf,sizeof(buf), "Expected %d parameters got %d", cur_event[0],
				length);
			testnote(buf);
			correct_values=-1;
			return(EOK);
		}
		for (x=0;x<(cur_event[1]);x++) {
			if (event_p[x]!=global_vals[x]) {
				snprintf(buf,sizeof(buf), "Expected value: %d for parameter %x but got %d", 
						global_vals[x],x, event_p[x]);
				testnote(buf);
				correct_values=-1;
				return(EOK);
			}
		}
		if (x==(cur_event[1]))
			correct_values=1;
			

	
	} else {
		/* if we are not in fast mode we only get the two most important values */

		if ((event_p[0]==global_vals[cur_event[4]]) && 
			(cur_event[1]>1)?(event_p[1]==global_vals[cur_event[5]]):1) {
			correct_values=1;
		} else {
			snprintf(buf,sizeof(buf), "Expected values %d and %d  Got values %d and %d", 
				global_vals[cur_event[4]],global_vals[cur_event[5]],
				event_p[0],event_p[1]);
			testnote(buf);
			correct_values=-1;
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

	tlpid=spawnlp(P_NOWAIT, "tracelogger", "tracelogger", "-n1", "-d", NULL);
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
	struct traceparser_state *tp_state;
	char message[200];
	/*
	 *  This array will define all the tests to be preformed.  It is atually an 
	 *  an array of arrays, where the outside array contains the arrys of tests.
	 *  Each tests array is of the format:
	 *  Number of parameters, Numer of valid parameters Kernel call number, 
	 *  kernel call name, Number of  first fast mode parameter, number of 
	 *	second fast mode parameter.
	 *  This differs from bk1_kercalls, in that all the expected values will be 
	 *  put in the global_vals array
	 * 
	 *  For example, for the SignalKill kernel call event it would be:
	 *  2, 1, __KER_SIGNAL_KILL, 0,1
	 * 
	 */
	unsigned events[][6] = { 
		{2, 0, __KER_NOP, (unsigned)"NOP", 0, 1 },
		{2, 1, __KER_TRACE_EVENT, (unsigned)"TraceEvent", 0, 1},
		{2, 1, __KER_SYS_CPUPAGE_GET, (unsigned)"__SysCpupageGet", 0, 1},
		{4, 4, __KER_MSG_SENDV, (unsigned)"MsgSendv", 0, 1},
		{4, 4,  __KER_MSG_SENDVNC, (unsigned)"MsgSendvnc", 0, 1},
		{2, 1, __KER_MSG_ERROR, (unsigned)"MsgError", 0, 1},
		{18, 17, __KER_MSG_RECEIVEV, (unsigned)"MsgReceivev", 0, 1},
		{2, 1, __KER_MSG_REPLYV, (unsigned)"MsgReplyv", 0, 1},
		{4, 4, __KER_MSG_READV, (unsigned)"MsgReadv", 0, 1},
		{2, 1, __KER_MSG_WRITEV, (unsigned)"MsgWritev", 0, 1},
		{14, 14, __KER_MSG_INFO, (unsigned)"MsgInfo", 0, 1},
		{2, 1, __KER_MSG_SEND_PULSE, (unsigned)"MsgSendPulse", 0, 1},
		{2, 2, __KER_MSG_DELIVER_EVENT, (unsigned)"MsgDeliverEvent", 0, 1},
		{2, 2, __KER_MSG_KEYDATA, (unsigned)"MsgKeyData", 0, 1},
		{4, 1, __KER_MSG_READIOV, (unsigned)"MsgReadiov", 0, 1},
		{18, 17, __KER_MSG_RECEIVEPULSEV, (unsigned)"MsgReceivePulsev"},
		{2, 1, __KER_MSG_VERIFY_EVENT, (unsigned)"MsgVerifyEvent", 0, 1},
		{2, 1, __KER_SIGNAL_KILL, (unsigned)"SignalKill", 0, 1 },
		{6, 5, __KER_SIGNAL_ACTION, (unsigned)"SignalAction", 0, 1},
		{4, 3, __KER_SIGNAL_PROCMASK, (unsigned)"SignalProcmask", 0, 1},
		{2, 2, __KER_SIGNAL_SUSPEND, (unsigned)"SignalSuspend", 0, 1},
		{12, 1, __KER_SIGNAL_WAITINFO, (unsigned)"SignalWaitinfo", 0, 2},
		{2, 1, __KER_CHANNEL_CREATE, (unsigned)"ChannelCreate", 0, 1},
		{2, 1, __KER_CHANNEL_DESTROY, (unsigned)"ChannelDestroy", 0, 1},
		{2, 1, __KER_CONNECT_ATTACH, (unsigned)"ConnectAttach", 0, 1},
		{2, 1, __KER_CONNECT_DETACH, (unsigned)"ConnectDetach", 0, 1},
		{14, 14, __KER_CONNECT_SERVER_INFO, (unsigned)"ConnectServerInfo", 0, 1},
		{20, 20, __KER_CONNECT_CLIENT_INFO, (unsigned)"ConnectClientInfo", 0, 1},
		{2, 1, __KER_CONNECT_FLAGS, (unsigned)"ConnectFlags", 0, 1},
		{2, 1, __KER_THREAD_CREATE, (unsigned)"ThreadCreate", 0, 1},
		{2, 1, __KER_THREAD_DESTROY, (unsigned)"ThreadDestroy", 0, 1},
#if 0
		/*  This event will not currently happen, as the ThreadDestroyAll
		 *  will remove the process, so there's nothing to exit to
	  	 */
		{2, 1, __KER_THREAD_DESTROYALL, (unsigned)"ThreadDestroyAll", 0, 1},
#endif
		{2, 1, __KER_THREAD_DETACH, (unsigned)"ThreadDetach", 0, 1},
		{2, 2, __KER_THREAD_JOIN, (unsigned)"ThreadJoin", 0, 1},
		{2, 1, __KER_THREAD_CANCEL, (unsigned)"ThreadCancel", 0, 1},
		{2, 1, __KER_THREAD_CTL, (unsigned)"ThreadCtl", 0, 1},
		{4, 3, __KER_CLOCK_TIME, (unsigned)"ClockTime", 0, 1},
		{4, 3, __KER_CLOCK_ADJUST, (unsigned)"ClockAdjust", 0, 1},
		{4, 3, __KER_CLOCK_PERIOD, (unsigned)"ClockPeriod", 0, 1},
		{2, 1, __KER_CLOCK_ID, (unsigned)"ClockId", 0, 1},
		{2, 1, __KER_TIMER_CREATE, (unsigned)"TimerCreate", 0, 1},
		{2, 1, __KER_TIMER_DESTROY, (unsigned)"TimerDestroy", 0, 1},
		{6, 5, __KER_TIMER_SETTIME, (unsigned)"TimerSettime", 0, 1},
		{20, 18, __KER_TIMER_INFO, (unsigned)"TimerInfo", 0, 1},
		{6, 5, __KER_TIMER_ALARM, (unsigned)"TimerAlarm", 0, 1},
		{4, 3, __KER_TIMER_TIMEOUT, (unsigned)"TimerTimeout", 0, 1},
		{2, 1, __KER_SYNC_CREATE, (unsigned)"SyncTypeCreate", 0, 1},
		{2, 1, __KER_SYNC_DESTROY, (unsigned)"SyncDestroy", 0, 1},
		{2, 1, __KER_SYNC_MUTEX_LOCK, (unsigned)"SyncMutexLock", 0, 1},
		{2, 1, __KER_SYNC_MUTEX_UNLOCK, (unsigned)"SyncMutexUnlock", 0, 1},
		{2, 1, __KER_SYNC_CONDVAR_WAIT, (unsigned)"SyncCondvarWait", 0, 1},
		{2, 1, __KER_SYNC_CONDVAR_SIGNAL, (unsigned)"SyncCondvarSignal", 0, 1},
		{2, 1, __KER_SYNC_SEM_POST, (unsigned)"SyncSemPost", 0, 1},
		{2, 1, __KER_SYNC_SEM_WAIT, (unsigned)"SyncSemWait", 0, 1},
		{2, 1, __KER_SYNC_CTL, (unsigned)"SyncCtl", 0, 1},
		{2, 1, __KER_SYNC_MUTEX_REVIVE, (unsigned)"SyncMutexRevive", 0, 1},
#ifdef __EXT_QNX
		{12, 10, __KER_SCHED_GET, (unsigned)"SchedGet", 0, 1},
#else
		{10, 4, __KER_SCHED_GET, (unsigned)"SchedGet", 0, 1},
#endif
		{2, 1, __KER_SCHED_SET, (unsigned)"SchedSet", 0, 1},
		{2, 1, __KER_SCHED_YIELD, (unsigned)"SchedYield()", 0, 1},
		{6, 6, __KER_SCHED_INFO, (unsigned)"SchedInfo()", 0, 2},
		{20, 1, __KER_NET_CRED, (unsigned)"NetCred()", 0, 1},
		{2, 1, __KER_NET_VTID, (unsigned)"NetVtid()", 0, 1},
		{2, 1, __KER_NET_UNBLOCK, (unsigned)"NetUnblock()", 0, 1},
		{2, 1, __KER_NET_INFOSCOID, (unsigned)"NetInfoscoid()", 0, 1},
		{2, 1, __KER_NET_SIGNAL_KILL, (unsigned)"NetSignalKill()", 0, 1},
#ifdef __KER_MSG_CURRENT
		{2, 1, __KER_MSG_CURRENT, (unsigned)"MsgCurrent", 0, 1},
#endif
		{0,0,0,0} /*END*/
	};

	path = argv[0];

	/* use this trick to determine if this process was created by
	 * the shell or by this program
	 */
	if (strcmp(argv[0], token[0]) == 0) {
		int rc;
		
		assert(argc > 1);
		
		while ((rc = getopt(argc, argv, "123456789a")) != -1) {
			switch (rc) {
				case '1':
				/* Start logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				delay(10);
				exit(1);
				abort();
				break;
			}
		}
		exit(0);
	}
	/*
	 * Start the tests.
	 */
	/* Get rid of tracelogger if it is running  */
	tlkilled=kill_tl();
	teststart(argv[0]);
	/* Setup a channel for the MsgSend Events */
	chid=ChannelCreate(0);
	assert(chid!=-1);
	/* and create a thread to block on the channel */
	rc=pthread_create(NULL, NULL, msg_thread, NULL);
	assert(rc==EOK);
	cur_event=events[0];
	while (cur_event[0]!=0) {

		/***********************************************************************/
	
		/***********************************************************************/
		/*
		 * Make sure that if we trigger a event, that it gets logged properly
		 * (all the information in the tracelogger is correct)
		 * This tests the information provided in wide mode.
		 */
		snprintf(message, sizeof(message),"%s in wide mode", (char *)cur_event[3]);
	 	testpntbegin(message);
		/* Get rid of any old tracebuffers */
		unlink("/dev/shmem/tracebuffer");
			
		/* We need to start up the tracelogger in daemon mode, 1 itteration.
		 * We will filter out everything other then our kernel calls, then 
		 * start logging. 
		 * We then will make a kernel call, and flush the trace buffer.  This 
		 * should create a VERY minimal trace buffer that will be easily parsed
		 */
		tlpid=start_logger();
		sleep(1);
		/* Set the logger to wide emmiting mode */
		rc=TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE);
		assert(rc!=-1);
		fast_mode=WIDE;
		/* Add the given kercall event in the Kernel call class back in */
		rc=TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_KERCALLEXIT,cur_event[2]);
		assert(rc!=-1);
		/* Filter out all pids but our pid for kernel calls. 
		 * This filter can not be added for the THREAD_DESTROYALL kernel call
		 * as it is triggered by proc, not by us directly
		 */
		if (cur_event[2]!=__KER_THREAD_DESTROYALL) {
			rc=TraceEvent(_NTO_TRACE_SETCLASSPID, _NTO_TRACE_KERCALLEXIT, getpid());
			assert(rc!=-1);
		}

		/* then trigger an event.  Logging is started inside the trigger_kercall_event
		 * function immediatly before the kernel call is triggered.
		 */
		
		trigger_kercall_event(cur_event);
		
		/* flush the trace buffer */
		rc=TraceEvent(_NTO_TRACE_FLUSHBUFFER);	
		assert(rc!=-1);
	 
		/* And wait for the tracelogger to exit */
		rc=waitpid(tlpid, &status, 0);
		assert(tlpid==rc);
	
		/* Now, setup the traceparser lib to pull out the kernel call events, 
		 * and make sure our event shows up 
		 */
		tp_state=traceparser_init(NULL);
		assert(tp_state!=NULL);
		traceparser_cs(tp_state, NULL, parse_cb, _NTO_TRACE_KERCALLEXIT, cur_event[2]);
	
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
			testpntfail("Wrong parameters in the event");
		else if (correct_values==1)
			testpntpass("Got the correct values");
		else 
			testpntfail("This should not happen");

		traceparser_destroy(&tp_state);
	 	testpntend();
		/***********************************************************************/
	
	
		/***********************************************************************/
		/*
		 * Make sure that if we trigger a event, that it gets logged properly
		 * (all the information in the tracelogger is correct)
		 * This tests the information provided in fast mode.
		 */
		snprintf(message, sizeof(message),"%s in fast mode", (char *)cur_event[3]);
	 	testpntbegin(message);

		/* Get rid of any old tracebuffers */
		unlink("/dev/shmem/tracebuffer");
			
		/* We need to start up the tracelogger in daemon mode, 1 itteration.
		 * we will filter out everything other then our kernel calls, then 
		 * start logging. 
		 * We then will make a kernel call, and flush the trace buffer.  This 
		 * should create a VERY minimal trace buffer that will be easily parsed
		 */
		tlpid=start_logger();
		sleep(1);
		/* Set the logger to fast emmiting mode */
		rc=TraceEvent(_NTO_TRACE_SETALLCLASSESFAST);
		assert(rc!=-1);
		fast_mode=FAST;
		errno=0;
		/* Add the  event in the Kernel call class back in */
		rc=TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_KERCALLEXIT,cur_event[2]);
		assert(rc!=-1);
		/* Filter out all pids but our pid for kernel calls. 
		 * This filter can not be added for the THREAD_DESTROYALL kernel call
		 * as it is triggered by proc, not by us directly
		 */
		if (cur_event[2]!=__KER_THREAD_DESTROYALL) {
			rc=TraceEvent(_NTO_TRACE_SETCLASSPID, _NTO_TRACE_KERCALLEXIT, getpid());
			assert(rc!=-1);
		}
		

		trigger_kercall_event(cur_event);
		
		/* flush the trace buffer */
		rc=TraceEvent(_NTO_TRACE_FLUSHBUFFER);	
		assert(rc!=-1);
		rc=waitpid(tlpid, &status, 0);
		assert(tlpid==rc);
	
		/* Now, setup the traceparser lib to pull out the kernel call events, 
		 * and make sure our event shows up 
		 */
		tp_state=traceparser_init(NULL);
		assert(tp_state!=NULL);
		traceparser_cs(tp_state, NULL, parse_cb, _NTO_TRACE_KERCALLEXIT, cur_event[2]);
	
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
			testpntfail("Wrong parameters in the event");
		else if (correct_values==1)
			testpntpass("Got the correct values");
		else 
			testpntfail("This should not happen");
		traceparser_destroy(&tp_state);
	 	testpntend();
	
		/***********************************************************************/

		/* Go to the next event to test */
		cur_event+=6;
	}
	/* If the tracelogger was running when we started, we should restart it again */
	if (tlkilled==1) 
		system("reopen /dev/null ; tracelogger -n 0 -f /dev/null &");
	teststop(argv[0]);
	return 0;
}

/****************************************************************************/


/****************************************************************************
*
*						Subroutine trigger_kercall_event
*
*	Purpose: 	This function is used to try to trigger the given event
*				This will trigger only kernel call events and should
*				generate both an entry and exit event
*				
*				This function will also start the logging, as some types
*				of events may need some setup, and that setup may need
*				to make some kernel calls
*
*	Parameters:	
*				event_p	 - This is the pointer to the test array entry
*						   for the event we want to trigger
*
*	Returns: 	EOK on success, any other value on failure
*
*****************************************************************************/
int trigger_kercall_event( unsigned * event_p)
{
	int rc,rcvid,child;
	static int coid=0;
	struct _msg_info myinfo;
	struct sigevent myevent;
	struct sigaction myaction;
	struct _client_info myclientinfo;
	sigset_t myset;
	uint64_t timeout,timeout2,oldtime;
	struct _server_info myserverinfo;
	unsigned a_value;
	struct inheritance	myinherit;
	pthread_t mythread;
	struct _clockadjust myadjustment;
	struct _clockperiod myclockperiod;
	struct _itimer myitime,myitime2;
	struct _timer_info mytimerinfo;
	sync_t mysync,mysync2;
	sync_attr_t mysyncattr;
	struct sched_param myschedparam;
	struct _sched_info myschedinfo;
	struct _vtid_info myvtidinfo;
	siginfo_t mysiginfo;
	struct {
		unsigned nd;
		pid_t    pid;
		int32_t  tid;
		int32_t  signo;
		int32_t  code;
		int32_t  value;
	} signal_info;
	struct _cred_info mycredinfo;


	switch(event_p[2]) {
		case __KER_NOP:
			memset(global_vals, 0, sizeof(global_vals));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			__kernop();
			return(EOK);
			break;
		case __KER_TRACE_EVENT:
			memset(global_vals, 0, sizeof(global_vals));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=TraceEvent(_NTO_TRACE_SETEVENTTID, _NTO_TRACE_KERCALL, 	__KER_SIGNAL_KILL,10,10);
			return(EOK);
			break;
		case __KER_SYS_CPUPAGE_GET:
			memset(global_vals, 0, sizeof(global_vals));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=__SysCpupageGet(1);
			break;
		case __KER_MSG_SENDV:
			memset(global_vals, 0, sizeof(global_vals));
			memset(reply_buf,0, sizeof(reply_buf));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&send,SEND, sizeof(SEND));
			SETIOV(&reply, reply_buf, 4);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgSendv(coid, &send, 1, &reply, 1);
			global_vals[1]= ((unsigned *)reply_buf)[0];
			global_vals[2]= ((unsigned *)reply_buf)[1];
			global_vals[3]= ((unsigned *)reply_buf)[2];
			break;
		case __KER_MSG_SENDVNC:
			memset(global_vals, 0, sizeof(global_vals));
			memset(reply_buf,0, sizeof(reply_buf));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&send,SEND, sizeof(SEND));
			SETIOV(&reply, reply_buf, 4);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgSendvnc(coid, &send, 1, &reply, 1);
			global_vals[1]= ((unsigned *)reply_buf)[0];
			global_vals[2]= ((unsigned *)reply_buf)[1];
			global_vals[3]= ((unsigned *)reply_buf)[2];
			break;
		case __KER_MSG_ERROR:
			memset(global_vals, 0, sizeof(global_vals));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			rcvid=MsgReceive(chid, reply_buf,sizeof(reply_buf), NULL);
			global_vals[0]=rcvid;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgError(rcvid, 10);
			break;
		case __KER_MSG_RECEIVEV:
			memset(global_vals, 0, sizeof(global_vals));
			memset(reply_buf,0, sizeof(reply_buf));
			memset(&myinfo, 0, sizeof(myinfo));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			else {
				ConnectDetach(coid);
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			}
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgReceivev(chid, &reply, 1, &myinfo);
			global_vals[1]= ((unsigned *)reply_buf)[0];
			global_vals[2]= ((unsigned *)reply_buf)[1];
			global_vals[3]= ((unsigned *)reply_buf)[2];
			global_vals[4]= myinfo.nd;
			global_vals[5]= myinfo.srcnd;
			global_vals[6]= myinfo.pid;
			global_vals[7]= myinfo.tid;
			global_vals[8]= myinfo.chid;
			global_vals[9]= myinfo.scoid;
			global_vals[10]= myinfo.coid;
			global_vals[11]= myinfo.msglen;
			global_vals[12]= myinfo.srcmsglen;
			global_vals[13]= myinfo.dstmsglen;
			global_vals[14]= myinfo.priority;
			global_vals[15]= myinfo.flags;
			global_vals[16]= myinfo.reserved;
			
			MsgReply(global_vals[0], EOK,  "bye", 4);
			break;
		case __KER_MSG_REPLYV:
			memset(global_vals, 0, sizeof(global_vals));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&send,SEND, sizeof(SEND));
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			rcvid=MsgReceivev(chid, &reply, 1,NULL);
			global_vals[0]=rcvid;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgReplyv(rcvid, EOK,  &send, 1);
			break;
		case __KER_MSG_READV:
			memset(global_vals, 0, sizeof(global_vals));
			memset(reply_buf,0, sizeof(reply_buf));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			rcvid=MsgReceivev(chid, &reply, 1,NULL);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgReadv(rcvid,&reply, 1,0);
			global_vals[1]= ((unsigned *)reply_buf)[0];
			global_vals[2]= ((unsigned *)reply_buf)[1];
			global_vals[3]= ((unsigned *)reply_buf)[2];
			MsgReply(rcvid, EOK,  "bye", 4);
			break;
		case __KER_MSG_WRITEV:
			memset(global_vals, 0, sizeof(global_vals));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&send,SEND, sizeof(SEND));
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			rcvid=MsgReceivev(chid, &reply, 1,NULL);
			global_vals[0]=rcvid;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgWritev(rcvid, &send, 1, 0);
			MsgReplyv(rcvid, EOK,  &send, 1);
			break;

		case __KER_MSG_INFO:
			memset(global_vals, 0, sizeof(global_vals));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			rcvid=MsgReceivev(chid, &reply, 1,NULL);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgInfo(rcvid, &myinfo);
			global_vals[1]= myinfo.nd;
			global_vals[2]= myinfo.srcnd;
			global_vals[3]= myinfo.pid;
			global_vals[4]= myinfo.tid;
			global_vals[5]= myinfo.chid;
			global_vals[6]= myinfo.scoid;
			global_vals[7]= myinfo.coid;
			global_vals[8]= myinfo.msglen;
			global_vals[9]= myinfo.srcmsglen;
			global_vals[10]= myinfo.dstmsglen;
			global_vals[11]= myinfo.priority;
			global_vals[12]= myinfo.flags;
			global_vals[13]= myinfo.reserved;
			MsgReply(rcvid, EOK,  "OK", 3);
			break;
		case __KER_MSG_SEND_PULSE:
			memset(global_vals, 0, sizeof(global_vals));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgSendPulse(coid, 10,11,12);
			return(EOK);
			break;
		case __KER_MSG_DELIVER_EVENT:
			memset(global_vals, 0, sizeof(global_vals));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			rcvid=MsgReceivev(chid, &reply, 1,NULL);
			MsgReplyv(rcvid, EOK,  &send, 1);
			
			/*Setup the event to send */
			myevent.sigev_notify=SIGEV_NONE;
			myevent.sigev_notify_function=NULL;
			myevent.sigev_notify_attributes=NULL;
			myevent.sigev_value.sival_int=10;
		
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgDeliverEvent(rcvid, &myevent);
			global_vals[1]=(unsigned)&myevent;
			break;
		case __KER_MSG_KEYDATA:
			memset(global_vals, 0, sizeof(global_vals));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			rcvid=MsgReceivev(chid, &reply, 1,NULL);
			MsgReplyv(rcvid, EOK,  &send, 1);
			
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgKeyData(rcvid, _NTO_KEYDATA_CALCULATE,101,&a_value,&reply, 1);
			global_vals[1]=a_value;
			break;
		case __KER_MSG_READIOV:
			memset(global_vals, 0, sizeof(global_vals));
			memset(reply_buf, 0, sizeof(reply_buf));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			rcvid=MsgReceivev(chid, &reply, 1,NULL);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgReadiov(rcvid,&reply, 1,0,0);
			global_vals[1]= ((unsigned *)reply_buf)[0];
			global_vals[2]= ((unsigned *)reply_buf)[1];
			global_vals[3]= ((unsigned *)reply_buf)[2];
			
			MsgReply(rcvid, EOK,  "bye", 4);
			break;
		case __KER_MSG_RECEIVEPULSEV:
			memset(global_vals, 0, sizeof(global_vals));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, sizeof(reply_buf));
			MsgSend(coid, "PSEND", 6, reply_buf, sizeof(reply_buf));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			errno=0;
			global_vals[0]=MsgReceivePulsev(chid, &reply, 1,&myinfo);
			global_vals[1]= ((unsigned *)reply_buf)[0];
			global_vals[2]= ((unsigned *)reply_buf)[1];
			global_vals[3]= ((unsigned *)reply_buf)[2];
			global_vals[4]= myinfo.nd;
			global_vals[5]= myinfo.srcnd;
			global_vals[6]= myinfo.pid;
			global_vals[7]= myinfo.tid;
			global_vals[8]= myinfo.chid;
			global_vals[9]= myinfo.scoid;
			global_vals[10]= myinfo.coid;
			global_vals[11]= myinfo.msglen;
			global_vals[12]= myinfo.srcmsglen;
			global_vals[13]= myinfo.dstmsglen;
			global_vals[14]= myinfo.priority;
			global_vals[15]= myinfo.flags;
			global_vals[16]= myinfo.reserved;
			
			break;
		case __KER_MSG_VERIFY_EVENT:
			memset(global_vals, 0, sizeof(global_vals));
			/*Setup the event to check */
			myevent.sigev_notify=SIGEV_NONE;
			myevent.sigev_notify_function=NULL;
			myevent.sigev_notify_attributes=NULL;
			myevent.sigev_value.sival_int=10;
		
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgVerifyEvent(0, &myevent);
			break;
		case __KER_SIGNAL_KILL:
			memset(global_vals, 0, sizeof(global_vals));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SignalKill(0,getpid(), pthread_self(), SIGUSR1,10,10);
			break;
		case __KER_SIGNAL_ACTION:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myaction, 0, sizeof(myaction));
			myaction.sa_handler=sig_hand;

			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SignalAction(getpid(), __signalstub, SIGUSR2, NULL, &myaction);
			global_vals[1]=(unsigned)myaction.sa_handler;
			global_vals[2]=myaction.sa_flags;
			global_vals[3]=((unsigned *)&myaction.sa_mask)[0];
			global_vals[4]=((unsigned *)&myaction.sa_mask)[1];

			break;
		case __KER_SIGNAL_PROCMASK:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myset, 0, sizeof(myset));

			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SignalProcmask(getpid(), pthread_self(),SIG_PENDING,NULL, &myset);
			global_vals[1]= ((unsigned *)(&myset))[0];
			global_vals[2]= ((unsigned *)(&myset))[1];

			break;
		case __KER_SIGNAL_SUSPEND:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myevent, 0, sizeof(myevent));
			myevent.sigev_notify = SIGEV_UNBLOCK;
			timeout = 2*NS_IN_SEC;

			sigemptyset(&myset);
			sigaddset(&myset, SIGHUP);
			sigaddset(&myset, SIGBUS);
			sigaddset(&myset, SIGSEGV);
			sigaddset(&myset, SIGXCPU);
			sigaddset(&myset, SIGRTMIN);
	
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			TimerTimeout( CLOCK_REALTIME, _NTO_TIMEOUT_SIGSUSPEND,&myevent, &timeout, NULL );
			global_vals[0]= SignalSuspend(&myset);
			global_vals[1]= NULL;
			
			return(EOK);
			break;

		case __KER_SIGNAL_WAITINFO:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myevent, 0, sizeof(myevent));
			memset(&mysiginfo, 0, sizeof(mysiginfo));
			myevent.sigev_notify = SIGEV_UNBLOCK;
			timeout = 1*NS_IN_SEC;

			sigemptyset(&myset);
			sigaddset(&myset, SIGHUP);
			sigaddset(&myset, SIGBUS);
			sigaddset(&myset, SIGSEGV);
			sigaddset(&myset, SIGXCPU);
			sigaddset(&myset, SIGRTMIN);
			sigaddset(&myset, SIGRTMAX);
	
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			TimerTimeout( CLOCK_REALTIME, _NTO_TIMEOUT_SIGWAITINFO,&myevent, &timeout, NULL );
			global_vals[0]= SignalWaitinfo(&myset,&mysiginfo);
			global_vals[1]= mysiginfo.si_signo;
			global_vals[2]= mysiginfo.si_code;
			global_vals[3]= mysiginfo.si_errno;
			global_vals[4]= ((unsigned *)&mysiginfo)[3];
			global_vals[5]= ((unsigned *)&mysiginfo)[4];
			global_vals[6]= ((unsigned *)&mysiginfo)[5];
			global_vals[7]= ((unsigned *)&mysiginfo)[6];
			global_vals[8]= ((unsigned *)&mysiginfo)[7];
			global_vals[9]= ((unsigned *)&mysiginfo)[8];
			global_vals[10]= ((unsigned *)&mysiginfo)[9];
			
			return(EOK);
			break;
		case __KER_CHANNEL_CREATE:
			memset(global_vals, 0, sizeof(global_vals));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ChannelCreate(_NTO_CHF_FIXED_PRIORITY);
			ChannelDestroy(global_vals[0]);
			return(EOK);
			break;

		case __KER_CHANNEL_DESTROY:
			memset(global_vals, 0, sizeof(global_vals));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			rc=ChannelCreate(_NTO_CHF_FIXED_PRIORITY);
			delay(10);
			global_vals[0]=rc;
			assert(rc!=-1);
			global_vals[0]=ChannelDestroy(rc);
			return(EOK);
			break;
		case __KER_CONNECT_ATTACH:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ConnectAttach(0,getpid(), 1, _NTO_SIDE_CHANNEL, 0);
			ConnectDetach(global_vals[0]);
			break;
		case __KER_CONNECT_DETACH:
			memset(global_vals, 0, sizeof(global_vals));
			a_value=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ConnectDetach(a_value);

			break;
		case __KER_CONNECT_SERVER_INFO:
			memset(global_vals, 0, sizeof(global_vals));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ConnectServerInfo(getpid(), 1,&myserverinfo);
			global_vals[1]=myserverinfo.nd;
			global_vals[2]=myserverinfo.srcnd;
			global_vals[3]=myserverinfo.pid;
			global_vals[4]=myserverinfo.tid;
			global_vals[5]=myserverinfo.chid;
			global_vals[6]=myserverinfo.scoid;
			global_vals[7]=myserverinfo.coid;
			global_vals[8]=myserverinfo.msglen;
			global_vals[9]=myserverinfo.srcmsglen;
			global_vals[10]=myserverinfo.dstmsglen;
			global_vals[11]=myserverinfo.priority;
			global_vals[12]=myserverinfo.flags;
			global_vals[13]=myserverinfo.reserved;
			return(EOK);
			break;
		case __KER_CONNECT_CLIENT_INFO:
			memset(global_vals, 0, sizeof(global_vals));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			ConnectServerInfo(getpid(), coid,&myserverinfo);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ConnectClientInfo(myserverinfo.scoid, &myclientinfo, 10);
			global_vals[1]=myclientinfo.nd;
			global_vals[2]=myclientinfo.pid;
			global_vals[3]=myclientinfo.sid;
			global_vals[4]=myclientinfo.flags;
			global_vals[5]=myclientinfo.cred.ruid;
			global_vals[6]=myclientinfo.cred.euid;
			global_vals[7]=myclientinfo.cred.suid;
			global_vals[8]=myclientinfo.cred.rgid;
			global_vals[9]=myclientinfo.cred.egid;
			global_vals[10]=myclientinfo.cred.sgid;
			global_vals[11]=myclientinfo.cred.ngroups;
			global_vals[12]=myclientinfo.cred.grouplist[0];
			global_vals[13]=myclientinfo.cred.grouplist[1];
			global_vals[14]=myclientinfo.cred.grouplist[2];
			global_vals[15]=myclientinfo.cred.grouplist[3];
			global_vals[16]=myclientinfo.cred.grouplist[4];
			global_vals[17]=myclientinfo.cred.grouplist[5];
			global_vals[18]=myclientinfo.cred.grouplist[6];
			global_vals[19]=myclientinfo.cred.grouplist[7];
			return(EOK);
			break;
		case __KER_CONNECT_FLAGS:
			memset(global_vals, 0, sizeof(global_vals));
			a_value=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ConnectFlags(getpid(), a_value,0xF0F0F0F0, 0x0F0F0F0F);
			ConnectDetach(global_vals[0]);
			break;
		case __KER_THREAD_CREATE:
			memset(global_vals, 0, sizeof(global_vals));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			pthread_create(&mythread, NULL, nothing_thread, NULL);
			global_vals[0]=mythread;
			break;
		case __KER_THREAD_DESTROY:
			memset(global_vals, 0, sizeof(global_vals));
			/* create a thread to destroy */
			rc=pthread_create(&mythread, NULL, nothing_thread, NULL);
			assert(rc==EOK);
			global_vals[0]=mythread;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ThreadDestroy(mythread, 10, (void *) 10);
			break;
		case __KER_THREAD_DESTROYALL:
			memset(global_vals, 0, sizeof(global_vals));
			/* To trigger this event we will have to kill an entire
			 * process, so we will have to spawn a child to trigger
			 * a thread destroyall, and the parent will just wait
			 * for the child to exit
			 */
			 /* create child to run the test itself */
			token[1] = "-1";
			memset(&myinherit,0,sizeof(myinherit));
			
			child = spawnp(path, 0, NULL, &myinherit, token, NULL);
			assert(child!=-1);
			rc=waitpid(child, &a_value, 0);
			assert(rc==child);
			global_vals[0]=WEXITSTATUS(a_value);
			break;
		case __KER_THREAD_DETACH:
			memset(global_vals, 0, sizeof(global_vals));
			/* create a thread to destroy */
			rc=pthread_create(&mythread, NULL, nothing_thread, NULL);
			assert(rc==EOK);
			global_vals[0]=mythread;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ThreadDetach(mythread);
			break;
		case __KER_THREAD_JOIN:
			memset(global_vals, 0, sizeof(global_vals));
			/* create a thread to destroy */
			rc=pthread_create(&mythread, NULL, nothing_thread, NULL);
			assert(rc==EOK);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ThreadJoin(mythread, (void **)&(global_vals[1]));
			break;
		case __KER_THREAD_CANCEL:
			memset(global_vals, 0, sizeof(global_vals));
			/* create a thread to destroy */
			rc=pthread_create(&mythread, NULL, nothing_thread, NULL);
			assert(rc==EOK);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ThreadCancel(mythread,can_stub);
			break;
		case __KER_THREAD_CTL:
			memset(global_vals, 0, sizeof(global_vals));
			/* create a thread to destroy */
			rc=pthread_create(&mythread, NULL, nothing_thread, NULL);
			assert(rc==EOK);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ThreadCtl(_NTO_TCTL_RUNMASK,(void *) 0xF0F0F0FF);
			break;
		case __KER_CLOCK_TIME:
			memset(global_vals, 0, sizeof(global_vals));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ClockTime(CLOCK_REALTIME, NULL, &oldtime);
			global_vals[1]=(oldtime/NS_IN_SEC);
			global_vals[2]=(oldtime-((oldtime/NS_IN_SEC)*NS_IN_SEC));
			break;
		case __KER_CLOCK_ADJUST:
			memset(global_vals, 0, sizeof(global_vals));
			/* This will cause the time to go ahead a total of 	
			 * 25 nano seconds, which we should never really notice..
			 */
			myadjustment.tick_nsec_inc=5;
			myadjustment.tick_count=5;
	
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ClockAdjust(CLOCK_REALTIME, &myadjustment, &myadjustment);
			global_vals[1]=myadjustment.tick_count;
			global_vals[2]=myadjustment.tick_nsec_inc;

			break;
		case __KER_CLOCK_PERIOD:
			memset(global_vals, 0, sizeof(global_vals));
			ClockPeriod(CLOCK_REALTIME, NULL, &myclockperiod,0);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ClockPeriod(CLOCK_REALTIME, &myclockperiod, &myclockperiod,0);
			global_vals[1]=myclockperiod.nsec;
			global_vals[2]=myclockperiod.fract;
			break;
		case __KER_CLOCK_ID:
			memset(global_vals, 0, sizeof(global_vals));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=ClockId(getpid(), pthread_self());
			break;
		case __KER_TIMER_CREATE:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myevent, 0,sizeof(myevent));
			SIGEV_SIGNAL_CODE_INIT( &myevent, SIGUSR1, 0xababcdcd, SI_MINAVAIL+1 );
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=TimerCreate(CLOCK_REALTIME, &myevent);
			TimerDestroy(global_vals[0]);
			break;
		case __KER_TIMER_DESTROY:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myevent, 0,sizeof(myevent));
			SIGEV_SIGNAL_CODE_INIT( &myevent,SIGUSR1,0, SI_MINAVAIL+1 );
			rc=TimerCreate(CLOCK_REALTIME, &myevent);
			
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=TimerDestroy(rc);
			break;
		case __KER_TIMER_SETTIME:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myitime2,0, sizeof(myitime2));	
			memset(&myevent, 0,sizeof(myevent));
			SIGEV_SIGNAL_CODE_INIT( &myevent,SIGUSR1,0, SI_MINAVAIL+1 );
			global_vals[0]=TimerCreate(CLOCK_REALTIME, &myevent);
			myitime.nsec=NS_IN_SEC;
			myitime.nsec*= 5;
			myitime.nsec+=6;

			myitime.interval_nsec=NS_IN_SEC;
			myitime.interval_nsec*=17;
			myitime.interval_nsec+=10;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=TimerSettime(global_vals[0],0, &myitime, &myitime);
			global_vals[1]=(myitime.nsec/NS_IN_SEC);
			global_vals[2]=(myitime.nsec-((myitime.nsec/NS_IN_SEC)*NS_IN_SEC));
			global_vals[3]=(myitime.interval_nsec/NS_IN_SEC);
			global_vals[4]=(myitime.interval_nsec-((myitime.interval_nsec/NS_IN_SEC)*NS_IN_SEC));

			TimerDestroy(global_vals[0]);
			break;
		case __KER_TIMER_INFO:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myevent, 0,sizeof(myevent));
			SIGEV_SIGNAL_CODE_INIT( &myevent,SIGUSR1,0, SI_MINAVAIL+1 );
			a_value=TimerCreate(CLOCK_REALTIME, &myevent);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=TimerInfo(getpid(), a_value, 0, &mytimerinfo);
			global_vals[1]=(mytimerinfo.itime.nsec/NS_IN_SEC);
			global_vals[2]=(mytimerinfo.itime.nsec-((mytimerinfo.itime.nsec/NS_IN_SEC)*NS_IN_SEC));
			global_vals[3]=(mytimerinfo.itime.interval_nsec/NS_IN_SEC);
			global_vals[4]=(mytimerinfo.itime.interval_nsec-((mytimerinfo.itime.interval_nsec/NS_IN_SEC)*NS_IN_SEC));
			global_vals[5]=(mytimerinfo.otime.nsec/NS_IN_SEC);
			global_vals[6]=(mytimerinfo.otime.nsec-((mytimerinfo.otime.nsec/NS_IN_SEC)*NS_IN_SEC));
			global_vals[7]=(mytimerinfo.otime.interval_nsec/NS_IN_SEC);
			global_vals[8]=(mytimerinfo.otime.interval_nsec-((mytimerinfo.otime.interval_nsec/NS_IN_SEC)*NS_IN_SEC));
			global_vals[9]=mytimerinfo.flags;
			global_vals[10]=mytimerinfo.tid;
			global_vals[11]=mytimerinfo.notify;
			global_vals[12]=mytimerinfo.clockid;
			global_vals[13]=mytimerinfo.overruns;
			global_vals[14]=mytimerinfo.event.sigev_notify;
			global_vals[15]=(unsigned)mytimerinfo.event.sigev_notify_function;
			global_vals[16]=(unsigned)mytimerinfo.event.sigev_value.sival_int;
			global_vals[17]=(unsigned)mytimerinfo.event.sigev_notify_attributes;
			TimerDestroy(a_value);
			break;
		case __KER_TIMER_ALARM:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myevent, 0,sizeof(myevent));
			SIGEV_SIGNAL_CODE_INIT( &myevent,SIGUSR1,0, SI_MINAVAIL+1 );
			a_value=TimerCreate(CLOCK_REALTIME, &myevent);
			myitime.nsec=NS_IN_SEC;
			myitime.nsec*= 6;
			myitime.nsec+=7;

			myitime.interval_nsec=NS_IN_SEC;
			myitime.interval_nsec*=8;
			myitime.interval_nsec+=9;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=TimerAlarm(a_value,&myitime, &myitime);
			global_vals[1]=(myitime.nsec/NS_IN_SEC);
			global_vals[2]=(myitime.nsec-((myitime.nsec/NS_IN_SEC)*NS_IN_SEC));
			global_vals[3]=(myitime.interval_nsec/NS_IN_SEC);
			global_vals[4]=(myitime.interval_nsec-((myitime.interval_nsec/NS_IN_SEC)*NS_IN_SEC));
			myitime.nsec=0;
			myitime.interval_nsec=0;
			TimerAlarm(a_value,&myitime, &myitime);
			TimerDestroy(a_value);
			break;
		case __KER_TIMER_TIMEOUT:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myevent, 0,sizeof(myevent));
			SIGEV_SIGNAL_CODE_INIT( &myevent, SIGUSR1, 0xababcdcd, SI_MINAVAIL+1 );
			timeout=NS_IN_SEC;
			timeout*= 10;
			timeout+=10;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			timeout2=0;
			global_vals[0]=TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_JOIN, &myevent, &timeout, &timeout2);
			global_vals[1]=(timeout2/NS_IN_SEC);
			global_vals[2]=(timeout2-((timeout2/NS_IN_SEC)*NS_IN_SEC));
			break;
		case __KER_SYNC_CREATE:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&mysync, 0, sizeof(mysync));
			memset(&mysyncattr, 0, sizeof(mysyncattr));
			mysync.count=10;
			mysync.owner=0;
			mysyncattr.protocol=PTHREAD_PRIO_PROTECT;
			mysyncattr.flags=0;
			mysyncattr.prioceiling=15;
			mysyncattr.clockid=CLOCK_REALTIME;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SyncTypeCreate(_NTO_SYNC_SEM, &mysync, &mysyncattr);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_DESTROY:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&mysync, 0, sizeof(mysync));
			mysync.count=0;
			mysync.owner=0;
			SyncTypeCreate(_NTO_SYNC_SEM, &mysync, NULL);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SyncDestroy(&mysync);
			break;
		case __KER_SYNC_MUTEX_LOCK:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_MUTEX_FREE , &mysync, NULL);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SyncMutexLock(&mysync);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_MUTEX_UNLOCK:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_MUTEX_FREE , &mysync, NULL);
			SyncMutexLock(&mysync);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SyncMutexUnlock(&mysync);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_CONDVAR_WAIT:
			memset(global_vals, 0, sizeof(global_vals));
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "CONDSIGNAL", 11, reply_buf, sizeof(reply_buf));

			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_COND  , &mysync, NULL);
			SyncTypeCreate(_NTO_SYNC_MUTEX_FREE , &mysync2, NULL);
			SyncMutexLock(&mysync2);
			global_vals[0]=(unsigned)&mysync;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SyncCondvarWait(&mysync,&mysync2);
			SyncDestroy(&mysync);
			SyncDestroy(&mysync2);
			break;
		case __KER_SYNC_CONDVAR_SIGNAL:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_COND  , &mysync, NULL);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SyncCondvarSignal(&mysync, 10);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_SEM_POST:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_SEM, &mysync, NULL);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SyncSemPost(&mysync);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_SEM_WAIT:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_SEM, &mysync, NULL);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SyncSemWait(&mysync, 10);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_CTL:
			memset(&mysync, 0, sizeof(mysync));
			memset(global_vals, 0, sizeof(global_vals));
			SyncTypeCreate(_NTO_SYNC_MUTEX_FREE , &mysync, NULL);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SyncCtl(_NTO_SCTL_GETPRIOCEILING,&mysync, &timeout);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_MUTEX_REVIVE:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_MUTEX_FREE , &mysync, NULL);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SyncMutexRevive(&mysync);
			SyncDestroy(&mysync);
			break;
		case __KER_SCHED_GET:
			memset(global_vals, 0, sizeof(global_vals));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SchedGet(getpid(), pthread_self(), &myschedparam);
			global_vals[1]=myschedparam.sched_priority;
			global_vals[2]=myschedparam.sched_curpriority;
			#ifdef __EXT_QNX
			global_vals[3]=myschedparam.sched_ss_low_priority;
			global_vals[4]=myschedparam.sched_ss_max_repl;
			global_vals[5]=myschedparam.sched_ss_repl_period.tv_sec;
			global_vals[6]=myschedparam.sched_ss_repl_period.tv_nsec;
			global_vals[7]=myschedparam.sched_ss_init_budget.tv_sec;
			global_vals[8]=myschedparam.sched_ss_init_budget.tv_nsec;
			global_vals[9]=myschedparam.__ss_un.reserved[6];
			global_vals[10]=myschedparam.__ss_un.reserved[7];
			#else
			global_vals[3]=myschedparam.spare[0];
			global_vals[4]=myschedparam.spare[1];
			global_vals[5]=myschedparam.spare[2];
			global_vals[6]=myschedparam.spare[3];
			global_vals[7]=myschedparam.spare[4];
			global_vals[8]=myschedparam.spare[5];
			#endif

			return(EOK);
			break;
		case __KER_SCHED_SET:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myschedparam, 0, sizeof(myschedparam));
			myschedparam.sched_priority=10;
			myschedparam.sched_curpriority=12;
#ifdef __EXT_QNX
			myschedparam.sched_ss_low_priority=10;
			myschedparam.sched_ss_max_repl=10;
			myschedparam.sched_ss_repl_period.tv_sec=5;
			myschedparam.sched_ss_repl_period.tv_nsec=5;
			myschedparam.sched_ss_init_budget.tv_sec=10;
			myschedparam.sched_ss_init_budget.tv_nsec=10;
#endif
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SchedSet(getpid(), pthread_self(), SCHED_FIFO, &myschedparam);
			return(EOK);
			break;
		case __KER_SCHED_YIELD:
			memset(global_vals, 0, sizeof(global_vals));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SchedYield();
			return(EOK);
			break;
		case __KER_SCHED_INFO:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myschedinfo, 0, sizeof(myschedinfo));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=SchedInfo(getpid(), SCHED_FIFO, &myschedinfo);
			global_vals[1]=myschedinfo.priority_min;
			global_vals[2]=myschedinfo.priority_max;
			global_vals[3]=(myschedinfo.interval/NS_IN_SEC);
			global_vals[4]=(myschedinfo.interval-((myschedinfo.interval/NS_IN_SEC)*NS_IN_SEC));
			global_vals[5]=myschedinfo.priority_priv;
			return(EOK);
			break;
		case __KER_NET_CRED:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myclientinfo, 0, sizeof(myclientinfo));
			a_value=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(a_value!=-1);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=NetCred(a_value, &myclientinfo);
			global_vals[1]=myclientinfo.nd;
			global_vals[2]=myclientinfo.pid;
			global_vals[3]=myclientinfo.sid;
			global_vals[4]=myclientinfo.flags;
			global_vals[5]=myclientinfo.cred.ruid;
			global_vals[6]=myclientinfo.cred.euid;
			global_vals[7]=myclientinfo.cred.suid;
			global_vals[8]=myclientinfo.cred.rgid;
			global_vals[9]=myclientinfo.cred.egid;
			global_vals[10]=myclientinfo.cred.sgid;
			global_vals[11]=myclientinfo.cred.ngroups;
			global_vals[12]=myclientinfo.cred.grouplist[0];
			global_vals[13]=myclientinfo.cred.grouplist[1];
			global_vals[14]=myclientinfo.cred.grouplist[2];
			global_vals[15]=myclientinfo.cred.grouplist[3];
			global_vals[16]=myclientinfo.cred.grouplist[4];
			global_vals[17]=myclientinfo.cred.grouplist[5];
			global_vals[18]=myclientinfo.cred.grouplist[6];
			global_vals[19]=myclientinfo.cred.grouplist[7];
			ConnectDetach(a_value);
			break;
		case __KER_NET_VTID:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myvtidinfo, 0, sizeof(myvtidinfo));
			myvtidinfo.tid=7;
			myvtidinfo.coid=8;
			myvtidinfo.priority=9;
			myvtidinfo.srcmsglen=10;
			myvtidinfo.keydata=11;
			myvtidinfo.srcnd=12;
			myvtidinfo.dstmsglen=13;
			myvtidinfo.zero=14;
			global_vals[0]=(unsigned)&myvtidinfo;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=NetVtid(101, &myvtidinfo);	
			break;
		case __KER_NET_UNBLOCK:
			memset(global_vals, 0, sizeof(global_vals));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=NetUnblock(101);	
			break;
		case __KER_NET_INFOSCOID:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&myvtidinfo, 0, sizeof(myvtidinfo));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=NetInfoscoid(101,102);	
			break;
		case __KER_NET_SIGNAL_KILL:
			memset(global_vals, 0, sizeof(global_vals));
			memset(&signal_info, 0, sizeof(signal_info));
			memset(&mycredinfo, 0, sizeof(mycredinfo));
			signal_info.nd=0;
			signal_info.pid=getpid();
			signal_info.tid=pthread_self();
			signal_info.signo=SIGUSR1;
			signal_info.value=10;
			signal_info.code=10;
			mycredinfo.ruid=10;
			mycredinfo.euid=10;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=NetSignalKill(&signal_info, &mycredinfo);
			break;
#ifdef __KER_MSG_CURRENT
		case __KER_MSG_CURRENT:
			memset(global_vals, 0, sizeof(global_vals));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			global_vals[0]=MsgCurrent(0);
			break;
#endif

	}
	return(-1);
}
