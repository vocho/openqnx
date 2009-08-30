/*
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable
 * license fees to QNX Software Systems before you may reproduce,
 * modify or distribute this software, or any work that includes
 * all or part of this software.   Free development licenses are
 * available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email
 * licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review
 * this entire file for other proprietary rights or license notices,
 * as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <libgen.h>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <sys/siginfo.h>
#include <sys/stat.h>
#include <sys/syspage.h>
#include <unistd.h>
#include <time.h>
#include "syspage_time.h"

#define PULSE_CODE_PATHSPACE	_PULSE_CODE_MINAVAIL

int _waitfor( __const char *path, int delay_ms, int poll_ms, int (*checkfunc)(__const char *, void *), void *handle )
{
	int					notify_id = -1;
    struct sigevent     event;
    int                 chid = -1, coid = -1;
    struct _pulse       pulse;
	int					ret_val = -1;
	uint64_t			polltimeout = 100 * (uint64_t)1000000;
	uint64_t			timeout_nsec;

	timeout_nsec = _syspage_time(CLOCK_MONOTONIC);
	timeout_nsec += delay_ms * (uint64_t)1000000;

    /* Check to make sure we don't wait for something already there... */
    ret_val = checkfunc( path, handle );
	if ( ret_val >= 0 || ret_val == WAITFOR_CHECK_ABORT ) {
		return ret_val;
    }

	if ( poll_ms > 0 ) {
		polltimeout = poll_ms * (uint64_t)1000000;
	}

	/* 
	 * We make the channel fixed since we want to remain at the current
	 * thread's priority
	 */
    chid = ChannelCreate(_NTO_CHF_FIXED_PRIORITY);
	if ( chid == -1 ) {
		goto fail1;
	}
    coid = ConnectAttach( ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0 );
	if ( coid == -1 ) {
		goto fail2;
	}

    SIGEV_PULSE_INIT( &event, coid, SIGEV_PULSE_PRIO_INHERIT, PULSE_CODE_PATHSPACE, 0 );
    notify_id = procmgr_event_notify_add( PROCMGR_EVENT_PATHSPACE, &event );
	if ( notify_id == -1 ) {
		goto fail3;
	}

    while(_syspage_time(CLOCK_MONOTONIC) < timeout_nsec) {

		/* we need to poll in case we are checking for a subdirectory/file,
		 * since we won't get a pathmgr event for those.
		 */
		SIGEV_UNBLOCK_INIT(&event);
		if ( TimerTimeout( CLOCK_MONOTONIC, _NTO_TIMEOUT_RECEIVE, &event, &polltimeout, NULL) == -1 ) {
			ret_val = -1;
			break;
		}
	   	if ( MsgReceivePulse( chid, &pulse, sizeof(pulse), NULL ) != -1 || errno == ETIMEDOUT ) {
			ret_val = checkfunc( path, handle );
			if ( ret_val >= 0 || ret_val == WAITFOR_CHECK_ABORT ) {
				break;
            }
		} else {
			break;
		}
    }

	(void)procmgr_event_notify_delete( notify_id );
fail3:
	(void)ConnectDetach(coid);
fail2:
	(void)ChannelDestroy(chid);
fail1:
    return ret_val;
}
