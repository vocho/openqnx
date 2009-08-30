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
*	File:		bk1_comm.c
*
******************************************************************************
*
*   Contents:	Tests to make sure the instrumentation properly intercepts
*				events in the communication class.
*
*	Date:		Nov. 20, 2001
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
 *									GLOBALS 								*
 *--------------------------------------------------------------------------*/
/* This is a global used by the traceparser callback function to  
 * tell the main thread that the values it got in the events were
 * correct 
 */
static int correct_values;

/* 
 * This is used to tell the state thread which state to try to get into.
 */
unsigned  cur_comm;

/* 
 * This is the thread id of the state thread. This is what we will expect to 
 * get in the traceparser callback.
 */
pthread_t exp_thread;


/* This is the channel that will be setup in main, and used by trigger_event
 * to trigger the various message related kernel calls.
 */
int chid;

/* This is a list of strings to give names to the various comm events */
char * comm_str[] = {
	"SENDMSG", 
	"SENDPULSE", 
	"RECMSG", 
	"RECPULSE", 
	"SPULSEEXE",
	"SPULSEDIS",
	"SPULSEDEA",
	"SPULSEUN",
	"SPULSEQUN"
};

/* This array is filled in by the message thread with the info that
 * should show up in the tracebuffer
 */
int results[3];

struct sigevent myevent; /* This is the event that will be returned by the 
						  * isr */
 
/*--------------------------------------------------------------------------*
 *									ROUTINES								*
 *--------------------------------------------------------------------------*/
/****************************************************************************
*
*					Subroutine isr_handler
*
*****************************************************************************
*
*   Purpose:	This is a simple isr that will just return a pulse event
*
****************************************************************************/
const struct sigevent * isr_handler (void *arg, int id)
{
	/* We just want to return */
	return(&myevent);
}
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
	int rcvid,rcvid2,rc;
	char buf[100];
	struct _msg_info info;
	struct _pulse * pulse;
	while(1) {
		rcvid=MsgReceive(chid, buf,sizeof(buf), &info);
		// pulses
		if (rcvid==0) {
			if (cur_comm==_NTO_TRACE_COMM_SPULSE_DIS) {
				pulse=(struct _pulse *)buf;
				ConnectDetach(pulse->scoid);
			} 

			continue;
		}
		results[0]=rcvid;
		results[1]=info.pid;
		if (buf[0]=='B') {
			/* If we should block, just don't reply right away.. */
			rcvid2=rcvid;
			/* and wait for the unblock pulse to get it out of the way.. */
			rcvid=MsgReceive(chid, buf,sizeof(buf), &info);
			if (rcvid!=0) {
				/* If we get a message when expecting a pulse, this should
				 * be the done message, so just jump down there.. 
				 */
				goto msg;
			}
			results[0]=info.scoid|_NTO_SIDE_CHANNEL;
			results[1]=info.pid;
			rc=MsgReply(rcvid2, EOK,  "Ok", 3);
			if (cur_comm==_NTO_TRACE_COMM_SPULSE_DIS) {
				pulse=(struct _pulse *)buf;
				ConnectDetach(pulse->scoid);
			}
			continue;
		}
msg:
		rc=MsgReply(rcvid, EOK,  "Ok", 3);
		if (buf[0]=='D')  {
			pthread_exit(0);
		}
	}
	
}

