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
*	File:		bk1_threadstate.c
*
******************************************************************************
*
*   Contents:	Tests to make sure the instrumentation properly intercepts
*				thread state changes (scheduler activity)
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
#include <sys/syspage.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/neutrino.h>
#include <sys/procfs.h>
#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>


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

/* This is a global that is set to tell us if we are in fast mode (1)
 * or wide mode (0)
 */
static int fast_mode;

/* 
 * This is used to tell the state thread which state to try to get into.
 */
unsigned  cur_state;

/* 
 * This is the thread id of the state thread. This is what we will expect to 
 * get in the traceparser callback.
 */
pthread_t exp_thread;

/* 
 *  This is a barrier used to syncronize between the main thread and the 
 *  stat thread
 */
pthread_barrier_t global_barrier;

/* This is the channel that will be setup in main, and used by trigger_event
 * to trigger the various message related kernel calls.
 */
int chid;

/* This is used mostly for the READY state, this will tell a thread that
 * will be running ready when it should exit
 */
int exit_now;

/* This is the pid that the main thread will start up at the start that will
 * be used in the STOPPED state test. 
 */
int child_pid;

/* A mutex to be used to trigger the mutex and condvar state */
pthread_mutex_t mymutex;

/* A condvar to be used to trigger the condvar state */
pthread_cond_t mycondvar;

/* A semaphore to be used to trigger the sem state */
sem_t mysem;

