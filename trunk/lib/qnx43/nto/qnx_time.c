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



#include <errno.h>
#include <time.h>

int getclock(clock_type, tp)
int clock_type;
register struct timespec *tp;
    {
    clock_type = clock_type;		/* shut up watcom's compiler */

//    return(qnx_getclock(0, clock_type, tp));
	  return(clock_gettime(CLOCK_REALTIME,tp));
    }

#if 0

int qnx_getclock(proc_pid, clock_type, tp)
pid_t proc_pid;
int clock_type;
register struct timespec *tp;
    {
    union {
	struct _proc_time	    s;
	struct _proc_time_reply     r;
	} msg;

    clock_type = clock_type;		/* shut up watcom's compiler */

    msg.s.type = _PROC_TIME;
    msg.s.seconds = -1;
    msg.s.nsec = -1;
    msg.s.zero1 = 0;
    if( Send( proc_pid ? proc_pid : PROC_PID,
		&msg.s, &msg.r, sizeof( msg.s ), sizeof( msg.r ) ) == -1 )
	return( -1 );

    tp->tv_sec	= msg.s.seconds;
    tp->tv_nsec = msg.s.nsec;

    if( msg.r.status != EOK ) {
	errno = msg.r.status;
	return( -1 );
	}

    return( 0 );
    }


int setclock(clock_type, tp)
int clock_type;
register struct timespec *tp;
    {
    clock_type = clock_type;		/* shut up watcom's compiler */

    return(qnx_setclock(0, clock_type, tp));
    }

int qnx_setclock(proc_pid, clock_type, tp)
pid_t proc_pid;
int clock_type;
register struct timespec *tp;
    {
    union {
	struct _proc_time	    s;
	struct _proc_time_reply     r;
	} msg;

    clock_type = clock_type;		/* shut up watcom's compiler */

    msg.s.type = _PROC_TIME;
    msg.s.seconds = tp->tv_sec;
    msg.s.nsec = tp->tv_nsec;
    msg.s.zero1 = 0;
    if( Send( proc_pid ? proc_pid : PROC_PID,
		&msg.s, &msg.r, sizeof( msg.s ), sizeof( msg.r ) ) == -1 )
	return( -1 );

    if( msg.r.status != EOK ) {
	errno = msg.r.status;
	return( -1 );
	}

    return( 0 );
    }

#endif

