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
#include <sys/iofunc.h>
#include <sys/pathmgr.h>
#include "pathmgr_node.h"
#include "pathmgr_object.h"
#include "pathmgr_proto.h"
#include <sys/trace.h>

static int pathmgr_resolve_handler(resmgr_context_t *ctp, void *msg) {
	struct _io_connect			*connect = msg;
	struct node_entry			*node;
	PROCESS						*prp;
	const char					*result;
	int							ret;

	if(ctp->rcvid == 0) {
		return _RESMGR_DEFAULT;
	} else if(connect->type != _IO_CONNECT) {
		return ENOSYS;
	/* TODO: We should be able to read in some component of the path and then 
	         decipher it, then read in more of the path so that while large
			 pathname resolution is slow, it is possible.  By doing this
			 we just plain say no to long pathname resolution, which isn't
			 right (but stops proc from crashing) and should definitely be fixed.
	*/
	} else if (connect->path_len + sizeof(*connect) >= ctp->msg_max_size) {
		return ENAMETOOLONG;
	}

	ret = ENOSYS;
	ctp->status = 0;

#if defined(VARIANT_instr)
    {
        iov_t                           iov[3];     //@@@ Reuse the ctp->iov?
        SETIOV(iov, &ctp->info.pid, sizeof(ctp->info.pid)); 
        SETIOV(iov + 1, &ctp->info.tid, sizeof(ctp->info.tid)); 
        SETIOV(iov + 2, connect->path, connect->path_len);	//Note, not necessarily null terminated
        (void) KerextAddTraceEventIOV(_TRACE_SYSTEM_C, _NTO_TRACE_SYS_PATHMGR, iov, 3);
    }
#endif

	switch(connect->handle) {
	case PATHMGR_HANDLE:
		if(!(prp = proc_lock_pid(ctp->info.pid))) {
			ret = EL2HLT;
			break;
		}
		node = pathmgr_resolve_path((connect->path[0] == '/') ? prp->root : prp->cwd, connect, connect->path, prp->root);
		proc_unlock(prp);
		ret = pathmgr_resolve_servers(ctp, node, connect, connect->path, prp->root);
		pathmgr_node_detach(node);
		break;

	case PATHMGR_HANDLE_REMOTE:
		// Request from remote node to localy resolve pathnames
		if((node = pathmgr_node_lookup(0, connect->path, PATHMGR_LOOKUP_ATTACH, &result))) {
			ret = pathmgr_resolve_servers(ctp, node, connect, result, 0);
			pathmgr_node_detach(node);
		} else {
			ret = ENOENT;
		}
		break;

	default:
		break;
	}

	return ret;
}

static int pathmgr_handler(message_context_t *ctp, int code, unsigned value, void *handle) {
	struct _io_connect			*connect = (struct _io_connect *)(void *)ctp->msg;
	struct node_entry			*node, *tmpnode;
	struct node_entry			**np;
	PROCESS						*prp;
	int							ret;

	ret = ENOSYS;
	ctp->status = 0;

	switch(connect->type) {
	case _PATH_CHDIR:
	case _PATH_CHROOT:
		if(!(prp = proc_lock_pid(ctp->info.pid))) {
			ret = EL2HLT;
			break;
		}
		
		/*
		 resolve_path artificially bumps up the links count on the node that it
		 returns.  We are going to create our own node, so knock down the link
		 count on tmpnode by calling detach when we are finished with it.
		*/
		tmpnode = pathmgr_resolve_path((connect->path[0] == '/') ?  prp->root : prp->cwd, connect, connect->path, prp->root);
		node = pathmgr_node_lookup(tmpnode, connect->path, 
						PATHMGR_LOOKUP_ATTACH | PATHMGR_LOOKUP_CREATE, 0);
		pathmgr_node_detach(tmpnode);
		if(node == NULL) {
			proc_unlock(prp);
			ret = ENOMEM;			
			break;
		}

		switch(connect->type) {
		case _PATH_CHDIR: 
			np = &prp->cwd;
			break;
		case _PATH_CHROOT:
			np = &prp->root;
			break;
		default:
			np = NULL;
		}
		if(np) {
			tmpnode = *np;
			*np = node;
			pathmgr_node_detach(tmpnode);
		}
		proc_unlock(prp);
		ret = EOK;
		/* fall through */
	default:
		break;
	}

	return proc_status((resmgr_context_t *)ctp, ret);
}

