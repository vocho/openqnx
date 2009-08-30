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

/*
 Handle all other miscellaneous messages
*/
#include <errno.h>
#include "rsrcdbmgr.h"
#include <sys/resmgr.h>

/*
 We need to map the old style enumerated entries to new string 
 based entries.
*/
static const char *rsrc_names[] = { "memory", "irq", "io", "dma", /*"memory/pci"*/ "pcimemory", "unknown" }; 
static char *
rsrcdbmgr_id2name(int id) {
	id = id & RSRCDBMGR_TYPE_MASK;
	if(id >= RSRCDBMGR_RESERVED_5) {
		id = RSRCDBMGR_RESERVED_5;
	}
	return (char *)rsrc_names[id];
}


static int
do_create_destroy(int (*getfunc)(rsrc_alloc_t *alloc, pid_t pid),
				int (*restorefunc)(rsrc_alloc_t *alloc, pid_t pid),
				rsrc_alloc_t *alloc, unsigned count, const char *rsrcname,
				pid_t pid) {
	int		indx;
	int		ret;

	ret = EOK;
	for(indx = 0; indx < count; indx++) { 
		if(alloc[indx].flags & RSRCDBMGR_FLAG_NAME) {
			if(rsrcname != NULL) {
				alloc[indx].name = rsrcname;
			}
		} else {
			alloc[indx].name = rsrcdbmgr_id2name(alloc[indx].flags);
		}
		if(rsrcname != NULL) {
			rsrcname += strlen(rsrcname) + 1;
		}

		ret = getfunc(&alloc[indx], pid);
		if(ret != EOK) {
			//If we failed, then try and back out those changes
			for(indx-=1; indx >= 0; indx--) {
				(void)restorefunc(&alloc[indx], pid);
			}		
			break;
		}
	}

	//Mark process as having some resources allocated
	if((ret == EOK) && (pid != 0)) {
		(void)_rsrcdbmgr_pid_mark(pid);
	}
	return ret;
}


static int
do_detach(rsrc_request_t *request, unsigned count, const char *rsrcname, pid_t pid) {
	int		indx;
	int		ret;
								   
	//TODO: Do we need the range flag set here?
	ret = EOK;
	for(indx = 0; indx < count; indx++) { 
		if(request[indx].flags & RSRCDBMGR_FLAG_NAME) {
			if(rsrcname != NULL) {
				request[indx].name = rsrcname;
			}
		} else {
			request[indx].name = rsrcdbmgr_id2name(request[indx].flags);
		}
		if(rsrcname != NULL) {
			rsrcname += strlen(rsrcname) + 1;
		}

		request[indx].flags &= RSRCDBMGR_TYPE_MASK;
		request[indx].flags |= RSRCDBMGR_FLAG_RANGE;
		ret = _rsrcdbmgr_detach(&request[indx], pid);
		if(ret != EOK) {
			//Re-aquire resources if we fail to detach them
			for( ;; ) {
				--indx;
				if(indx < 0) break;
				(void)_rsrcdbmgr_attach(&request[indx], pid);
			}
			break;
		}
	}
	return ret;
}


static int
do_attach(rsrc_request_t *request, unsigned count, const char *rsrcname, pid_t pid) {
	int		indx;
	int		bestindx; 
	int		bestpri; 
	int		startindx;
	int		ret;

	ret = EOK;
	for (indx = 0; !ret && indx < count; indx++) { 
		startindx = bestindx = indx;

		if(request[indx].flags & RSRCDBMGR_FLAG_NAME) {
			if(rsrcname != NULL) {
				request[indx].name = rsrcname;
			}
		} else {
			request[indx].name = rsrcdbmgr_id2name(request[indx].flags);
		}
		if(rsrcname != NULL) {
			rsrcname += strlen(rsrcname) + 1;
		}

		if((ret = _rsrcdbmgr_find(&request[indx], 0 /* Ignored */, &bestpri))) {
			bestindx = -1;
			bestpri = INT_MAX;
		}
		while(request[indx].flags & RSRCDBMGR_FLAG_LIST && (ret == EOK || ret == EINVAL)) {
			int newpri;

			if (indx+1 >= count) {
				ret = EINVAL;
				break;
			} 

			if(request[indx+1].flags & RSRCDBMGR_FLAG_NAME) {
				if(rsrcname != NULL) {
					request[indx+1].name = rsrcname;
				}
			} else {
				request[indx+1].name = rsrcdbmgr_id2name(request[indx].flags);
			}
			if(rsrcname != NULL) {
				rsrcname += strlen(rsrcname) + 1;
			}
		
			if(strcmp(request[indx].name, request[indx+1].name) != 0) {
				ret = EINVAL;
				break;
			}
			indx++;

			//For now ignore any errors we encounter here ...
			if(_rsrcdbmgr_find(&request[indx], pid, &newpri) == EOK && newpri < bestpri) {
				bestpri = newpri;
				bestindx = indx;
				ret = EOK;
			}
		}

		if(ret == EOK && bestindx == -1) {
			ret = EINVAL;
		}

		if(ret == EOK) {
			request[bestindx].flags |= RSRCDBMGR_FLAG_RANGE;
			ret = _rsrcdbmgr_remove(&request[bestindx], pid, NULL);

			while(ret == EOK && startindx <= indx) {
				memcpy(&request[startindx++], &request[bestindx], sizeof(*request));
			}
		}
	}

	//Detach resources if we fail to attach to them
	for(indx-=2; ret && indx >= 0; indx--) {
		if(!(request[indx].flags & RSRCDBMGR_FLAG_LIST)) {
			(void)_rsrcdbmgr_detach(&request[indx], pid);
		}
	}

	if((ret == EOK) && (pid != 0)) {
		(void)_rsrcdbmgr_pid_mark(pid);
	}
	return ret;
}

