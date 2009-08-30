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
*	File:		bk3_kercalls.c
*
******************************************************************************
*
*   Contents:	Tests to make sure the instrumentation properly intercepts
*				kerel calls, and their arguments on kernel entry
*
*	Date:		July 18, 2001
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

/* This is a value that can be set in the tests array to tell the parser
 * that the current value should be compaired to the global_val1/2 value
 * instead of the one in the array.  This value should not normally 
 * show up as an actually parameter.
 */
#define GLOBAL_MASK      0xFFFFFF00
#define GLOBAL_VAL		 0xabcdef00
#define USE_GLOBAL_VAL1  0xabcdef00
#define USE_GLOBAL_VAL2  0xabcdef01
#define USE_GLOBAL_VAL3  0xabcdef02
#define USE_GLOBAL_VAL4  0xabcdef03
#define USE_GLOBAL_VAL5  0xabcdef04
#define USE_GLOBAL_VAL6  0xabcdef05

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
/* This is the same as above, but to check the values from the event handler. */
static int correct_values_eh;
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


 
/* This is a generic global unsigned used for passing usefull info 
 * around between the trigger event call, and the parse function.
 * if a parameter is set to the value of USE_GLOBAL_VAL1/2, the parse
 * function will use the value that is currenly in this int as the
 * value that it expects.
 */
unsigned mypid;
unsigned mytid;
unsigned global_val1;
unsigned global_val2;
unsigned global_vals[10];

/* Buffer to send and receive messages in and the iovs we will use */
char reply_buf[100];
iov_t send, reply;


/* These are used to let us spawn copies of this program
 * if needed 
 */
char * path;
char *token[] = { "childproc", NULL, NULL };