void pathmgr_init(void) {
	static resmgr_connect_funcs_t		pathmgr_connect_funcs;
	static resmgr_io_funcs_t			pathmgr_io_funcs;
	static iofunc_attr_t				pathmgr_attr;
	message_attr_t						mattr;
	resmgr_attr_t						rattr;

	/* Redirect PATHMGR messages to path handler */
	memset(&mattr, 0x00, sizeof(mattr));
	mattr.flags = MSG_FLAG_CROSS_ENDIAN;
	message_attach(dpp, &mattr, _PATHMGR_BASE, _PATHMGR_MAX, pathmgr_handler, 0);
	
	memset(&rattr, 0x00, sizeof(rattr));
	rattr.flags = RESMGR_FLAG_ATTACH_OTHERFUNC | RESMGR_FLAG_CROSS_ENDIAN;
	rattr.other_func = pathmgr_resolve_handler;

	/* allocate first line pathname resolution entry entry (MUST BE PATHMGR_HANDLE) */
	if(resmgr_attach(dpp, &rattr, "", _FTYPE_ANY, PATHMGR_FLAG_DIR, 0, 0, 0) != PATHMGR_HANDLE) {
		crash();
	}

	/* allocate network root entry (MUST BE PATHMGR_HANDLE_REMOTE) */
	if(resmgr_attach(dpp, &rattr, "", _FTYPE_ANY, PATHMGR_FLAG_OPAQUE | PATHMGR_FLAG_DIR, 0, 0, 0) != PATHMGR_HANDLE_REMOTE) {
		crash();
	}

	if(rsrcdbmgr_proc_devno("pathmgr", &path_devno, -1, 0) == -1) {
		crash();
	}
	/* Make sure path_devno is not used... */
	if(path_devno == 0 && rsrcdbmgr_proc_devno("pathmgr", &path_devno, -1, 0) == -1) {
		crash();
	}

	/* Initialize the pathmgr functions */
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &pathmgr_connect_funcs, _RESMGR_IO_NFUNCS, &pathmgr_io_funcs);
	pathmgr_connect_funcs.open = pathmgr_open;
	pathmgr_connect_funcs.unlink = pathmgr_dounlink;
	pathmgr_connect_funcs.mknod = pathmgr_domknod;
	pathmgr_connect_funcs.readlink = pathmgr_readlink;
	pathmgr_connect_funcs.link = pathmgr_dolink;
	
	/* Initialize the pathmgr root directory attribute */
	iofunc_attr_init(&pathmgr_attr, S_IFDIR | 0555, 0, 0);

	/* Put the proc symlink handler on the bottom of the stack (save the id?) */
	link_root_id = resmgr_attach(dpp, &rattr, "", _FTYPE_ALL, PATHMGR_FLAG_OPAQUE | PATHMGR_FLAG_DIR | PATHMGR_FLAG_AFTER,
				&pathmgr_connect_funcs, &pathmgr_io_funcs, &pathmgr_attr);

	/* Attach to root to catch all requests that don't resolve to another manager. */
	root_id = resmgr_attach(dpp, &rattr, "", _FTYPE_ALL, PATHMGR_FLAG_OPAQUE | PATHMGR_FLAG_DIR | PATHMGR_FLAG_AFTER,
				&pathmgr_connect_funcs, &pathmgr_io_funcs, &pathmgr_attr);

}

/*
 * This function overrides the one in libc to allow internal attaches to be
 * resolved through the pathname tables.
 */
int pathmgr_link(const char *path, uint32_t nd, pid_t pid, int chid, unsigned handle, enum _file_type file_type, unsigned flags) {
	struct node_entry					*node;
	char								buff[512 + 1];		/* a large internal size... */
	int									ret;

	// Make sure all proc pathmgrs can be looked up by proc
	flags |= PATHMGR_FLAG_SELF;

	switch(handle) {
	case PATHMGR_HANDLE:
		/* Nothing required for this. */
		return 0;

	case PATHMGR_HANDLE_REMOTE:
		/* Allocate a system process root. */
		pathmgr_prp->cwd = pathmgr_prp->root = node = pathmgr_node_alloc(0, 0);
		node->parent = node;
		return 0;

	default:
		break;
	}

	/* Copy path to a buffer as the passed in path is const, and the */
	/* pathmgr_resolve_path function will modify the path. */
	/* 
	 No error checking is performed, but this path mangling is done so that we 
	 remain consistent with the C library function in that if a NULL path is 
	 provided.  We expect the user flags to be set properly.
	*/
	if (path) {
		STRLCPY(buff, path, sizeof(buff));
	} else {
		buff[0] = '/'; buff[1] = '\0';
	}

	/* Find the existing node. */
	ret = -1;
	if((node = pathmgr_resolve_path(pathmgr_prp->root, 0, buff, pathmgr_prp->root))) {
		struct _io_resmgr_link_extra		extra;
		OBJECT								*obp;
	
		extra.nd = ND_LOCAL_NODE;
		extra.pid = pid ? pid : SYSMGR_PID;
		extra.chid = chid;
		extra.handle = handle;
		extra.file_type = file_type;
		extra.flags = flags;
		obp = pathmgr_object_attach(pathmgr_prp, node, buff, OBJECT_SERVER, flags, &extra);
		if(obp != NULL) {
			ret = 0;
			pathmgr_object_done(obp);
		}
		pathmgr_node_detach(node);
	}
	return ret;
}

__SRCVERSION("pathmgr_init.c $Rev: 159046 $");
