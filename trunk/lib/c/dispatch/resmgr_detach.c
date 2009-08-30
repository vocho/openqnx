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
#include <atomic.h>
#include <malloc.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/iomsg.h>
#include <sys/resmgr.h>
#include <sys/pathmgr.h>
#include <sys/dispatch.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "dispatch.h"
#include "resmgr.h"


/* TODO: Big re-write of this function to make it block until 
   we are all disconnected ...
*/
int resmgr_detach(dispatch_t *dpp, int id, unsigned flags) {
	resmgr_context_t				ctp;
	struct link						*linkl, *callerlink;
	int								 lastcount;

	//We don't want to increment our link count here ...
	linkl = _resmgr_link_query(id, 0);
	if((linkl == NULL) || (linkl->flags & _RESMGR_LINK_HALFOPEN)) {
		// The _RESMMGR_LINK_HALFOPEN check is to prevent a detach
		// from happening at the same time that a resmgr_attach() is
		// still in progress. We probably don't need both this bit
		// _and_ the _RESMGR_LINK_DETACHWAIT one - they can both use the
		// same one.
		errno = ENOENT;
		return -1;
	}

	/*
	 Just turn off the RESMGR_LINK_OTHERFUNC flag on this connection, 
	 but don't invalidate the dpp function pointer since there
	 might be other mountpoints sharing this dpp.
	*/
	if(linkl->flags & _RESMGR_LINK_OTHERFUNC) {
		linkl->flags &= ~_RESMGR_LINK_OTHERFUNC;
	}

	/*
	 Rip the pathname out of the space so that no further connections 
	 are forwarded from proc.  There are still possibly connections 
	 "in flight" se we don't rip this request out since the id might 
	 get accidentally re-used by some other _resmgr_link_alloc().
	*/
	if(_resmgr_link_free(id, _RESMGR_DETACH_PATHNAME) == -1) {
		errno = ENOENT;
		return -1;
	}

	//If all we wanted to do was detach the pathname ... then so be it
	if(flags & _RESMGR_DETACH_PATHNAME) {
		return 0;
	}

	/*
	 Setting this flag will prevent any further requests to 
	 resmgr_link_query() to fail on this id. There may be 
	 requests who are already in the connect() functions which 
	 will be holding a valid link handle pointer.

	 We need to atomically test and set this flag since we want 
	 to make sure that there is only one of us who is doing the 
	 detach at a time.  If we set the flag and find it is already 
	 set then we bail with ENOENT just as if we didn't get the 
	 link value.  This lets the previous guy handle the detach.
	*/
	if((atomic_set_value(&linkl->flags, _RESMGR_LINK_DETACHWAIT)) & _RESMGR_LINK_DETACHWAIT) {
		errno = ENOENT;
		return -1;
	}

	/* 
	 We do a useless lookup on the id in order to prevent a race 
	 between this code and the _resmgr_link_query() code which 
	 actually does the count increment.  This is also a sanity test.
	*/
	if(_resmgr_link_query(id, 0) != NULL) {
#ifndef NDEBUG
		errno = EINVAL;
		return -1;
#endif
	}

	/*
	 We wait for the system to go "quiet" in that all of the 
	 people who were using the link structure are no longer 
	 using it. Don't count ourselves if we were in a connect
	 function.
	*/
	callerlink = pthread_getspecific(_resmgr_thread_key);
	lastcount = (callerlink == linkl) ? 1 : 0; 

	//TODO: Use a dedicated sleepon. Fix _resmgr_link_return
	pthread_sleepon_lock();
	while (linkl->count > lastcount) {
		pthread_sleepon_wait(linkl);
	}
	pthread_sleepon_unlock();

	/* Invalidate the link so it won't be used again by accident */
	if (lastcount) {
		pthread_setspecific(_resmgr_thread_key, NULL);
	}

	memset(&ctp, 0, sizeof(ctp)); 
	(void)_resmgr_detach_id(&ctp, id, flags & _RESMGR_DETACH_CLOSE);

	/*
	 This will actually rip the link entry out and free it.
	 There should be no active requests in the system at this
	 point and the id will get re-used by some other attach.
	*/
	if(_resmgr_link_free(id, 0) == -1) {
		return -1;
	}

	return 0;
}


__SRCVERSION("resmgr_detach.c $Rev: 162224 $");