/* These two globals are used to pass into the event handlers */
event_data_t my_event_data;
unsigned data_array[30];


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
	int fd,rc, curpid,rval,x;
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
				 while (kill(curpid, 0)==0) {
					x++;
					delay(300);
					if (x>100)
						break;
				}
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
	return(NULL);
}
/****************************************************************************
*
*						Subroutine msg_thread
*
*	Purpose: 	This is a thread that will be used as a target for triggering
*				all of the message related kernel calls. It will sit recieved 
*				blocked on a channel created in main, and will just receive 
*				messages, and if the message indicated it, will turn around 
*				and send a simple message out on the channel to be received.
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
		if ((cur_event[1]==__KER_MSG_SENDV)||(cur_event[1]==__KER_MSG_SENDVNC)) {
			global_vals[0]=rcvid;
		}
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
		}
		delay(100);
	}
	
}
/****************************************************************************
*
*						Subroutine event_handler
*
*	Purpose: 	This is an event handler that will be used to verify that the 
*				instrumented kernel is properly calling event handlers for
*				kernel call entries.
*
*	Parameters:	event_data - this is a pointer to all the event data
*				
*
*	Returns:	This function will always return 1.	
*			
*
*****************************************************************************/
int event_handler(event_data_t * event_data)
{
	int x;

	/* This is to handle the case when we may see multiple events in the 
	 * trace logger.  If we have seen the correct event, then we can just
	 * ignore this one.  This was put in mostly to deal the the TraceEvent
	 * call which will force us to deal with multiple events
	 */
	if (correct_values_eh==1) 
		return(1);

	correct_values_eh=-1;
	/*
	 * Now check that the values that we recieved in the callback are the expected values.
	 */

	if ((_NTO_TRACE_GETEVENT(event_data->header)/128)!=0) 
		correct_values_eh=-2;

	if (_NTO_TRACE_GETEVENT_C(event_data->header)!=_TRACE_KER_CALL_C) {
		correct_values_eh=-3;
	}
	if ((_NTO_TRACE_GETEVENT(event_data->header)%128)!=cur_event[1]) 
		correct_values_eh=-4;


	if (cur_event[1]!=__KER_THREAD_DESTROYALL) {
	  if(event_data->feature[_NTO_TRACE_FIPID]!=mypid)
	    correct_values_eh=-5;
	  if(event_data->feature[_NTO_TRACE_FITID]!=mytid)
	    correct_values_eh=-6;
	  /*		if (data_array[0]!=mypid)  
			correct_values_eh=-5;

		if (data_array[1]!=mytid) 
		correct_values_eh=-6;*/
	}
	if (fast_mode==WIDE) {
		if (!((event_data->el_num==cur_event[0]-4)||(event_data->el_num=cur_event[0]-5))) {
			correct_values_eh=-1;
			return(1);
		}
		for (x=0;x<(cur_event[0]-4);x++) {
			if (event_data->data_array[x]!=(((cur_event[x+5]&GLOBAL_MASK)==GLOBAL_VAL)?global_vals[cur_event[x+5]&(~GLOBAL_MASK)]:cur_event[x+5])) {
				correct_values_eh=-2;
				return(1);
			}
		}
		if (x==(cur_event[0]-4))
			correct_values_eh=1;
			

	
	} else {
		/* if we are not in fast mode we only get the pid and signo */

		if ((event_data->data_array[0]==(((cur_event[cur_event[3]]&GLOBAL_MASK)==GLOBAL_VAL)?global_vals[cur_event[cur_event[3]]&(~GLOBAL_MASK)]:cur_event[cur_event[3]])) && 
			(event_data->data_array[1]==(((cur_event[cur_event[4]]&GLOBAL_MASK)==GLOBAL_VAL)?global_vals[cur_event[cur_event[4]]&(~GLOBAL_MASK)]:cur_event[cur_event[4]]))) {
			correct_values_eh=1;
		} else {
			correct_values_eh=-2;
		}

	}

	return(1);
	
	
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
		if (length!=cur_event[0]-4) {
 			snprintf(buf,sizeof(buf), "Expected %d parameters got %d", cur_event[0]-4,
				length);
			testnote(buf);
			correct_values=-1;
			return(EOK);
		}
		for (x=0;x<(cur_event[0]-4);x++) {
			if (event_p[x]!=(((cur_event[x+5]&GLOBAL_MASK)==GLOBAL_VAL)?global_vals[cur_event[x+5]&(~GLOBAL_MASK)]:cur_event[x+5])) {
				snprintf(buf,sizeof(buf), "Expected value: %d for parameter %x but got %d", 
						(((cur_event[x+5]&GLOBAL_MASK)==GLOBAL_VAL)?global_vals[cur_event[x+5]&(~GLOBAL_MASK)]:cur_event[x+5]),
						x, event_p[x]);
				testnote(buf);
				correct_values=-1;
				return(EOK);
			}
		}
		if (x==(cur_event[0]-4))
			correct_values=1;
			

	
	} else {
		/* if we are not in fast mode we only get the pid and signo */

		if ((event_p[0]==(((cur_event[cur_event[3]]&GLOBAL_MASK)==GLOBAL_VAL)?global_vals[cur_event[cur_event[3]]&(~GLOBAL_MASK)]:cur_event[cur_event[3]])) && 
			(event_p[1]==(((cur_event[cur_event[4]]&GLOBAL_MASK)==GLOBAL_VAL)?global_vals[cur_event[cur_event[4]]&(~GLOBAL_MASK)]:cur_event[cur_event[4]]))) {
			correct_values=1;
		} else {
			snprintf(buf,sizeof(buf), "Expected values %d and %d  Got values %d and %d", 
				(((cur_event[cur_event[3]]&GLOBAL_MASK)==GLOBAL_VAL)?global_vals[cur_event[cur_event[3]]&(~GLOBAL_MASK)]:cur_event[cur_event[3]]),
				(((cur_event[cur_event[4]]&GLOBAL_MASK)==GLOBAL_VAL)?global_vals[cur_event[cur_event[4]]&(~GLOBAL_MASK)]:cur_event[cur_event[4]]),
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
	struct traceparser_state * tp_state;
	char message[200];
	/*
	 *  This array will define all the tests to be preformed.  It is atually an 
	 *  an array of arrays, where the outside array contains the arrys of tests.
	 *  Each tests array is of the format:
	 *  Number of elemets, Kernel call number, call name,  Number of first fast 
	 *  mode parameter, number of second fast mode parameter, exiexted parameter
	 *  1, expected parameter 2, etc etc 
	 *  where expected parameters are the values that you expect to find in the 
	 *  traceparsers callback functions event pointer.
	 * 
	 *  For example, for the SignalKill kernel call event it would be:
	 *  9, __KER_SIGNAL_KILL,(unsigned)"SignalKill", 5,6, (unsigned)getpid(), (unsigned)pthread_seld(), 
	 *    (unsigned)SIGUSR1, 10,10
	 *  
	 */
	unsigned events[][30] = { 
		{6, __KER_NOP, (unsigned)"NOP", 5, 6, 0, 0 },
		/* Note, TraceEvent is a bit special, since it has variable parameters, we will always get a count
		 * that would indcate the maximum number of parameters. 	
		 */
		{10, __KER_TRACE_EVENT, (unsigned)"TraceEvent", 5, 6, _NTO_TRACE_SETEVENTTID, _NTO_TRACE_KERCALL, __KER_SIGNAL_KILL, 10,10 },
		{6, __KER_SYS_CPUPAGE_GET, (unsigned)"__SysCpupageGet", 5, 6,1,0 },
		{10, __KER_MSG_SENDV, (unsigned)"MsgSendv", 5, 8,COID, 1, 1, *((unsigned *)SEND),0,0},
		{10, __KER_MSG_SENDVNC, (unsigned)"MsgSendvnc", 5, 8,COID, 1, 1,  *((unsigned *)SEND),0,0},
		{6, __KER_MSG_ERROR, (unsigned)"MsgError", 5, 6,USE_GLOBAL_VAL1,10},
		{6, __KER_MSG_RECEIVEV, (unsigned)"MsgReceivev", 5, 6,1,1},
		{10, __KER_MSG_REPLYV, (unsigned)"MsgReplyv", 5, 7,USE_GLOBAL_VAL1,1,EOK,*((unsigned *)SEND),0,0,1,1},
		{8, __KER_MSG_READV, (unsigned)"MsgReadv", 5, 8,USE_GLOBAL_VAL1,(unsigned)&reply,1,0},
		{10, __KER_MSG_WRITEV, (unsigned)"MsgWritev", 5, 7,USE_GLOBAL_VAL1,1,EOK,*((unsigned *)SEND),0,0},
		{6, __KER_MSG_INFO, (unsigned)"MsgInfo", 5, 6,USE_GLOBAL_VAL1,USE_GLOBAL_VAL2},
		{8, __KER_MSG_SEND_PULSE, (unsigned)"MsgSendPulse", 5, 7,COID, 10,11,12},
		{10, __KER_MSG_DELIVER_EVENT, (unsigned)"MsgDeliverEvent", 5, 6,USE_GLOBAL_VAL1,SIGEV_NONE, (unsigned) NULL, 10,(unsigned)NULL},
		{6, __KER_MSG_KEYDATA, (unsigned)"MsgKeyData", 5, 6,USE_GLOBAL_VAL1,_NTO_KEYDATA_CALCULATE },
		{8, __KER_MSG_READIOV, (unsigned)"MsgReadiov", 5, 7,USE_GLOBAL_VAL1,1,0,0},
		{6, __KER_MSG_RECEIVEPULSEV, (unsigned)"MsgReceivePulsev", 5, 6,1,1},
		{10, __KER_MSG_VERIFY_EVENT, (unsigned)"MsgVerifyEvent", 5, 6,0,SIGEV_NONE, (unsigned) NULL, 10,(unsigned)NULL},
		{10, __KER_SIGNAL_KILL, (unsigned)"SignalKill", 6, 8, 0, (unsigned)getpid(),(unsigned) pthread_self(), (unsigned)SIGUSR1, 10,10 },
		{12, __KER_SIGNAL_ACTION, (unsigned)"SignalAction", 7, 8,getpid(),(unsigned) __signalstub, (unsigned)SIGUSR2, (unsigned)sig_hand, 0,0,0},
		{10, __KER_SIGNAL_PROCMASK, (unsigned)"SignalProcmask", 5, 6,getpid(),pthread_self(), SIG_PENDING, USE_GLOBAL_VAL1,USE_GLOBAL_VAL2},
		{6, __KER_SIGNAL_SUSPEND, (unsigned)"SignalSuspend", 5, 6, USE_GLOBAL_VAL1,USE_GLOBAL_VAL2},
		{6, __KER_SIGNAL_WAITINFO, (unsigned)"SignalWaitinfo", 5, 6, USE_GLOBAL_VAL1,USE_GLOBAL_VAL2},
		{6, __KER_CHANNEL_CREATE, (unsigned)"ChannelCreate", 5, 6, _NTO_CHF_FIXED_PRIORITY,0},
		{6, __KER_CHANNEL_DESTROY, (unsigned)"ChannelDestroy", 5, 6, USE_GLOBAL_VAL1,0},
		{10, __KER_CONNECT_ATTACH, (unsigned)"ConnectAttach", 5, 6, 0, getpid(), 1,_NTO_SIDE_CHANNEL, 0},
		{6, __KER_CONNECT_DETACH, (unsigned)"ConnectDetach", 5, 6, USE_GLOBAL_VAL1,0},
		{6, __KER_CONNECT_SERVER_INFO, (unsigned)"ConnectServerInfo", 5, 6, getpid(),1},
		{6, __KER_CONNECT_CLIENT_INFO, (unsigned)"ConnectClientInfo", 5, 6, 1,10},
		{8, __KER_CONNECT_FLAGS, (unsigned)"ConnectFlags", 6, 8, getpid(), USE_GLOBAL_VAL1, 0xF0F0F0F0, 0x0F0F0F0F},
#ifdef __EXT_QNX
		{26, __KER_THREAD_CREATE, (unsigned)"ThreadCreate", 6, 7, getpid(), (unsigned)nothing_thread, NULL, PTHREAD_MULTISIG_DISALLOW, 4096, (unsigned)NULL, (unsigned)NULL, 0, 1,2,3,4,5,6,7,8,0,0,0,0,0,0},
#else 
		{24, __KER_THREAD_CREATE, (unsigned)"ThreadCreate", 6, 7, getpid(), (unsigned)nothing_thread, NULL, PTHREAD_MULTISIG_DISALLOW, 4096, (unsigned)NULL, (unsigned)NULL, 0, 1,2,0,0,0,0,0,0,0,0,0,0},
#endif
		{8, __KER_THREAD_DESTROY, (unsigned)"ThreadDestroy", 5, 7, USE_GLOBAL_VAL1, 10, 10, 0},
		{6, __KER_THREAD_DESTROYALL, (unsigned)"ThreadDestroyAll", 5, 6, 0, 0},
		{6, __KER_THREAD_DETACH, (unsigned)"ThreadDetach", 5, 6, USE_GLOBAL_VAL1, 0},
		{6, __KER_THREAD_JOIN, (unsigned)"ThreadJoin", 5, 6, USE_GLOBAL_VAL1, (unsigned)&global_val2},
		{6, __KER_THREAD_CANCEL, (unsigned)"ThreadCancel", 5, 6, USE_GLOBAL_VAL1, (unsigned)can_stub},
		{6, __KER_THREAD_CTL, (unsigned)"ThreadCtl", 5, 6,_NTO_TCTL_RUNMASK,0xF0F0F0FF },
		{8, __KER_CLOCK_TIME, (unsigned)"ClockTime", 5, 6, CLOCK_REALTIME, 0,0,0},
		{8, __KER_CLOCK_ADJUST, (unsigned)"ClockAdjust", 5, 6, CLOCK_REALTIME, 5,5,0},
		{8, __KER_CLOCK_PERIOD, (unsigned)"ClockPeriod", 5, 6, CLOCK_REALTIME, USE_GLOBAL_VAL1, USE_GLOBAL_VAL2,0},
		{6, __KER_CLOCK_ID, (unsigned)"ClockId", 5, 6, getpid(), pthread_self()},
/* This ifdef is for the last parameter, since the logger will be converting 
 * shorts to longs, this will generate a different result on be and le machines
 */
#if defined(__LITTLEENDIAN__)
		{10, __KER_TIMER_CREATE, (unsigned)"TimerCreate", 5, 6, CLOCK_REALTIME, SIGEV_SIGNAL_CODE, SIGUSR1, 0xababcdcd, 0xff81 },
#else
		{10, __KER_TIMER_CREATE, (unsigned)"TimerCreate", 5, 6, CLOCK_REALTIME, SIGEV_SIGNAL_CODE, SIGUSR1, 0xababcdcd, 0xff810000 },
#endif
		{6, __KER_TIMER_DESTROY, (unsigned)"TimerDestroy", 5, 6, USE_GLOBAL_VAL1, 0 },
		{10, __KER_TIMER_SETTIME, (unsigned)"TimerSettime", 5, 7, USE_GLOBAL_VAL1, 0, 5,6,17,8 },
		{8, __KER_TIMER_INFO, (unsigned)"TimerInfo", 5, 6, getpid(), USE_GLOBAL_VAL1, 0, USE_GLOBAL_VAL2  },
		{10, __KER_TIMER_ALARM, (unsigned)"TimerAlarm", 5, 6, USE_GLOBAL_VAL1, 5,6,17,8 },
/* See TimerCreate  about this define */
#if defined(__LITTLEENDIAN__)
		{12, __KER_TIMER_TIMEOUT, (unsigned)"TimerTimeout", 6, 7, CLOCK_REALTIME, _NTO_TIMEOUT_JOIN, 10,10, SIGEV_SIGNAL_CODE, SIGUSR1, 0xababcdcd, 0xff81 },
#else
		{12, __KER_TIMER_TIMEOUT, (unsigned)"TimerTimeout", 6, 7, CLOCK_REALTIME, _NTO_TIMEOUT_JOIN, 10,10, SIGEV_SIGNAL_CODE, SIGUSR1, 0xababcdcd, 0xff810000 },
#endif
		{12, __KER_SYNC_CREATE, (unsigned)"SyncTypeCreate", 5, 6, _NTO_SYNC_SEM, USE_GLOBAL_VAL1,10, 0, PTHREAD_PRIO_PROTECT, 0, 15, CLOCK_REALTIME },
		{8, __KER_SYNC_DESTROY, (unsigned)"SyncDestroy", 5, 7, USE_GLOBAL_VAL1,0,-4,0},
		{8, __KER_SYNC_MUTEX_LOCK, (unsigned)"SyncMutexLock", 5, 7, USE_GLOBAL_VAL1,USE_GLOBAL_VAL2,0,0},
		{8, __KER_SYNC_MUTEX_UNLOCK, (unsigned)"SyncMutexUnlock", 5, 7, USE_GLOBAL_VAL1,0x80000000, USE_GLOBAL_VAL2,0},
		{10, __KER_SYNC_CONDVAR_WAIT, (unsigned)"SyncCondvarWait", 5, 6, USE_GLOBAL_VAL1,USE_GLOBAL_VAL2,USE_GLOBAL_VAL3,USE_GLOBAL_VAL4,USE_GLOBAL_VAL5,USE_GLOBAL_VAL6},
		{8, __KER_SYNC_CONDVAR_SIGNAL, (unsigned)"SyncCondvarSignal", 5, 6, USE_GLOBAL_VAL1, 10,USE_GLOBAL_VAL2,USE_GLOBAL_VAL3},
		{8, __KER_SYNC_SEM_POST, (unsigned)"SyncSemPost", 5, 6, USE_GLOBAL_VAL1, USE_GLOBAL_VAL2,USE_GLOBAL_VAL3,0},
		{8, __KER_SYNC_SEM_WAIT, (unsigned)"SyncSemWait", 5, 7, USE_GLOBAL_VAL1, 10, USE_GLOBAL_VAL2,USE_GLOBAL_VAL3},
		{10, __KER_SYNC_CTL, (unsigned)"SyncCtl", 5, 6, _NTO_SCTL_GETPRIOCEILING, USE_GLOBAL_VAL1, USE_GLOBAL_VAL2, USE_GLOBAL_VAL3, USE_GLOBAL_VAL4},
		{8, __KER_SYNC_MUTEX_REVIVE, (unsigned)"SyncMutexRevive", 5, 7, USE_GLOBAL_VAL1, USE_GLOBAL_VAL2, USE_GLOBAL_VAL3, 0},
		{6, __KER_SCHED_GET, (unsigned)"SchedGet", 5, 6, getpid(), pthread_self()},
#ifdef __EXT_QNX
		{18, __KER_SCHED_SET, (unsigned)"SchedSet", 5, 8, getpid(), pthread_self(), SCHED_FIFO,10,10,5,5,10,10,10,10,0,0,0},
#else
		{16, __KER_SCHED_SET, (unsigned)"SchedSet", 5, 8, getpid(), pthread_self(), SCHED_FIFO,10,10,0,0,0,0,0,0,0},
#endif
		{6, __KER_SCHED_YIELD, (unsigned)"SchedYield()", 5, 6, 0, 0 },
		{6, __KER_SCHED_INFO, (unsigned)"SchedInfo()", 5, 6, getpid(), SCHED_FIFO },
		{6, __KER_NET_CRED, (unsigned)"NetCred()", 5, 6, USE_GLOBAL_VAL1, USE_GLOBAL_VAL2 },
		{14, __KER_NET_VTID, (unsigned)"NetVtid()", 5, 6, 101,USE_GLOBAL_VAL1, 10,11,12,13,14,15,16,17 },
		{6, __KER_NET_UNBLOCK, (unsigned)"NetUnblock()", 5, 6, 101,0 },
		{6, __KER_NET_INFOSCOID, (unsigned)"NetInfoscoid()", 5, 6, 101,102 },
		{12, __KER_NET_SIGNAL_KILL, (unsigned)"NetSignalKill()", 8, 10, 0,0,0,getpid(), pthread_self(), SIGUSR1, 10,10 },
#ifdef __KER_MSG_CURRENT
		{6, __KER_MSG_CURRENT, (unsigned)"MsgCurrent", 5, 6, 0,0},
#endif
		{0,0,0,0} /*END*/
	};

	path = argv[0];

	// use this trick to determine if this process was created by
	// the shell or by this program
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
	teststart(argv[0]);
	/* Get rid of tracelogger if it is running  */
	tlkilled=kill_tl();
	/* Setup a channel for the MsgSend Events */
	chid=ChannelCreate(0);
	assert(chid!=-1);
	/* and create a thread to block on the channel */
	rc=pthread_create(NULL, NULL, msg_thread, NULL);
	assert(rc==EOK);
	cur_event=events[0];
	mypid=getpid();
	mytid=pthread_self();
	
	/* We should need io privity to attach event handlers */
	rc=ThreadCtl( _NTO_TCTL_IO, 0 );
	assert(rc!=-1);

	while (cur_event[0]!=0) {

		/***********************************************************************/
	
		/***********************************************************************/
		/*
		 * Make sure that if we trigger a event, that it gets logged properly
		 * (all the information in the tracelogger is correct)
		 * This tests the information provided in wide mode.
		 */
		snprintf(message, sizeof(message),"%s in wide mode", (char *)cur_event[2]);
	 	testpntbegin(message);
			
		/* We need to start up the tracelogger in daemon mode, 1 itteration.
		 * we will filter out everything other then our kernel calls, then 
		 * start logging. 
		 * We then will make a kernel call, and flush the trace buffer.  This 
		 * should create a VERY minimal trace buffer that will be easily parsed
		 */
		correct_values=0;
		correct_values_eh=0;
		tlpid=start_logger();
		sleep(1);
		/* Set the logger to wide emmiting mode */
		rc=TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE);
		assert(rc!=-1);
		fast_mode=WIDE;
		/* Add the given kercall event in the Kernel call class back in */
		rc=TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_KERCALL,cur_event[1]);
		assert(rc!=-1);
		/* Filter out all pids but our pid for kernel calls. */
		if (cur_event[1]!=__KER_THREAD_DESTROYALL) {
			rc=TraceEvent(_NTO_TRACE_SETCLASSPID, _NTO_TRACE_KERCALL, getpid());
			assert(rc!=-1);
		}

		/* Setup the event handler */
		memset(&my_event_data, 0, sizeof(my_event_data));
		memset(data_array, 0, sizeof(data_array));
		my_event_data.data_array=data_array;
		rc=TraceEvent(_NTO_TRACE_ADDEVENTHANDLER, _NTO_TRACE_KERCALLENTER,cur_event[1], event_handler, &my_event_data);
		assert(rc!=-1);

		/* then trigger an event.  Logging is started inside the trigger_kercall_event
		 * function immediatly before the kernel call is triggered.
		 */
		
		trigger_kercall_event(cur_event);
		delay(10);
		
		/* flush the trace buffer */
		rc=TraceEvent(_NTO_TRACE_FLUSHBUFFER);	
		assert(rc!=-1);
		rc=waitpid(tlpid, &status, 0);
		assert(tlpid==rc);
		/* Remove the event handler */
		rc=TraceEvent(_NTO_TRACE_DELEVENTHANDLER, _NTO_TRACE_KERCALLENTER,cur_event[1]);
		assert(rc!=-1);
	
		/* Now, setup the traceparser lib to pull out the kernel call events, 
		 * and make sure our event shows up 
		 */
		tp_state=traceparser_init(NULL);
		assert(tp_state!=NULL);
		traceparser_cs(tp_state, NULL, parse_cb, _NTO_TRACE_KERCALLENTER, cur_event[1]);
	
		/* Since we don't want a bunch of output being displayed in the 
		 * middle of the tests, turn off verbose output.
		 */
		traceparser_debug(tp_state, stdout, _TRACEPARSER_DEBUG_NONE);
		/* Set correct_values to 0, so we can see if the callback actually
		 * got called. 
		 */
		/* And parse the tracebuffer */
		traceparser(tp_state, NULL, "/dev/shmem/tracebuffer");
		
		if (correct_values==0) 
			testpntfail("Our callback never got called, no events?");
		else if (correct_values==-1)
			testpntfail("Wrong parameters in the event");
		else if (correct_values!=1)
			testpntfail("This should not happen");
		else if (correct_values_eh==1)
			testpntpass("Good");
		else if (correct_values_eh==0)
			testpntfail("Event handler was never called");
		else if (correct_values_eh<0){
			snprintf(message,sizeof(message),"correct_values_eh:%d",correct_values_eh);
			testnote(message);
			testpntfail("Event handler got incorrect information");
			}
		else
			testpntfail("THis should not happen");

		traceparser_destroy(&tp_state);
	 	testpntend();
		/***********************************************************************/
	
	
		/***********************************************************************/
		/*
		 * Make sure that if we trigger a event, that it gets logged properly
		 * (all the information in the tracelogger is correct)
		 * This tests the information provided in fast mode.
		 */
		snprintf(message, sizeof(message),"%s in fast mode", (char *)cur_event[2]);
	 	testpntbegin(message);
			
		/* We need to start up the tracelogger in daemon mode, 1 itteration.
		 * we will filter out everything other then our kernel calls, then 
		 * start logging. 
		 * We then will make a kernel call, and flush the trace buffer.  This 
		 * should create a VERY minimal trace buffer that will be easily parsed
		 */
		correct_values=0;
		correct_values_eh=0;
		tlpid=start_logger();
		sleep(1);
		/* Set the logger to fast emmiting mode */
		rc=TraceEvent(_NTO_TRACE_SETALLCLASSESFAST);
		assert(rc!=-1);
		fast_mode=FAST;
		/* Add the  event in the Kernel call class back in */
		rc=TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_KERCALLENTER,cur_event[1]);
		assert(rc!=-1);
		/* Filter out all pids but our pid for kernel calls. */
		if (cur_event[1]!=__KER_THREAD_DESTROYALL) {
			rc=TraceEvent(_NTO_TRACE_SETCLASSPID, _NTO_TRACE_KERCALL, getpid());
			assert(rc!=-1);
		}
		/* Setup the event handler */
		memset(&my_event_data, 0, sizeof(my_event_data));
		memset(data_array, 0, sizeof(data_array));
		my_event_data.data_array=data_array;
		TraceEvent(_NTO_TRACE_ADDEVENTHANDLER, _NTO_TRACE_KERCALLENTER,cur_event[1], event_handler, &my_event_data);
		

		trigger_kercall_event(cur_event);
		delay(10);
		
		/* flush the trace buffer */
		rc=TraceEvent(_NTO_TRACE_FLUSHBUFFER);	
		assert(rc!=-1);
		rc=waitpid(tlpid, &status, 0);
		assert(tlpid==rc);

		/* And make sure we remove the event handler */
		rc=TraceEvent(_NTO_TRACE_DELEVENTHANDLER, _NTO_TRACE_KERCALLENTER,cur_event[1]);
		assert(rc!=-1);
	
		/* Now, setup the traceparser lib to pull out the kernel call events, 
		 * and make sure our event shows up 
		 */
		tp_state=traceparser_init(NULL);
		assert(tp_state!=NULL);
		traceparser_cs(tp_state, NULL, parse_cb, _NTO_TRACE_KERCALLENTER, cur_event[1]);
	
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
		else if (correct_values!=1)
			testpntfail("This should not happen");
		else if (correct_values_eh==1)
			testpntpass("Good");
		else if (correct_values_eh==0)
			testpntfail("Event handler was never called");
		else if (correct_values_eh<0) 
			testpntfail("Event handler got incorrect information");
		else
			testpntfail("THis should not happen");

		traceparser_destroy(&tp_state);
	 	testpntend();
	
		/***********************************************************************/

		/* Go to the next event to test */
		cur_event+=30;
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
*				with the given parameters. This will trigger only 
*				kernel call events, and should generate both an entry
*				and exit event
*				This function will also start the loging, as some types
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
	uint64_t timeout,oldtime;
	struct _server_info myserverinfo;
	unsigned a_value;
	struct _thread_attr mythreadattr;
	struct inheritance	myinherit;
	pthread_t mythread;
	struct _clockadjust myadjustment;
	struct _clockperiod myclockperiod;
	struct _itimer myitime;
	struct _timer_info mytimerinfo;
	sync_t mysync,mysync2;
	sync_attr_t mysyncattr;
	struct sched_param myschedparam;
	struct _sched_info myschedinfo;
	struct _vtid_info myvtidinfo;
	struct {
		unsigned nd;
		pid_t    pid;
		int32_t  tid;
		int32_t  signo;
		int32_t  code;
		int32_t  value;
	} signal_info;
	struct _cred_info mycredinfo;
	switch(event_p[1]) {
		case __KER_NOP:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			__kernop();
			return(EOK);
			break;
		case __KER_TRACE_EVENT:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			TraceEvent(_NTO_TRACE_SETEVENTTID, _NTO_TRACE_KERCALL, 	__KER_SIGNAL_KILL,10,10);
			return(EOK);
			break;
		case __KER_SYS_CPUPAGE_GET:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			__SysCpupageGet(1);
			break;
		case __KER_MSG_SENDV:
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
			MsgSendv(coid, &send, 1, &reply, 1);
			break;
		case __KER_MSG_SENDVNC:
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
			MsgSendvnc(coid, &send, 1, &reply, 1);
			break;
		case __KER_MSG_ERROR:
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
			MsgError(rcvid, 10);
			break;
		case __KER_MSG_RECEIVEV:
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			rcvid=MsgReceivev(chid, &reply, 1,NULL);
			MsgReply(rcvid, EOK,  "bye", 4);
			break;
		case __KER_MSG_REPLYV:
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
			MsgReplyv(rcvid, EOK,  &send, 1);
			break;
		case __KER_MSG_READV:
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			rcvid=MsgReceivev(chid, &reply, 1,NULL);
			global_vals[0]=rcvid;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			MsgReadv(rcvid,&reply, 1,0);
			MsgReply(rcvid, EOK,  "bye", 4);
			break;
		case __KER_MSG_WRITEV:
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
			MsgWritev(rcvid, &send, 1, 0);
			MsgReplyv(rcvid, EOK,  &send, 1);
			break;

		case __KER_MSG_INFO:
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			rcvid=MsgReceivev(chid, &reply, 1,NULL);
			global_vals[0]=rcvid;
			global_vals[1]=(unsigned)&myinfo;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			MsgInfo(rcvid, &myinfo);
			MsgReply(rcvid, EOK,  "OK", 3);
			break;
		case __KER_MSG_SEND_PULSE:
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			MsgSendPulse(coid, 10,11,12);
			return(EOK);
			break;
		case __KER_MSG_DELIVER_EVENT:
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			rcvid=MsgReceivev(chid, &reply, 1,NULL);
			global_vals[0]=rcvid;
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
			MsgDeliverEvent(rcvid, &myevent);
			break;
		case __KER_MSG_KEYDATA:
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			rcvid=MsgReceivev(chid, &reply, 1,NULL);
			global_vals[0]=rcvid;
			MsgReplyv(rcvid, EOK,  &send, 1);
			
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			MsgKeyData(rcvid, _NTO_KEYDATA_CALCULATE,101,&a_value,&reply, 1);
			break;
		case __KER_MSG_READIOV:
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "SEND", 5, reply_buf, sizeof(reply_buf));
			rcvid=MsgReceivev(chid, &reply, 1,NULL);
			global_vals[0]=rcvid;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			MsgReadiov(rcvid,&reply, 1,0,0);
			MsgReply(rcvid, EOK,  "bye", 4);
			break;
		case __KER_MSG_RECEIVEPULSEV:
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			SETIOV(&reply, reply_buf, 4);
			MsgSend(coid, "PSEND", 6, reply_buf, sizeof(reply_buf));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			rcvid=MsgReceivePulsev(chid, &reply, 1,NULL);
			break;
		case __KER_MSG_VERIFY_EVENT:
			/*Setup the event to check */
			myevent.sigev_notify=SIGEV_NONE;
			myevent.sigev_notify_function=NULL;
			myevent.sigev_notify_attributes=NULL;
			myevent.sigev_value.sival_int=10;
		
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			MsgVerifyEvent(0, &myevent);
			break;
		case __KER_SIGNAL_KILL:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			rc=SignalKill(0,getpid(), pthread_self(), event_p[8], event_p[9],event_p[10]);
			if (rc!=-1)
				return(EOK);
			else 
				return(-1);
			break;
		case __KER_SIGNAL_ACTION:
			memset(&myaction, 0, sizeof(myaction));
			sigemptyset(&myaction.sa_mask);
			myaction.sa_handler=sig_hand;

			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SignalAction(getpid(), __signalstub, SIGUSR2, &myaction, NULL);

			break;
		case __KER_SIGNAL_PROCMASK:
			sigemptyset(&myset);
			sigaddset(&myset, SIGHUP);
			sigaddset(&myset, SIGBUS);
			sigaddset(&myset, SIGSEGV);
			sigaddset(&myset, SIGXCPU);
			sigaddset(&myset, SIGRTMIN);
			sigaddset(&myset, SIGRTMAX);
			global_vals[0]= ((unsigned *)(&myset))[0];
			global_vals[1]= ((unsigned *)(&myset))[1];

			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			rc=SignalProcmask(getpid(), pthread_self(),SIG_PENDING,&myset, &myset);

			break;
		case __KER_SIGNAL_SUSPEND:
			memset(&myevent, 0, sizeof(myevent));
			myevent.sigev_notify = SIGEV_UNBLOCK;
			timeout = 1*1000000000L;

			sigemptyset(&myset);
			sigaddset(&myset, SIGHUP);
			sigaddset(&myset, SIGBUS);
			sigaddset(&myset, SIGSEGV);
			sigaddset(&myset, SIGXCPU);
			sigaddset(&myset, SIGRTMIN);
			sigaddset(&myset, SIGRTMAX);
			global_vals[0]= ((unsigned *)(&myset))[0];
			global_vals[1]= ((unsigned *)(&myset))[1];
	
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			TimerTimeout( CLOCK_REALTIME, _NTO_TIMEOUT_SIGSUSPEND,&myevent, &timeout, NULL );
			SignalSuspend(&myset);
			
			return(EOK);
			break;

		case __KER_SIGNAL_WAITINFO:
			memset(&myevent, 0, sizeof(myevent));
			myevent.sigev_notify = SIGEV_UNBLOCK;
			timeout = 1*1000000000L;

			sigemptyset(&myset);
			sigaddset(&myset, SIGHUP);
			sigaddset(&myset, SIGBUS);
			sigaddset(&myset, SIGSEGV);
			sigaddset(&myset, SIGXCPU);
			sigaddset(&myset, SIGRTMIN);
			sigaddset(&myset, SIGRTMAX);
			global_vals[0]= ((unsigned *)(&myset))[0];
			global_vals[1]= ((unsigned *)(&myset))[1];
	
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			TimerTimeout( CLOCK_REALTIME, _NTO_TIMEOUT_SIGWAITINFO,&myevent, &timeout, NULL );
			SignalWaitinfo(&myset,NULL);
			
			return(EOK);
			break;
		case __KER_CHANNEL_CREATE:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			rc=ChannelCreate(event_p[5]);
			ChannelDestroy(rc);
			return(EOK);
			break;

		case __KER_CHANNEL_DESTROY:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			rc=ChannelCreate(_NTO_CHF_FIXED_PRIORITY);
			delay(10);
			global_vals[0]=rc;
			assert(rc!=-1);
			ChannelDestroy(rc);
			return(EOK);
			break;
		case __KER_CONNECT_ATTACH:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			rc=ConnectAttach(event_p[5], event_p[6], event_p[7],  event_p[8], event_p[9]);
			ConnectDetach(rc);
			break;
		case __KER_CONNECT_DETACH:
			rc=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			global_vals[0]=rc;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			ConnectDetach(global_vals[0]);
			break;
		case __KER_CONNECT_SERVER_INFO:
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			ConnectServerInfo(event_p[5], event_p[6],&myserverinfo);
			return(EOK);
			break;
		case __KER_CONNECT_CLIENT_INFO:
			if (coid==0)
				coid=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(coid!=-1);
			assert(coid==COID);
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			ConnectClientInfo(event_p[5], &myclientinfo, event_p[6]);
			return(EOK);
			break;
		case __KER_CONNECT_FLAGS:
			rc=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			global_vals[0]=rc;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			ConnectFlags(event_p[5], global_vals[0],event_p[7], event_p[8]);
			ConnectDetach(global_vals[0]);
			break;
		case __KER_THREAD_CREATE:
			/*Setup all the thread attribure structes that we need */
			mythreadattr.flags=event_p[8];
			mythreadattr.stacksize=event_p[9];
			mythreadattr.stackaddr=(void *)event_p[10];
			mythreadattr.exitfunc=(void *)event_p[11];
			mythreadattr.policy=event_p[12];
			mythreadattr.param.sched_priority=event_p[13];
			mythreadattr.param.sched_curpriority=event_p[14];
#ifdef __EXT_QNX
			mythreadattr.param.sched_ss_low_priority=event_p[15];
			mythreadattr.param.sched_ss_max_repl=event_p[16];
			mythreadattr.param.sched_ss_repl_period.tv_sec=event_p[17];
			mythreadattr.param.sched_ss_repl_period.tv_nsec=event_p[18];
			mythreadattr.param.sched_ss_init_budget.tv_sec=event_p[19];
			mythreadattr.param.sched_ss_init_budget.tv_nsec=event_p[20];
			mythreadattr.param.__ss_un.reserved[6]=event_p[21];
			mythreadattr.param.__ss_un.reserved[7]=event_p[22];
			mythreadattr.guardsize=event_p[23];
			mythreadattr.spare[0]=event_p[24];
			mythreadattr.spare[1]=event_p[25];
			mythreadattr.spare[2]=event_p[26];
#else
			mythreadattr.spare[0]=event_p[15];
			mythreadattr.spare[1]=event_p[16];
			mythreadattr.spare[2]=event_p[17];
			mythreadattr.spare[3]=event_p[18];
			mythreadattr.spare[4]=event_p[19];
			mythreadattr.spare[5]=event_p[20];
			mythreadattr.guardsize=event_p[21];
			mythreadattr.spare[0]=event_p[22];
			mythreadattr.spare[1]=event_p[23];
			mythreadattr.spare[2]=event_p[26];
#endif
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			ThreadCreate(event_p[5], (void *)event_p[6], (void *)event_p[7], &mythreadattr);
			break;
		case __KER_THREAD_DESTROY:
			/* create a thread to destroy */
			rc=pthread_create(&mythread, NULL, nothing_thread, NULL);
			assert(rc==EOK);
			global_vals[0]=mythread;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			ThreadDestroy(mythread, event_p[6], (void *)event_p[7]);
			break;
		case __KER_THREAD_DESTROYALL:
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
			rc=waitpid(child, NULL, 0);
			assert(rc==child);
			break;
		case __KER_THREAD_DETACH:
			/* create a thread to destroy */
			rc=pthread_create(&mythread, NULL, nothing_thread, NULL);
			assert(rc==EOK);
			global_vals[0]=mythread;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			ThreadDetach(mythread);
			break;
		case __KER_THREAD_JOIN:
			/* create a thread to destroy */
			rc=pthread_create(&mythread, NULL, nothing_thread, NULL);
			assert(rc==EOK);
			global_vals[0]=mythread;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			ThreadJoin(mythread, (void **)event_p[6]);
			break;
		case __KER_THREAD_CANCEL:
			/* create a thread to destroy */
			rc=pthread_create(&mythread, NULL, nothing_thread, NULL);
			assert(rc==EOK);
			global_vals[0]=mythread;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			ThreadCancel(mythread,(void *)event_p[6]);
			break;
		case __KER_THREAD_CTL:
			/* create a thread to destroy */
			rc=pthread_create(&mythread, NULL, nothing_thread, NULL);
			assert(rc==EOK);
			global_vals[0]=mythread;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			ThreadCtl(event_p[5], (void *)event_p[6]);
			break;
		case __KER_CLOCK_TIME:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			ClockTime(CLOCK_REALTIME, NULL, &oldtime);
			break;
		case __KER_CLOCK_ADJUST:
			/* This will cause the time to go ahead a total of 	
			 * 25 nano seconds, which we should never really notice..
			 */
			myadjustment.tick_nsec_inc=5;
			myadjustment.tick_count=5;
	
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			ClockAdjust(CLOCK_REALTIME, &myadjustment, &myadjustment);
			break;
		case __KER_CLOCK_PERIOD:
			ClockPeriod(CLOCK_REALTIME, NULL, &myclockperiod,0);
			global_vals[0]=myclockperiod.nsec;
			global_vals[1]=myclockperiod.fract;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			ClockPeriod(CLOCK_REALTIME, &myclockperiod, &myclockperiod,0);
			break;
		case __KER_CLOCK_ID:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			rc=ClockId(event_p[5], event_p[6]);
			break;
		case __KER_TIMER_CREATE:
			memset(&myevent, 0,sizeof(myevent));
			SIGEV_SIGNAL_CODE_INIT( &myevent, event_p[7], event_p[8], SI_MINAVAIL+1 );
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			rc=TimerCreate(event_p[5], &myevent);
			TimerDestroy(rc);
			break;
		case __KER_TIMER_DESTROY:
			memset(&myevent, 0,sizeof(myevent));
			SIGEV_SIGNAL_CODE_INIT( &myevent,SIGUSR1,0, SI_MINAVAIL+1 );
			global_vals[0]=TimerCreate(CLOCK_REALTIME, &myevent);
			
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			TimerDestroy(global_vals[0]);
			break;
		case __KER_TIMER_SETTIME:
	
			memset(&myevent, 0,sizeof(myevent));
			SIGEV_SIGNAL_CODE_INIT( &myevent,SIGUSR1,0, SI_MINAVAIL+1 );
			global_vals[0]=TimerCreate(CLOCK_REALTIME, &myevent);
			myitime.nsec=1000000000L;
			myitime.nsec*= event_p[7];
			myitime.nsec+=event_p[8];

			myitime.interval_nsec=1000000000L;
			myitime.interval_nsec*=event_p[9];
			myitime.interval_nsec+=event_p[10];
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			TimerSettime(global_vals[0],0, &myitime, &myitime);
			TimerDestroy(global_vals[0]);
			break;
		case __KER_TIMER_INFO:
			memset(&myevent, 0,sizeof(myevent));
			SIGEV_SIGNAL_CODE_INIT( &myevent,SIGUSR1,0, SI_MINAVAIL+1 );
			global_vals[0]=TimerCreate(CLOCK_REALTIME, &myevent);
			global_vals[1]=(unsigned)&mytimerinfo;	
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			TimerInfo(event_p[5], global_vals[0], event_p[7], (void *)global_vals[1]);
			TimerDestroy(global_vals[0]);
			break;
		case __KER_TIMER_ALARM:
			memset(&myevent, 0,sizeof(myevent));
			SIGEV_SIGNAL_CODE_INIT( &myevent,SIGUSR1,0, SI_MINAVAIL+1 );
			global_vals[0]=TimerCreate(CLOCK_REALTIME, &myevent);
			myitime.nsec=1000000000L;
			myitime.nsec*= event_p[6];
			myitime.nsec+=event_p[7];

			myitime.interval_nsec=1000000000L;
			myitime.interval_nsec*=event_p[8];
			myitime.interval_nsec+=event_p[9];
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			TimerAlarm(global_vals[0],&myitime, &myitime);
			myitime.nsec=0;
			myitime.interval_nsec=0;
			TimerAlarm(global_vals[0],&myitime, &myitime);
			TimerDestroy(global_vals[0]);
			break;
		case __KER_TIMER_TIMEOUT:
			memset(&myevent, 0,sizeof(myevent));
			SIGEV_SIGNAL_CODE_INIT( &myevent, event_p[10], event_p[11], SI_MINAVAIL+1 );
			timeout=1000000000L;
			timeout*= event_p[7];
			timeout+=event_p[8];
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			TimerTimeout(event_p[5], event_p[6], &myevent, &timeout, &timeout);
			break;
		case __KER_SYNC_CREATE:
			memset(&mysync, 0, sizeof(mysync));
			memset(&mysyncattr, 0, sizeof(mysyncattr));
			mysync.count=event_p[7];
			mysync.owner=event_p[8];
			mysyncattr.protocol=event_p[9];
			mysyncattr.flags=event_p[10];
			mysyncattr.prioceiling=event_p[11];
			mysyncattr.clockid=event_p[12];
			global_vals[0]=(unsigned)&mysync;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SyncTypeCreate(event_p[5], &mysync, &mysyncattr);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_DESTROY:
			memset(&mysync, 0, sizeof(mysync));
			mysync.count=event_p[6];
			mysync.owner=event_p[7];
			SyncTypeCreate(_NTO_SYNC_SEM, &mysync, NULL);
			global_vals[0]=(unsigned)&mysync;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_MUTEX_LOCK:
			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_MUTEX_FREE , &mysync, NULL);
			global_vals[0]=(unsigned)&mysync;
			global_vals[1]=mysync.count;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SyncMutexLock(&mysync);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_MUTEX_UNLOCK:
			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_MUTEX_FREE , &mysync, NULL);
			SyncMutexLock(&mysync);
			global_vals[0]=(unsigned)&mysync;
			global_vals[1]=mysync.owner;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SyncMutexUnlock(&mysync);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_CONDVAR_WAIT:
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
			global_vals[1]=(unsigned)&mysync2;
			global_vals[2]=mysync.count;
			global_vals[3]=mysync.owner;
			global_vals[4]=mysync2.count;
			global_vals[5]=mysync2.owner;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SyncCondvarWait(&mysync,&mysync2);
			SyncDestroy(&mysync);
			SyncDestroy(&mysync2);
			break;
		case __KER_SYNC_CONDVAR_SIGNAL:
			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_COND  , &mysync, NULL);
			global_vals[0]=(unsigned)&mysync;
			global_vals[1]=mysync.count;
			global_vals[2]=mysync.owner;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SyncCondvarSignal(&mysync, event_p[6]);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_SEM_POST:
			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_SEM, &mysync, NULL);
			global_vals[0]=(unsigned)&mysync;
			global_vals[1]=mysync.count;
			global_vals[2]=mysync.owner;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SyncSemPost(&mysync);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_SEM_WAIT:
			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_SEM, &mysync, NULL);
			global_vals[0]=(unsigned)&mysync;
			global_vals[1]=mysync.count;
			global_vals[2]=mysync.owner;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SyncSemWait(&mysync, event_p[6]);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_CTL:
			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_MUTEX_FREE , &mysync, NULL);
			global_vals[0]=(unsigned)&mysync;
			global_vals[1]=(unsigned)&timeout;
			global_vals[2]=mysync.count;
			global_vals[3]=mysync.owner;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SyncCtl(event_p[5],&mysync, &timeout);
			SyncDestroy(&mysync);
			break;
		case __KER_SYNC_MUTEX_REVIVE:
			memset(&mysync, 0, sizeof(mysync));
			SyncTypeCreate(_NTO_SYNC_MUTEX_FREE , &mysync, NULL);
			global_vals[0]=(unsigned)&mysync;
			global_vals[1]=mysync.count;
			global_vals[2]=mysync.owner;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SyncMutexRevive(&mysync);
			SyncDestroy(&mysync);
			break;
		case __KER_SCHED_GET:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SchedGet(event_p[5], event_p[6], &myschedparam);
			return(EOK);
			break;
		case __KER_SCHED_SET:
			memset(&myschedparam, 0, sizeof(myschedparam));
			myschedparam.sched_priority=event_p[8];
			myschedparam.sched_curpriority=event_p[9];
