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

#include <sys/procmgr.h>
#include "externs.h"
#include "procmgr_internal.h"

static struct procmgr_event_entry {
	struct procmgr_event_entry			*next;
	struct procmgr_event_entry			**prev;
	int							rcvid;
	unsigned					flags;
	struct sigevent				event;
}							*event_list;

static pthread_mutex_t				event_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * procmgr_event_del() deletes the event associated with <handle>
 *
 * If this is the last handle stored in the prp->events vector then the
 * vector is freed.
 *
 * This function assumes that the process <prp> is already locked.
 */
static int procmgr_event_del(PROCESS *prp, int handle) {
	struct procmgr_event_entry			*p;
	VECTOR								*events;

	if((p = vector_lookup( prp->events, handle ))) {
		vector_rem( prp->events, handle );

		pthread_mutex_lock(&event_mutex);
		LINK2_REM(p);
		pthread_mutex_unlock(&event_mutex);

		_sfree(p, sizeof *p);

		if (prp->events->nentries == prp->events->nfree) {
			events = prp->events;
			prp->events = NULL;
			vector_free(events);
			_sfree( events, sizeof(VECTOR));
		}
		return EOK;
	}
	return ESRCH; 
}

/*
 * Destroy all procmgr events associated with this process
 *
 * Assumes that the process locked.
 */
void procmgr_event_destroy(PROCESS *prp) {
	int handle;

	for ( handle = 0; prp->events != NULL; handle++ ) {
		(void)procmgr_event_del(prp, handle);
	}
}

/*
 * Deliver the sigevent associated with the event that matches <flags>
 */
void procmgr_trigger(unsigned flags) {
	struct procmgr_event_entry 			*p;
#ifdef VARIANT_gcov
	extern void __bb_exit_func(void);

	/* If we are compiled with the gcov option, override the sync event to
	 * dump the code coverage data.
	 */
	if ( flags & PROCMGR_EVENT_SYNC ) {
		__bb_exit_func();
	}
#endif

	pthread_mutex_lock(&event_mutex);
	for(p = event_list; p; p = p->next) {
		if(p->flags & flags) {
			(void)MsgDeliverEvent(p->rcvid, &p->event);
		}
	}
	pthread_mutex_unlock(&event_mutex);
}

/* Handle the _PROC_EVENT type.
 *
 * Earlier versions only supported the _PROC_NOTIFY_EVENT and _PROC_NOTIFY_TRIGGER
 * subtypes.
 *
 * We support _PROC_NOTIFY_EVENT by assuming a handle of 0 for that type.
 */
int procmgr_event(resmgr_context_t *ctp, proc_event_t *msg) {
	struct procmgr_event_entry 			*p;
	PROCESS								*prp;
	int									handle = 0;
	int                                 allocated = 0;
	proc_event_del_t					*dmsg;

	switch(msg->i.subtype) {
	case _PROC_EVENT_NOTIFY_ADD:
		/* start searching at handle 1 - 0 is reserved for the legacy
		 * _PROC_EVENT_NOTIFY
		 */
		handle = 1;
		/* Fallthrough */
	case _PROC_EVENT_NOTIFY:
		if(!(prp = proc_lock_pid(ctp->info.pid))) {
			return ESRCH;
		}

		if ( prp->events == NULL ) {
			prp->events = _scalloc(sizeof(VECTOR));
			if ( prp->events == NULL ) {
				return proc_error(ENOMEM, prp);
			}
		}

		if(handle == 0 && msg->i.flags == 0) {
			/* legacy behaviour - this was how events got deleted in the old
			 * system
			 */
			procmgr_event_del(prp, 0);
		} else {
			if ( handle == 0 ) {
				p = vector_lookup( prp->events, 0 );
			} else {
				p = NULL;
			}
			if ( p == NULL ) {
				if(!(p = _scalloc(sizeof *p))) {
					return proc_error(ENOMEM, prp);
				}
				handle = vector_add( prp->events, p, handle );
				if ( handle == -1 ) {
					_sfree(p, sizeof(*p));
					return proc_error(ENOMEM, prp);
				}
				allocated = 1;
			}

			/*
			 * Lock the mutex to prevent procmgr_trigger() from delivering
			 * the event before all the necessary fields have been updated.
			 */

			pthread_mutex_lock(&event_mutex);
			p->rcvid = ctp->rcvid;
			p->flags = msg->i.flags;
			p->event = msg->i.event;

			if (allocated != 0) {
				LINK2_BEG(event_list, p, struct procmgr_event_entry);
			}
			pthread_mutex_unlock(&event_mutex);
		}

		_RESMGR_STATUS(ctp, handle);
		return proc_error(EOK, prp);

	case _PROC_EVENT_NOTIFY_DEL:

		dmsg = (proc_event_del_t *)msg;

		if(!(prp = proc_lock_pid(ctp->info.pid))) {
			return ESRCH;
		}

		return proc_error(procmgr_event_del(prp, dmsg->i.id), prp);

		/* NOTREACHED */
		break;

	case _PROC_EVENT_TRIGGER:
		if(!proc_isaccess(0, &ctp->info)) {
			if(msg->i.flags & PROCMGR_EVENT_PRIVILEGED) {
				return EPERM;
			}
		}
		procmgr_trigger(msg->i.flags);
		return EOK;

	default:
		break;
	}
	return ENOSYS;
}

__SRCVERSION("procmgr_event.c $Rev: 198544 $");
