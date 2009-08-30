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




#include <stdlib.h>
#include <errno.h>
#include <sys/resmgr.h>
#include <atomic.h>
#include "resmgr.h"
#include <string.h>
#include <stdio.h>
#ifndef NDEBUG
#include <assert.h>
#endif


static int ebadf_reply(resmgr_context_t *ctp, void *msg, RESMGR_OCB_T *ocb) {
	return EBADF;
}

//TODO: Should we fill in the entire table or just a select few?
static const resmgr_io_funcs_t ebadf_io_funcs = {
        _RESMGR_IO_NFUNCS,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        NULL,                 /* Close ocb */
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        NULL,                 /* Close dup */
        (void *)ebadf_reply,
        (void *)ebadf_reply,
        (void *)ebadf_reply,
};

#if 1
/*
 This is an ugly way to do the detach since it requires that we know both
 about the locks and size of the table managed by _resmgr_handle() as well
 as locking the entire data structure for a longer period of time.  It is
 a cheaper solution however then the second block of code, which would require
 additional changes to the _resmgr_handle() code to not shift the found 
 bindings around to the head of the list.

 It still would be better to have this entire operation encapsulated inside 
 of _resmgr_handle() since both implementations rely on _resmgr_io_table.nentries 
 being known, at least the code below is honest about breaking the encapsulation.
 This code is _almost_ the same as the code in resmgr_pathname_detach().
*/
int _resmgr_detach_id(resmgr_context_t *ctp, int id, unsigned flags) {
	struct binding					*binding;
	struct binding					*current_binding;
	struct _resmgr_handle_list		*list;
	struct _resmgr_handle_entry		*entry, *nentry;
	int found = 0;
	enum _resmgr_handle_type type = _RESMGR_HANDLE_FIND_LOCK;
	_resmgr_func_t func;
	resmgr_iomsgs_t *saved_msgs;
	struct _msg_info saved_info;
	int saved_rcvid;
	const resmgr_io_funcs_t *saved_funcs = NULL;
	io_close_t close;
	unsigned						scoid;
	/*
	 * Something on this thread's stack.
	 * Should be unique for this thread to mark paths (via coids)
	 * to bindings it already has locked.  Can't use something
	 * like NULL in case 2 threads doing resmgr_detach() at same time.
	 */
	int marker;
	

	saved_msgs   = ctp->msg;
	saved_rcvid  = ctp->rcvid;
	memcpy(&saved_info, &ctp->info, sizeof ctp->info);

	/* Set the following 2 for close callout */
	ctp->rcvid = -1; /* In case they try to reply to someone */
	ctp->msg_max_size =
	ctp->info.msglen = sizeof close;

	close.i.type = _IO_CLOSE;
	close.i.combine_len = sizeof close.i;

	ctp->msg     = (resmgr_iomsgs_t *)&close;

	current_binding = NULL;
	found = 0;

	_mutex_lock(&_resmgr_io_table.mutex);
loop_again:

	for (list = _resmgr_io_table.vector, scoid = 0; 
		 scoid < _resmgr_io_table.nentries; 
		 list++, scoid++) {
		ctp->info.scoid = scoid | _NTO_SIDE_CHANNEL;
		for (entry = list->list; entry != NULL; entry = nentry) {
			nentry = entry->next;


			binding = (struct binding *)entry->handle;

			if(type == _RESMGR_HANDLE_FIND_LOCK) {
				if(entry->handle == &marker || binding->funcs == &ebadf_io_funcs) {
					/*
					 * Either we've done the locking phase on this binding (first
					 * test), or we've done both lock, unlock phases (second test).
					 */
					continue;
				}
			}
			else {
				if(entry->handle != &marker) {
					/* We've already unlocked this one (or never had it locked) */
					continue;
				}
				/* restore */
				entry->handle = current_binding;
				binding       = current_binding;
			}


			if((!current_binding && binding->id == id) || binding == current_binding) {

				ctp->info.coid = entry->coid & ~_RESMGR_HANDLE_LOCK;
				ctp->info.pid = list->pid;
				ctp->info.nd = list->nd;
				ctp->info.tid = 0;
				_mutex_unlock(&_resmgr_io_table.mutex);

				binding = _resmgr_handle(&ctp->info, NULL, type);

				if(type == _RESMGR_HANDLE_FIND_LOCK) {
					if(binding != (void *) -1) {
						/* Good idea to recheck in case something changed in the interim */
						if(binding->id != id || binding->funcs == &ebadf_io_funcs || (current_binding != NULL && binding != current_binding)) {
							_resmgr_handle(&ctp->info, NULL, _RESMGR_HANDLE_UNLOCK);
						}
						else {
							current_binding = binding;
							found++;
							entry->handle = &marker;
						}
					}
				}
				else {
					found--;
					if(flags & _RESMGR_DETACH_CLOSE) {
						/* Do close callout(s) */

						if((func = _resmgr_io_func(saved_funcs, _IO_CLOSE))) {
							(void)func(ctp, ctp->msg, binding->ocb);
						}

						/*
						 * The CLOSE_OCB callout is bassed on found, not binding->count
						 * because we always want it to be called if present.  The last
						 * decrement of binding->count may happen on real _IO_CLOSE message
						 * handling.  By that time, function table is already overwritten.
						 */
						if(found == 0 && (func = _resmgr_io_func(saved_funcs, _IO_RSVD_CLOSE_OCB))) {
							(void)func(ctp, 0, binding->ocb);
						}
					}

					/*	we need to remove the binding (not just decrement the ref)
						from the list as a mount request could be inflight
						with a stale ocb handle.  This way the _resmgr_mount_handler code will fail its
						resmgr_bind check.
					*/
					if(_resmgr_handle(&ctp->info, binding, _RESMGR_HANDLE_REMOVE_LOCK) != (void*)-1
						&& (atomic_sub_value(&binding->count, 1) == 1)) {
						free(binding);
					}
					
				}
				_mutex_lock(&_resmgr_io_table.mutex);
			}
		}
	}

	if(type == _RESMGR_HANDLE_UNLOCK) {
		current_binding = NULL;
#ifndef NDEBUG
		assert(found == 0);
#endif
		found = 0;
		type = _RESMGR_HANDLE_FIND_LOCK;
		goto loop_again;
	}
	else if(found) {
		/* Atomic in case multiple resmgr_detach() happening on same id */
		atomic_add(&current_binding->count, found);

		/*
		 * We're about to do the unlocking so overwrite the binding's funcs.
		 * Everyone now gets EBADF on whatever (even IO_CLOSE for which the
		 * callouts will be done just after the unlock).
		 */
		saved_funcs = current_binding->funcs;
		current_binding->funcs = &ebadf_io_funcs;
		type = _RESMGR_HANDLE_UNLOCK;
		goto loop_again;
	}

	_mutex_unlock(&_resmgr_io_table.mutex);

	ctp->msg     = saved_msgs;
	ctp->rcvid   = saved_rcvid;
	memcpy(&ctp->info, &saved_info, sizeof ctp->info);

	return 0;
}