#ifdef __EXT_QNX
			myschedparam.sched_ss_low_priority=event_p[10];
			myschedparam.sched_ss_max_repl=event_p[11];
			myschedparam.sched_ss_repl_period.tv_sec=event_p[12];
			myschedparam.sched_ss_repl_period.tv_nsec=event_p[13];
			myschedparam.sched_ss_init_budget.tv_sec=event_p[14];
			myschedparam.sched_ss_init_budget.tv_nsec=event_p[15];
#endif
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SchedSet(event_p[5], event_p[6], event_p[7], &myschedparam);
			return(EOK);
			break;
		case __KER_SCHED_YIELD:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SchedYield();
			return(EOK);
			break;
		case __KER_SCHED_INFO:
			memset(&myschedinfo, 0, sizeof(myschedinfo));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			SchedInfo(event_p[5], event_p[6], &myschedinfo);
			return(EOK);
			break;
		case __KER_NET_CRED:
			global_vals[0]=ConnectAttach(0, getpid(), chid,  _NTO_SIDE_CHANNEL, 0);
			assert(global_vals[0]!=-1);
			global_vals[1]=(unsigned)&myclientinfo;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			NetCred(global_vals[0], (struct _client_info *)global_vals[1]);
			ConnectDetach(global_vals[0]);
			break;
		case __KER_NET_VTID:
			memset(&myvtidinfo, 0, sizeof(myvtidinfo));
			myvtidinfo.tid=event_p[7];
			myvtidinfo.coid=event_p[8];
			myvtidinfo.priority=event_p[9];
			myvtidinfo.srcmsglen=event_p[10];
			myvtidinfo.keydata=event_p[11];
			myvtidinfo.srcnd=event_p[12];
			myvtidinfo.dstmsglen=event_p[13];
			myvtidinfo.zero=event_p[14];
			global_vals[0]=(unsigned)&myvtidinfo;
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			NetVtid(event_p[5], &myvtidinfo);	
			break;
		case __KER_NET_UNBLOCK:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			NetUnblock(event_p[5]);	
			break;
		case __KER_NET_INFOSCOID:
			memset(&myvtidinfo, 0, sizeof(myvtidinfo));
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			NetInfoscoid(event_p[5], event_p[6]);	
			break;
		case __KER_NET_SIGNAL_KILL:
			memset(&signal_info, 0, sizeof(signal_info));
			memset(&mycredinfo, 0, sizeof(mycredinfo));
			signal_info.nd=event_p[7];
			signal_info.pid=event_p[8];
			signal_info.tid=event_p[9];
			signal_info.signo=event_p[10];
			signal_info.code=event_p[11];
			signal_info.value=event_p[12];
			mycredinfo.ruid=event_p[5];
			mycredinfo.euid=event_p[6];
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			NetSignalKill(&signal_info, &mycredinfo);
			break;
#ifdef __KER_MSG_CURRENT
		case __KER_MSG_CURRENT:
			/* Start logging */
			rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
			assert(rc!=-1);
			delay(10);
			MsgCurrent(event_p[5]);
			break;
#endif

	}
	return(-1);
}
