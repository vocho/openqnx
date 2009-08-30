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

#include "externs.h"

static pthread_attr_t			thread_attr;

static resmgr_attr_t			resmgr_attr = {
	RESMGR_FLAG_CROSS_ENDIAN,			/* Flags */
	10,									/* Max nparts */
	sizeof(union proc_msg_union)		/* Max message size required */
};
	

static thread_pool_attr_t		pool_attr = {
	NULL,								/* Handle */
	dispatch_block,						/* Block function */
	NULL,								/* Unblock func */
	dispatch_handler,					/* Handler function */
	dispatch_context_alloc,				/* Context allocation */
	dispatch_context_free,				/* Context free */
	&thread_attr,						/* Thread attribute struct */
	3,									/* low water mark */
	1,									/* increment */
	10,									/* high water mark */
	75,									/* maximum threads */
};
 
// Internal dispatch function to specify our own chid
void *_dispatch_create(int chid, unsigned flags);
// Internal dispatch function to specify receiving of pulses
dispatch_context_t *dispatch_block_receive_pulse(dispatch_context_t *ctp);

void message_init(void) {

	// Check if already initialized
	if(dpp != NULL) {
		return;
	}

	// allocate the system process main channel (MUST BE SYSMGR_CHID)
	if(ChannelCreate(_NTO_CHF_UNBLOCK | _NTO_CHF_DISCONNECT) != SYSMGR_CHID) {
		crash();
	}

	// allocate the process manager connection (MUST BE SYSMGR_COID)
	if(ConnectAttach(0, SYSMGR_PID, SYSMGR_CHID, SYSMGR_COID, 0) != SYSMGR_COID) {
		crash();
	}

	if((dpp = _dispatch_create(SYSMGR_CHID, DISPATCH_FLAG_NOLOCK)) == NULL) {
		crash();
	}

	// Init the resmgr sublayer with the right attributes
	if(resmgr_attach(dpp, &resmgr_attr, NULL, 0, 0, NULL, NULL, NULL) == -1) {
		crash();
	}
}

static thread_pool_t		*tpp;
int message_start(void) {

	message_init();

	// Init proc thread attributes
	(void)pthread_attr_init(&thread_attr);
	(void)pthread_attr_setstacksize(&thread_attr, __PAGESIZE*2);

	pool_attr.handle = (void *)dpp;

	if((tpp = thread_pool_create(&pool_attr, POOL_FLAG_EXIT_SELF | POOL_FLAG_RESERVE)) == NULL) {
		crash();
	}

	// Never returns
	(void)thread_pool_start(tpp);
	return 0; 
}


/*
 Control the thread pool by adding some extra threads when
 an operation is going to be performed that may block and
 then decrementing that thread count when required.
*/
int _thread_pool_reserve(thread_pool_t *pool, int wait_count);

/* 
 Called before/after any blocking operation to reserve/
 create an extra operating thread.  

 If wait_count >= 0 and there are less than wait_count threads
 waiting in the thread pool, then we create a new thread
 and wait for that thread to be going before we continue.

 If wait_count < 0 then the high water mark is decremented.

 We always adjust the high water mark to allow us to 
 maintain a simplicity in the functionality.

 Returns 0 on success -1 on failure
 
 IMPORTANT:
 if you add any new calls to proc_thread_pool_reserve(), make sure you turn on
 the CATCH_NONPROC_RESV compile flag and set 'catch_nonproc_resv' in the
 debugger to be sure you know if you added into the call path of the loader.
 If so, go talk to Brian before checking in and update the list of known
 loader called proc_thread_pool_reserve()'s below as required

	List of loader call paths that we know flow through proc_thread_pool_reserve()
		pathmgr_node2fullpath()
		_netmgr_connect()
 
*/
//#define CATCH_NONPROC_RESV
#ifdef	CATCH_NONPROC_RESV
unsigned catch_nonproc_resv = 0;	// some run time control from the kernel debugger
#endif	/* CATCH_NONPROC_RESV */

static int thread_pool_reserve(thread_pool_t *pool, int wait_count) {
	if (getpid() != PROCMGR_PID) {
#ifdef	CATCH_NONPROC_RESV
		if (catch_nonproc_resv) {
			DebugBreak();
		}
#endif	/* CATCH_NONPROC_RESV */
		return 0;
	} else {
		return(_thread_pool_reserve(pool, wait_count));
	}
}

int
proc_thread_pool_reserve() {
	return thread_pool_reserve(tpp, 1); 
}

int
proc_thread_pool_reserve_done() {
	return thread_pool_reserve(tpp, -1);
}


__SRCVERSION("message.c $Rev: 200672 $");