/*
 This function is the entry point for our messages from
 the veneer client for resource manipulation.  All range
 validation is done here prior to actually dispatching
 the request.
*/
int 
rsrcdbmgr_handler(message_context_t *mctp, int code, unsigned flags, void *handle) {
	resmgr_context_t	*ctp = (resmgr_context_t *)mctp;
	rsrc_cmd_t 			*msg = (rsrc_cmd_t *)ctp->msg;
	rsrc_alloc_t		*alloc = (rsrc_alloc_t *)((char *)msg + sizeof(*msg));
	rsrc_request_t		*request = (rsrc_request_t *)alloc;
	char				*rsrcname;
	iov_t				reply_iov[2];
	int					ret;
	pid_t				pid;
	int					cmd;


	(void)MsgInfo(ctp->rcvid, &ctp->info);

	//Validate that we handle this type of message in the first place
	//then validate that the message fits entirely into the rcv buffer.
	switch((cmd = msg->i.subtype & RSRCDBMGR_REQ_MASK)) {
	case RSRCDBMGR_REQ_CREATE:
	case RSRCDBMGR_REQ_DESTROY:
		rsrcname = (char*)((char*)alloc + msg->i.count * sizeof(*alloc));
		goto check;
	case RSRCDBMGR_REQ_ATTACH:
	case RSRCDBMGR_REQ_DETACH:
		rsrcname = (char*)((char*)request + msg->i.count * sizeof(*request));
check:		
		if(!proc_isaccess(0, &ctp->info)) {
			return proc_status(ctp, EPERM);
		} else if(msg->i.nbytes == 0) {
			return proc_status(ctp, EOK);
		} else if(msg->i.nbytes >= ctp->msg_max_size - sizeof(*msg)) {
			return proc_status(ctp, EMSGSIZE);
		}
		break;
	case RSRCDBMGR_REQ_COUNT:
	case RSRCDBMGR_REQ_QUERY:
	case RSRCDBMGR_REQ_QUERY_NAME:
		if(msg->i.nbytes) {
			rsrcname = (char *)msg + sizeof(msg->i);
		} else {
			rsrcname = rsrcdbmgr_id2name(msg->i.subtype);
		}

		//Limit msg->i.count to our ctp buffer size.
		if(msg->i.count > (ret = (ctp->msg_max_size - sizeof(*msg)) / sizeof(*alloc))) {
			msg->i.count = ret;
		}
		break;
	default:
		return proc_status(ctp, EINVAL);
	}

	//Special handling required for minor numbers, weed them out early
	if((msg->i.subtype & RSRCDBMGR_TYPE_MASK) == RSRCDBMGR_MINOR_NUM) {
		ret = rsrcmgr_handle_minor(ctp, msg, (rsrc_minor_request_t *)alloc);
		return proc_status(ctp, ret);
	}

	pid = (msg->i.pid > 0) ? msg->i.pid : ctp->info.pid;
	ctp->status = ret = EOK;

	// Handle all other resources types here
	pthread_mutex_lock(&g_rsrc_mutex);
	switch(cmd) {
	case RSRCDBMGR_REQ_CREATE:
		//Permanently add to list
		ret = do_create_destroy(_rsrcdbmgr_create, _rsrcdbmgr_destroy, 
									alloc, msg->i.count, rsrcname, pid);
		break;
					
	case RSRCDBMGR_REQ_DESTROY: 
		//Permanently remove from list
		ret = do_create_destroy(_rsrcdbmgr_destroy, _rsrcdbmgr_create, 
									alloc, msg->i.count, rsrcname, pid);
		break;

	case RSRCDBMGR_REQ_DETACH: 
		//Move used to unused list
		ret = do_detach(request, msg->i.count, rsrcname, pid);
		break;

	case RSRCDBMGR_REQ_ATTACH: 
		//Move unsed to used list
		ret = do_attach(request, msg->i.count, rsrcname, pid);

		if(ret == EOK) {
			SETIOV(&reply_iov[0], request, msg->i.nbytes);
			MsgReplyv(ctp->rcvid, ret, reply_iov, 1);
			ret = _RESMGR_NOREPLY;
		}
		break;

	case RSRCDBMGR_REQ_QUERY: 
		//Perform some re-mapping to support previous query
		msg->i.pid = (msg->i.subtype & RSRCDBMGR_FLAG_USED) ? -1 : 0;
		msg->i.index = msg->i.count;
		msg->i.count = msg->i.nbytes / sizeof(*alloc);
		msg->i.nbytes = 0;
		/* Fall through */

	case RSRCDBMGR_REQ_COUNT: 
	case RSRCDBMGR_REQ_QUERY_NAME: 
		ret = _rsrcdbmgr_query(rsrcname, msg->i.pid, 
							  (cmd == RSRCDBMGR_REQ_COUNT) ? FLAG_QUERY_COUNT : 0, 
							  msg->i.index, alloc, msg->i.count * sizeof(*alloc), NULL);
						 
		if(ret >= 0) {
			SETIOV(&reply_iov[0], alloc, ret * sizeof(*alloc));
			if(cmd == RSRCDBMGR_REQ_QUERY) {	//Historical use
				ret = _rsrcdbmgr_query(rsrcname, msg->i.pid, FLAG_QUERY_COUNT, 0, NULL, 0, NULL); 
			}

			MsgReplyv(ctp->rcvid, ret, reply_iov, (cmd == RSRCDBMGR_REQ_COUNT) ? 0 : 1);
			ret = _RESMGR_NOREPLY;
		} else {
			ret = errno;
		}
		break;
	default:
		break;
	}
	ctp->status = ret;

	pthread_mutex_unlock(&g_rsrc_mutex);

	return proc_status(ctp, ret);
}