/****************************************************************************
*
*						Subroutine parse_cb
*
*	Purpose: 	This is a traceparcer callback for all events. It will check 
*				the given event length, and parameter values against those
*				that are currenly pointed to in the global cur_comm
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
	char buf[100];

	/* This is to handle the case when we may see multiple events in the 
	 * trace logger.  If we have seen the correct event, then we can just
	 * ignore this one.  
	 */
	if (correct_values==1) 
		return(EOK);
	/*
	 * Now check that the values that we recieved in the callback are the expected values.
	 */
	if ((event_p[0]==results[0]) && (event_p[1]==results[1]))  
		correct_values=1;
	else  {
		snprintf(buf,sizeof(buf), "Expected: %x and %d  Got: %x and %d", 
			results[0], results[1], event_p[0], event_p[1]);
		testnote(buf);
		correct_values=-1;
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
	rc=TraceEvent(_NTO_TRACE_CLRCLASSPID, _NTO_TRACE_COMM);
	assert(rc!=-1);
	rc=TraceEvent(_NTO_TRACE_CLRCLASSTID, _NTO_TRACE_COMM);
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
	int iid,tlkilled,tlpid, rc,status,coid;  /*tlpid=trace logger pid */
	struct traceparser_state * tp_state;
	char message[200];
	pthread_t cur_thread;
	uint64_t timeout;
	struct _server_info info;

	/*
	 * Start the tests.
	 */
	teststart(argv[0]);
	/* Get rid of tracelogger if it is running  */
	tlkilled=kill_tl();

	
	for (cur_comm=_NTO_TRACE_COMM_SMSG;cur_comm<_NTO_TRACE_COMM_SPULSE_QUN;cur_comm++) {
		/***********************************************************************/

		/***********************************************************************/
		/*
		 * Make sure that if we trigger a communication event, that it gets
	 	 * properly logged
		 */
		snprintf(message, sizeof(message),"ADDEVENT %s", comm_str[cur_comm]);
	 	testpntbegin(message);
		if (cur_comm==_NTO_TRACE_COMM_RPULSE) {
			testpntsetupxfail("9752");
		}
		exp_thread=cur_thread;
			
		/* We need to start up the tracelogger in daemon mode, 1 itteration.
		 * we will filter out everything other then comm events, then 
		 * start logging. 
		 * We then will trigger a communication event, and flush the trace buffer. 
		 * This  should create a VERY minimal trace buffer that will be easily parsed
		 */
		tlpid=start_logger();
		sleep(1);
		/* Set the logger to wide emmiting mode */
		rc=TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE);
		assert(rc!=-1);
		/* Add the given communication event in the communication class back in */
		rc=TraceEvent(_NTO_TRACE_ADDEVENT, _NTO_TRACE_COMM,cur_comm);
		assert(rc!=-1);
		/* Filter out all pids other then us */
		rc=TraceEvent(_NTO_TRACE_SETCLASSPID, _NTO_TRACE_COMM, getpid());
		assert(rc!=-1);

		rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
		assert(rc!=-1);
		/* then trigger an event. 
		 */
		/* We always need a chid */
		switch (cur_comm) {
			case _NTO_TRACE_COMM_SPULSE_DIS:
				chid=ChannelCreate(_NTO_CHF_DISCONNECT);
				break;
			case _NTO_TRACE_COMM_SPULSE_UN:
				chid=ChannelCreate(_NTO_CHF_UNBLOCK);
				break;
			case _NTO_TRACE_COMM_SPULSE_DEA:
				chid=ChannelCreate(_NTO_CHF_COID_DISCONNECT);
				break;
			default:
				chid=ChannelCreate(0);
		}
		assert(chid!=-1);
		/* and create a thread to send and receive various messages/pulses. */
		rc=pthread_create(&cur_thread, NULL, msg_thread, NULL);
		assert(rc==EOK);
		coid = ConnectAttach(0, getpid(), chid, _NTO_SIDE_CHANNEL, 0);
		assert(coid!=-1);
		switch (cur_comm) {
			case _NTO_TRACE_COMM_RMSG:
			case _NTO_TRACE_COMM_SMSG:
				/* We trigger a SMSG and a RMSG the same way */
				MsgSend(coid, "Message", 8, message, sizeof(message));
				break;
			case _NTO_TRACE_COMM_RPULSE:
			case _NTO_TRACE_COMM_SPULSE:
				/* We trigger a SPULSE and a RPULSE the same way */
				assert(coid!=-1);
				/* in the case of a pulse, we have to pull out the scoid here */
				ConnectServerInfo(getpid(), coid, &info);
				results[0]=info.scoid;
				results[1]=info.pid;
				
				MsgSendPulse(coid, 10, -11, 12);
				break;
			case _NTO_TRACE_COMM_SPULSE_EXE:
				/* To get an exe, we need to have an interrupt handler 
				 * return a pulse 
	 			 */
				/* in the case of a pulse, we have to pull out the scoid here */
				ConnectServerInfo(getpid(), coid, &info);
				results[0]=info.scoid;
				results[1]=info.pid;
				memset(&myevent, 0, sizeof(myevent));
				SIGEV_PULSE_INIT( &myevent, coid, 10, -11, 0 );

				/*Install our interrupt handler */
				/* Get io privity */
				ThreadCtl( _NTO_TCTL_IO, 0 );
	 
				/* Now, add our interrupt handler. 
				 */
				iid=InterruptAttach(SYSPAGE_ENTRY(qtime)->intr, isr_handler,NULL, 0, 0);
				assert(iid!=-1);
				
				/* Wait for some interrupts to happen */
				delay(300);

				/* and when we are done, remove the interrupt handler */
				InterruptDetach(iid);
				break;
			case _NTO_TRACE_COMM_SPULSE_DIS:
				/* in the case of a pulse, we have to pull out the scoid here */
				ConnectServerInfo(getpid(), coid, &info);
				results[0]=info.scoid;
				results[1]=info.pid;
				/* For the disconnect pulse, just disconnect from the channel */
				ConnectDetach(coid);
				delay(100); 
				break;
			case _NTO_TRACE_COMM_SPULSE_DEA:
				ConnectServerInfo(getpid(), coid, &info);
				results[0]=-1;
				results[1]=info.pid;
				rc=ChannelCreate(0);
				ConnectAttach(0, getpid(), rc, _NTO_SIDE_CHANNEL, 0);
				ChannelDestroy(rc);
				break;
			case _NTO_TRACE_COMM_SPULSE_UN:
				/* in the case of a pulse, we have to pull out the scoid here */
				ConnectServerInfo(getpid(), coid, &info);
				/* We trigger a SMSG and a RMSG the same way */
				myevent.sigev_notify = SIGEV_UNBLOCK;
				timeout = 1*100000000;

				TimerTimeout( CLOCK_REALTIME, _NTO_TIMEOUT_SEND | _NTO_TIMEOUT_REPLY,
					&myevent, &timeout, NULL );


				MsgSend(coid, "Block", 8, message, sizeof(message));
				sleep(1);
				results[0]=info.scoid|_NTO_SIDE_CHANNEL;
				results[1]=info.pid;
				break;

				
		}
		/* Give stuff a chance to sync up */
		sleep(1);
		
		/* flush the trace buffer and wait for the tracelogger to exit*/
		rc=TraceEvent(_NTO_TRACE_FLUSHBUFFER);	
		rc=waitpid(tlpid, &status, 0);
		assert(tlpid==rc);
	
		/* Now, setup the traceparser lib to pull out the comm events, 
		 * and make sure our event shows up 
		 */
		tp_state=traceparser_init(NULL);
		assert(tp_state!=NULL);
		traceparser_cs(tp_state, NULL, parse_cb, _NTO_TRACE_COMM, cur_comm);
	
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
			testpntfail("Got the wrong comm events");
		else if (correct_values==1)
			testpntpass("Got the correct values");
		else 
			testpntfail("This should not happen");

		/* Clean up after ourselves */
		traceparser_destroy(&tp_state);
	 	testpntend();
		ConnectDetach(coid);
		/***********************************************************************/
		/* This is similar to above, but we will all the whole comm class, not just
		 * an event.  This should make no difference, but there are 2 paths for the
		 * the addition, and there have been some problems with the 2 paths 
		 *
		 */
		/***********************************************************************/
	
		/***********************************************************************/
		/*
		 * Make sure that if we trigger a communication event, that it gets
	 	 * properly logged
		 */
		snprintf(message, sizeof(message),"ADDCLASS %s", comm_str[cur_comm]);
	 	testpntbegin(message);

		exp_thread=cur_thread;
			
		/* We need to start up the tracelogger in daemon mode, 1 itteration.
		 * we will filter out everything other then comm events, then 
		 * start logging. 
		 * We then will trigger a communication event, and flush the trace buffer. 
		 * This  should create a VERY minimal trace buffer that will be easily parsed
		 */
		tlpid=start_logger();
		sleep(1);
		/* Set the logger to wide emmiting mode */
		rc=TraceEvent(_NTO_TRACE_SETALLCLASSESWIDE);
		assert(rc!=-1);
		/* Add the given communication event in the communication class back in */
		rc=TraceEvent(_NTO_TRACE_ADDCLASS, _NTO_TRACE_COMM);
		assert(rc!=-1);
		/* Filter out all pids other then us */
		rc=TraceEvent(_NTO_TRACE_SETCLASSPID, _NTO_TRACE_COMM, getpid());
		assert(rc!=-1);

		rc=TraceEvent(_NTO_TRACE_STARTNOSTATE);
		assert(rc!=-1);
		/* then trigger an event. 
		 */
		/* We always need a coid */
		coid = ConnectAttach(0, getpid(), chid, _NTO_SIDE_CHANNEL, 0);
		assert(coid!=-1);
		switch (cur_comm) {
			case _NTO_TRACE_COMM_RMSG:
			case _NTO_TRACE_COMM_SMSG:
				/* We trigger a SMSG and a RMSG the same way */
				MsgSend(coid, "Message", 8, message, sizeof(message));
				break;
			case _NTO_TRACE_COMM_RPULSE:
			case _NTO_TRACE_COMM_SPULSE:
				/* We trigger a SPULSE and a RPULSE the same way */
				assert(coid!=-1);
				MsgSendPulse(coid, 10, -11, 12);
				break;
			case _NTO_TRACE_COMM_SPULSE_EXE:
				/* To get an exe, we need to have an interrupt handler 
				 * return a pulse 
	 			 */
				memset(&myevent, 0, sizeof(myevent));
				SIGEV_PULSE_INIT( &myevent, coid, 10, -11, 0 );

				/*Install our interrupt handler */
				/* Get io privity */
				ThreadCtl( _NTO_TCTL_IO, 0 );
	 
				/* Now, add our interrupt handler. 
				 */
				iid=InterruptAttach(SYSPAGE_ENTRY(qtime)->intr, isr_handler,NULL, 0, 0);
				assert(iid!=-1);
				
				/* Wait for some interrupts to happen */
				delay(300);

				/* and when we are done, remove the interrupt handler */
				InterruptDetach(iid);
				break;
			case _NTO_TRACE_COMM_SPULSE_DIS:
				/* For the disconnect pulse, just disconnect from the channel */
				ConnectDetach(coid);
				delay(100);
				break;
			case _NTO_TRACE_COMM_SPULSE_DEA:
				ConnectServerInfo(getpid(), coid, &info);
				results[0]=-1;
				results[1]=info.pid;
				rc=ChannelCreate(0);
				ConnectAttach(0, getpid(), rc, _NTO_SIDE_CHANNEL, 0);
				ChannelDestroy(rc);
				break;
			case _NTO_TRACE_COMM_SPULSE_UN:
				/* in the case of a pulse, we have to pull out the scoid here */
				ConnectServerInfo(getpid(), coid, &info);
				/* We trigger a SMSG and a RMSG the same way */
				myevent.sigev_notify = SIGEV_UNBLOCK;
				timeout = 1*100000000;

				TimerTimeout( CLOCK_REALTIME, _NTO_TIMEOUT_SEND | _NTO_TIMEOUT_REPLY,
					&myevent, &timeout, NULL );


				MsgSend(coid, "Block", 8, message, sizeof(message));
				sleep(1);
				results[0]=info.scoid|_NTO_SIDE_CHANNEL;
				results[1]=info.pid;
				break;

				
		}
		/* Give stuff a chance to sync up */
		sleep(1);
		
		/* flush the trace buffer and wait for the tracelogger to exit*/
		rc=TraceEvent(_NTO_TRACE_FLUSHBUFFER);	
		rc=waitpid(tlpid, &status, 0);
		assert(tlpid==rc);
	
		/* Now, setup the traceparser lib to pull out the comm events, 
		 * and make sure our event shows up 
		 */
		tp_state=traceparser_init(NULL);
		assert(tp_state!=NULL);
		traceparser_cs(tp_state, NULL, parse_cb, _NTO_TRACE_COMM, cur_comm);
	
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
			testpntfail("Got the wrong comm events");
		else if (correct_values==1)
			testpntpass("Got the correct values");
		else 
			testpntfail("This should not happen");

		/* Clean up after ourselves */
		traceparser_destroy(&tp_state);
	 	testpntend();
		/***********************************************************************/
		/* Tell the msg_thread that we are done */
		MsgSend(coid, "Done", 5, message, sizeof(message));
		ConnectDetach(coid);

		/* And kill the channel */
		ChannelDestroy(chid);
	}
	/* If the tracelogger was running when we started, we should restart it again */
	if (tlkilled==1) 
		system("reopen /dev/null ; tracelogger -n 0 -f /dev/null &");
	teststop(argv[0]);
	return 0;
}