#else

/*
 In order for _resmgr_detach_id() to work properly, the _resmgr_handle() code
 has to be modified to not move the link objects around when they are 
 found, otherwise looping through the objects without knowing the internals
 becomes next to impossible.  
 
 This code encapsulates the table a little bit better than the previous i
 _resmgr_detach_id() code, but still has knowledge about the table through 
 the _resmgr_io_table.nentries.  Since it requires more work (and changes) 
 without gaining any real additional elegance we will stick with the original 
 detach code until _resmgr_handle() can support this operation internally.

#define _RESMGR_HANDLE_NOMOVE	(int)(((~0u ^ (~0u >> 1)) >> 1) >> 1)
*/

int _resmgr_detach_id(resmgr_context_t *ctp, int id, unsigned flags) {
	struct binding					*binding, *first;
	int								scoid, done;

	// Loop until we cannot detach anything more. We have to do multiple
	// passes, as someone else may come in and change the order of the lists.
	do {
		done = 0;
		for(scoid = 0; scoid < _resmgr_io_table.nentries; scoid++) {
			memset(&ctp->info, 0, sizeof ctp->info);
			first = NULL;
			ctp->info.scoid = scoid | _NTO_SIDE_CHANNEL;
			ctp->info.coid = -1;
			while((binding = _resmgr_handle(&ctp->info, NULL, 
								(ctp->info.coid != -1) ? (_RESMGR_HANDLE_FIND_LOCK | _RESMGR_HANDLE_NOMOVE) 
								                       : (_RESMGR_HANDLE_DISCONNECT_LOCK))) != (void *)-1) {
				if(binding->id == id && binding->funcs != &ebadf_io_funcs) {
					/* We don't want to remove this item, we just want to call out on it's
                       installed close handler if required and then switch the io function 
					   callout table to the internal EBADF callouts.

					   Doing this rather than just removing the entry means that there is no
					   need to have any "magic" checks or returns scattered elsewhere in the 
					   code, we can control all of the errors from the above callouts. 
					*/
					if(flags & _RESMGR_DETACH_CLOSE) {
						_resmgr_close_handler(ctp, binding);
					}

					/* Replace the function handler after the close call has been made */
					binding->funcs = &ebadf_io_funcs;

					done++;
				} 

				/* Unlock this handle, and fill in the information for the next query */
				_resmgr_handle(&ctp->info, NULL, _RESMGR_HANDLE_UNLOCK | _RESMGR_HANDLE_NEXT);
				
				if(first == binding) {		// We've gone through all bindings ?!
					break;
				} else if (first == NULL) {
					first = binding;
				}
			}
		}
	} while(done);

	return 0;
}

#endif


__SRCVERSION("_resmgr_detach_id.c $Rev: 153052 $");