/*
 PROC INTERFACE
*/

/*
 Command can be any of 
 
 RSRCDBMGR_REQ_CREATE, RSRCDBMGR_REQ_DESTROY
	list is an array of count elements which are to be placed into/
	taken out of the resource database.  
	Returns errno on failure

 RSRCDBMGR_REQ_ATTACH, RSRCDBMGR_REQ_DETACH	

*/
int 
rsrcdbmgr_proc_interface(void *list, int count, int cmd) {
	int		ret;

	ret = ENOTSUP;
	pthread_mutex_lock(&g_rsrc_mutex);
	switch(cmd & RSRCDBMGR_REQ_MASK) {
	case RSRCDBMGR_REQ_CREATE: 
		ret = do_create_destroy(_rsrcdbmgr_create, _rsrcdbmgr_destroy, list, 
				count, NULL, SYSMGR_PID);
		break;
	case RSRCDBMGR_REQ_DESTROY:
		ret = do_create_destroy(_rsrcdbmgr_destroy, _rsrcdbmgr_create, list, 
				count, NULL, SYSMGR_PID);
		break;
	case RSRCDBMGR_REQ_DETACH: 
		ret = do_detach(list, count, NULL, SYSMGR_PID);
		break;
	case RSRCDBMGR_REQ_ATTACH: 
		ret = do_attach(list, count, NULL, SYSMGR_PID);
		break;
	default:
		break;
	}
	pthread_mutex_unlock(&g_rsrc_mutex);

	return(ret);
}


int 
rsrcdbmgr_proc_query(rsrc_alloc_t *list, int count, int start, int flags) {
	int		ret;

	pthread_mutex_lock(&g_rsrc_mutex);
	ret = _rsrcdbmgr_query(rsrcdbmgr_id2name(flags), 0, 0, start,
					  list, count * sizeof(*list), NULL);
	pthread_mutex_unlock(&g_rsrc_mutex);
	return ret;
}

int 
rsrcdbmgr_proc_query_name(rsrc_alloc_t *list, int count, int start, char *name) {
	int		ret;

	pthread_mutex_lock(&g_rsrc_mutex);
	ret = _rsrcdbmgr_query(name, 0, 0, start,
					  list, count * sizeof(*list), NULL);
	pthread_mutex_unlock(&g_rsrc_mutex);
	return ret;
}


/*
 Pass NULL in for name and we will free the devno, otherwise we will
 be setting a minor request based on minor_request.

 Return EOK if things succeed.
*/
int 
rsrcdbmgr_proc_devno(char *name, dev_t *devno, int minor_request, int flags) {
	int ret, major, minor;

	pthread_mutex_lock(&g_rsrc_mutex);

	major = major(*devno);
	minor = (name) ? minor_request : minor(*devno);
	ret =_rsrc_minor(name, &major, &minor, (name) ? RSRCDBMGR_REQ_ATTACH : RSRCDBMGR_REQ_DETACH);
	if(name) {
		*devno = makedev(0, major, minor);
	}

	pthread_mutex_unlock(&g_rsrc_mutex);

	return(ret);
}

__SRCVERSION("rsrcdbmgr_cmd.c $Rev: 153052 $");