/* This is just a list of strings to print out the thread states*/
const char * const state_str[] = {
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
*						Subroutine ready_thread 
*
*	Purpose: 	This is a do nothing thread that will just loop running
*				ready until exit_now becomes 1
*
*	Parameters:	None
*
*	Returns:	Nothing
*			
*
*****************************************************************************/
void * ready_thread(void * arg)
{
	int x;
	while (exit_now==0)
		x++;
	return(NULL);
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
	sleep(1);
	return(NULL);
}
/****************************************************************************
*
*						Subroutine state_thread
*
*	Purpose: 	This is a thread that is used to trigger the various thread
*				states
*
*	Parameters:	None
*
*	Returns:	Nothing
*			
*
*****************************************************************************/
void * state_thread(void * arg)
{
	int rc,x,coid;
	sigset_t myset;
	uint64_t timeout;
	struct sigevent myevent;
	pthread_t mythread;
	char buf[100];
	while(1) {
		/* Syncronize with the main thread. */
		pthread_barrier_wait(&global_barrier);
		switch (cur_state) {
			case STATE_DEAD: 
				/* Start the logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				delay(10);
				/* Trigger the dead state by calling pthread_exit */
				pthread_exit(NULL);
				/* Should never get here */
				abort();
			case STATE_READY: 
				exit_now=0;
				for (x=0;x<_syspage_ptr->num_cpu;x++)
					pthread_create(NULL, NULL, ready_thread, NULL);
				/*  Let the new threads get started */
				sleep(1);
			case STATE_RUNNING: 
				/* Start the logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				delay(10);
				/* Trigger the running state by just doing some work. 
				 * this should also trigger a ready state before
				 * we are running
				 */
				x=0;
				while(x<10)
					x++;
				exit_now=1;
				while(x<1000)
					x++;
				exit_now=1;
				/* and unblock the parent */
				pthread_barrier_wait(&global_barrier);
				break;
			case STATE_STOPPED: 

				/* Start the logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				delay(100);
				kill (child_pid, SIGSTOP);
				delay(10);
				kill (child_pid, SIGCONT);
				delay(100);
				pthread_barrier_wait(&global_barrier);
				break;
			case STATE_SEND: 
			case STATE_REPLY: 
				coid=ConnectAttach(0, getpid(), chid, _NTO_SIDE_CHANNEL, 0);
				/* Start the logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				delay(10);
				/* Trigger the SEND/REPLY blocked states */
				MsgSend(coid, buf, 10, buf,10);
				pthread_barrier_wait(&global_barrier);
				break;
			case STATE_RECEIVE: 
				/* Start the logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				delay(10);
				/* Trigger a receive state  */
				rc=MsgReceive(chid, buf, sizeof(buf), NULL);
				MsgReply(rc, EOK, "ok", 3);
				pthread_barrier_wait(&global_barrier);
				break;
			case STATE_WAITTHREAD: 
				/* Start the logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				delay(10);
				/* Trigger a waitthread state  */
				pthread_create(NULL, NULL, nothing_thread, NULL);
				pthread_barrier_wait(&global_barrier);
				break;
			case STATE_SIGSUSPEND: 
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
		
				/* Start logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				delay(10);
				TimerTimeout( CLOCK_REALTIME, _NTO_TIMEOUT_SIGSUSPEND,&myevent, &timeout, NULL );
				SignalSuspend(&myset);
				pthread_barrier_wait(&global_barrier);
				break;
			case STATE_SIGWAITINFO: 
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
		
				/* Start logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				delay(10);
				TimerTimeout( CLOCK_REALTIME, _NTO_TIMEOUT_SIGWAITINFO,&myevent, &timeout, NULL );
				SignalWaitinfo(&myset,NULL);
				pthread_barrier_wait(&global_barrier);
				break;
			case STATE_NANOSLEEP:
				/* Start logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				sleep(1);
				sleep(1);
				pthread_barrier_wait(&global_barrier);
				break;
			case STATE_MUTEX:
				/* Start logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				sleep(1);
				pthread_mutex_lock(&mymutex);
				pthread_mutex_unlock(&mymutex);
				pthread_barrier_wait(&global_barrier);
				break;
			case STATE_CONDVAR:
				/* Start logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				sleep(1);
				pthread_mutex_lock(&mymutex);
				pthread_cond_wait(&mycondvar, &mymutex);
				pthread_mutex_unlock(&mymutex);
				pthread_barrier_wait(&global_barrier);
				break;
			case STATE_JOIN:
				pthread_create(&mythread, NULL, nothing_thread, NULL);
				/* Start logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				delay(10);
				pthread_join(mythread, NULL);
				pthread_barrier_wait(&global_barrier);
				break;
			case STATE_SEM:
				/* Start logging */
				rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
				assert(rc!=-1);
				sleep(1);
				sem_wait(&mysem);
				pthread_barrier_wait(&global_barrier);
				break;
			
		}
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

	/* This is to handle the case when we may see multiple events in the 
	 * trace logger.  If we have seen the correct event, then we can just
	 * ignore this one.  
	 */
	if (correct_values==1) 
		return(EOK);
	/*
	 * Now check that the values that we recieved in the callback are the expected values.
	 */
	if (cur_state != _NTO_TRACE_GETEVENT(header)) 
		correct_values=-1;
	if (event_p[0]!=((cur_state==STATE_STOPPED)?child_pid:getpid())) 
		correct_values=-2;
	if (event_p[1]!=((cur_state==STATE_STOPPED)?1:exp_thread))
		correct_values=-3;

	if (correct_values==0)
		correct_values=1;

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
	int tlkilled,tlpid, rc,status;  /*tlpid=trace logger pid */
	struct traceparser_state * tp_state;
	char message[200];
	pthread_t cur_thread;

	/*
	 * Start the tests.
	 */
	teststart(argv[0]);
	/* Get rid of tracelogger if it is running  */
	tlkilled=kill_tl();

	/* Setup a channel for the MsgSend Events */
	chid=ChannelCreate(0);
	assert(chid!=-1);
	
	/* Setup the barrier used to syncronize */
	rc=pthread_barrier_init(&global_barrier, NULL, 2);
	assert(rc==EOK);

	/* Setup the mutex for the mutex/condvar state test */
	rc=pthread_mutex_init(&mymutex,NULL);	
	assert(rc==EOK);

	/* Setup the condvar for the condvar state test */
	rc=pthread_cond_init(&mycondvar,NULL);	
	assert(rc==EOK);

	/* setup the sem tfor the sem state test */
	rc=sem_init(&mysem, 0,0);
	assert(rc==0);

	/* Setup the needed child */
	child_pid=fork();
	assert(child_pid!=-1);
	if (child_pid==0) {
		/* This child is used only in the stopped state test.
 		 * it will just sit and loop
		 */
		while (1) {
			delay(10);
		}
		exit(0);
	}

	/* and create a thread to go into various states */
	rc=pthread_create(&cur_thread, NULL, state_thread, NULL);
	assert(rc==EOK);
	for (cur_state=STATE_DEAD;cur_state<=STATE_SEM;cur_state++) {
		/* these states can not be reliably and or safely triggered. */
		if ((cur_state==STATE_STACK) || (cur_state==STATE_WAITPAGE) || cur_state==STATE_INTR)
			continue;
	
	
		/***********************************************************************/
	
		/***********************************************************************/
		/*
		 * Make sure that if we trigger a thread state, that it gets logged
		 * properly (all the information in the tracelogger is correct)
		 * This tests the information provided in wide mode.
		 */
		snprintf(message, sizeof(message),"%s in wide mode", state_str[cur_state]);
	 	testpntbegin(message);

		exp_thread=cur_thread;
			
		/* We need to start up the tracelogger in daemon mode, 1 itteration.
		 * we will filter out everything other then thread states, then 
		 * start logging. 
		 * We then will trigger a thread state change, and flush the trace buffer. 
		 * This  should create a VERY minimal trace buffer that will be easily parsed
		 */
		tlpid=start_logger();
		sleep(1);
		/* Set the logger to wide emmiting mode */
		rc=TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE);
		assert(rc!=-1);
		fast_mode=WIDE;
		/* Add the given thread event in the thread class back in */
		rc=TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_THREAD,(1<<cur_state));
		assert(rc!=-1);
		/* Filter out all tids other then the state_thread */
		if  (cur_state==STATE_STOPPED) {
			rc=TraceEvent(_NTO_TRACE_SETCLASSTID, _NTO_TRACE_THREAD, child_pid, 1);
		} else {
			rc=TraceEvent(_NTO_TRACE_SETCLASSTID, _NTO_TRACE_THREAD, getpid(),cur_thread);
		}
		assert(rc!=-1);

		/* then trigger an event.  Logging is started inside the state thread
		 * right before it tries to trigger the given state.
		 * the two barrier waits are to 
		 * 1) Tell the state thread that everything is setup so it should trigger the event
		 * 2) Wait for the state thread to tell us it has finished.
		 */
		pthread_barrier_wait(&global_barrier);

		if ((cur_state==STATE_SEND)|| (cur_state==STATE_REPLY)) {
			/* If the thread is going into the send state, we should receive and reply */
			sleep(1);
			status=MsgReceive(chid, message, sizeof(message), NULL);
			MsgReply(status, EOK, "ok", 3);
		} else if (cur_state==STATE_RECEIVE) {
			/* If the thread is going to call reveive, we should send it a message */
			sleep(1);
			status=ConnectAttach(0, getpid(), chid, _NTO_SIDE_CHANNEL, 0);
			MsgSend(status, message, 10, message,10);
		} else if (cur_state==STATE_MUTEX) {
			pthread_mutex_lock(&mymutex);
			sleep(2);
			pthread_mutex_unlock(&mymutex);
		} else if (cur_state==STATE_CONDVAR) {
			sleep(2);
			pthread_cond_signal(&mycondvar);
		} else if (cur_state==STATE_SEM) {
			sleep(2);
			sem_post(&mysem);
		}
		
		/* If the state thread is going to try to trigger the dead state, it will have to
		 * exit, so we can not expect it to call barrier_wait to tell us it's done. 
		 */
		if ((1<<cur_state)!=_NTO_TRACE_THDEAD) {
			pthread_barrier_wait(&global_barrier);
		} else {
			/* If the state thread is going to exit, then we should just
 			 * give it time to exit, then restart it.
			 */
			sleep(2); 
			rc=pthread_join(cur_thread, (void **)&status);
			assert(rc==EOK);
			rc=pthread_create(&cur_thread, NULL, state_thread, NULL);
		}
		delay(100);
		
		/* flush the trace buffer and wait for the tracelogger to exit*/
		rc=TraceEvent(_NTO_TRACE_FLUSHBUFFER);	
		assert(rc!=-1);
		rc=waitpid(tlpid, &status, 0);
		assert(tlpid==rc);
	
		/* Now, setup the traceparser lib to pull out the thread state events, 
		 * and make sure our event shows up 
		 */
		tp_state=traceparser_init(NULL);
		assert(tp_state!=NULL);
		traceparser_cs(tp_state, NULL, parse_cb, _NTO_TRACE_THREAD, (1<<cur_state));
	
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
			testpntfail("Got the wrong thread state");
		else if (correct_values==-2) 
			testpntfail("Got the wrong pid");
		else if (correct_values==-3)
			testpntfail("Got the wrong tid");
		else if (correct_values==1)
			testpntpass("Got the correct values");
		else 
			testpntfail("This should not happen");

		traceparser_destroy(&tp_state);
	 	testpntend();
		/***********************************************************************/
	
		/***********************************************************************/
		/*
		 * Make sure that if we trigger a thread state, that it gets logged
		 * properly (all the information in the tracelogger is correct)
		 * This tests the information provided in fast mode.
		 */
		snprintf(message, sizeof(message),"%s in fast mode", state_str[cur_state]);
	 	testpntbegin(message);

		exp_thread=cur_thread;
			
		/* We need to start up the tracelogger in daemon mode, 1 itteration.
		 * we will filter out everything other then thread states, then 
		 * start logging. 
		 * We then will trigger a thread state change, and flush the trace buffer. 
		 * This  should create a VERY minimal trace buffer that will be easily parsed
		 */
		tlpid=start_logger();
		sleep(1);
		/* Set the logger to fast emmiting mode */
		rc=TraceEvent(_NTO_TRACE_SETALLCLASSESFAST);
		assert(rc!=-1);
		fast_mode=FAST;
		/* Add the given thread event in the thread class back in */
		rc=TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_THREAD,(1<<cur_state));
		assert(rc!=-1);
		/* Filter out all tids other then the state_thread */
		if  (cur_state==STATE_STOPPED) {
			rc=TraceEvent(_NTO_TRACE_SETCLASSTID, _NTO_TRACE_THREAD, child_pid, 1);
		} else {
			rc=TraceEvent(_NTO_TRACE_SETCLASSTID, _NTO_TRACE_THREAD, getpid(),cur_thread);
		}
		assert(rc!=-1);

		/* then trigger an event.  Logging is started inside the state thread
		 * right before it tries to trigger the given state.
		 * the two barrier waits are to 
		 * 1) Tell the state thread that everything is setup so it should trigger the event
		 * 2) Wait for the state thread to tell us it has finished.
		 */
		pthread_barrier_wait(&global_barrier);

		if ((cur_state==STATE_SEND)|| (cur_state==STATE_REPLY)) {
			/* If the thread is going into the send state, we should receive and reply */
			sleep(1);
			status=MsgReceive(chid, message, sizeof(message), NULL);
			MsgReply(status, EOK, "ok", 3);
		} else if (cur_state==STATE_RECEIVE) {
			/* If the thread is going to call reveive, we should send it a message */
			sleep(1);
			status=ConnectAttach(0, getpid(), chid, _NTO_SIDE_CHANNEL, 0);
			MsgSend(status, message, 10, message,10);
		} else if (cur_state==STATE_MUTEX) {
			pthread_mutex_lock(&mymutex);
			sleep(2);
			pthread_mutex_unlock(&mymutex);
		} else if (cur_state==STATE_CONDVAR) {
			sleep(2);
			pthread_cond_signal(&mycondvar);
		} else if (cur_state==STATE_SEM) {
			sleep(2);
			sem_post(&mysem);
		}
		
		/* If the state thread is going to try to trigger the dead state, it will have to
		 * exit, so we can not expect it to call barrier_wait to tell us it's done. 
		 */
		if ((1<<cur_state)!=_NTO_TRACE_THDEAD) {
			pthread_barrier_wait(&global_barrier);
		} else {
			/* If the state thread is going to exit, then we should just
 			 * give it time to exit, then restart it.
			 */
			sleep(2); 
			rc=pthread_join(cur_thread, (void **)&status);
			assert(rc==EOK);
			rc=pthread_create(&cur_thread, NULL, state_thread, NULL);
		}
		delay(100);
		
		/* flush the trace buffer and wait for the tracelogger to exit*/
		rc=TraceEvent(_NTO_TRACE_FLUSHBUFFER);	
		assert(rc!=-1);
		rc=waitpid(tlpid, &status, 0);
		assert(tlpid==rc);
	
		/* Now, setup the traceparser lib to pull out the thread state events, 
		 * and make sure our event shows up 
		 */
		tp_state=traceparser_init(NULL);
		assert(tp_state!=NULL);
		traceparser_cs(tp_state, NULL, parse_cb, _NTO_TRACE_THREAD, (1<<cur_state));
	
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
			testpntfail("Got the wrong thread state");
		else if (correct_values==-2) 
			testpntfail("Got the wrong pid");
		else if (correct_values==-3)
			testpntfail("Got the wrong tid");
		else if (correct_values==1)
			testpntpass("Got the correct values");
		else 
			testpntfail("This should not happen");

		traceparser_destroy(&tp_state);
	 	testpntend();
		/***********************************************************************/

	}
	/* If the tracelogger was running when we started, we should restart it again */
	if (tlkilled==1) 
		system("reopen /dev/null ; tracelogger -n 0 -f /dev/null &");
	/* Kill off the child we had forked earler */
	kill (child_pid, SIGKILL);
	teststop(argv[0]);
	return 0;
}

