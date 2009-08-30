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
#include <sys/pathmgr.h>
#include <sys/procmgr.h>
#include "pathmgr_node.h"
#include "pathmgr_object.h"
#include "pathmgr_proto.h"

int reply_symlink(resmgr_context_t *ctp, unsigned eflag, struct _io_connect_link_reply *linkp, struct symlink_object *symlinkp, const char *path, const char *tail) {
	int						len;
	iov_t					*iov;

	if(path) {
		_IO_SET_CONNECT_RET(ctp, _IO_CONNECT_RET_LINK);
	}
	linkp->eflag = eflag;
	linkp->nentries = 0;
	linkp->path_len = symlinkp->len;

	iov = ctp->iov;
	SETIOV(iov, linkp, sizeof *linkp);
	iov++;

	len = 0;
	if(path) {
		if(symlinkp->name[0] != '/' && (len = tail - path)) {
			while(--len && path[len-1] != '/') {
				/* nothing to do */
			}
		}

		if(len) {
			linkp->path_len += len;
			SETIOV(iov, path, len);
			iov++;
		}

		len = strlen(tail);
	}

	SETIOV(iov, symlinkp->name, len ? symlinkp->len - 1 : symlinkp->len);
	iov++;

	if(len) {
		linkp->path_len += len + 1;
		SETIOV(iov, tail - 1, len + 2);
		iov++;
	}

	return _RESMGR_NPARTS(iov - ctp->iov);
}

static int pathmgr_close_ocb(resmgr_context_t *ctp, void *reserved, void *ocb) {
	pathmgr_object_detach(ocb);

	//Namespace change notification, equivalent to procmgr_event_trigger
	procmgr_trigger(PROCMGR_EVENT_PATHSPACE);
	
	return EOK;
}

static int pathmgr_stat(resmgr_context_t *ctp, io_stat_t *msg, void *ocb) {
	memset(&msg->o, 0x00, sizeof msg->o);
	msg->o.st_ino = (uintptr_t)ocb ^ (uintptr_t)&pathmgr_stat;
	msg->o.st_dev = (ctp->info.srcnd << ND_NODE_BITS) | path_devno;
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

static int pathmgr_object_exists(char *path, int type) {
	OBJECT						*obp;
	NODE						*nop;

	if((nop = pathmgr_node_lookup(0, path, PATHMGR_LOOKUP_ATTACH, 0))) {
		for(obp = nop->object; obp; obp = obp->hdr.next) {
			if(obp->hdr.type == type) {
				pathmgr_node_detach(nop);
				return 1;
			}
		}
		pathmgr_node_detach(nop);
	}
	return 0;
}

/* 
 This is an override of the C library function which just does a _connect() call.
 Unfortunately this can lead to deadlock problems so proc queries the internal
 table first for the path, and if it isn't registered then fails.  If the connection
 exists then it attempts to establish a connection with that server, and that 
 server _only_.
*/
int _netmgr_connect(int base, const char *path, mode_t mode, unsigned oflag, unsigned sflag, 
					unsigned subtype, int testcancel, unsigned accessl, unsigned file_type, 
					unsigned extra_type, unsigned extra_len, const void *extra, 
					unsigned response_len, void *response, int *status) {
	struct _io_connect_entry    entry;
	struct node_entry			*node;
	union object				*o;
	char						*newpath;
	int							ret;
	size_t  slen = strlen(path) + 1;
	
	if(!(newpath = alloca(slen))) {
		errno = ENOMEM;
		return -1;
	}		
	memcpy(newpath, path, slen);	// '\0' guaranteed to be copied
	memset(&entry, 0, sizeof(entry));

	/* Perform the lookup in the pathmgr database */
	node = pathmgr_resolve_path(pathmgr_prp->root, NULL, newpath, pathmgr_prp->root);
	for(o = (node && *newpath == '\0') ? node->object : NULL; o; o = o->hdr.next) {
		/* NOTE: We can't call netmgr_remote_nd() here because it will cause deadlock
           so regardless of the client, we always require that the /dev/netmgr be local. */
		if(o->hdr.type == OBJECT_SERVER && ND_NODE_CMP(ND_LOCAL_NODE, o->server.nd) == 0) {
			entry.nd = ND_LOCAL_NODE /* netmgr_remote_nd(ND_LOCAL_NODE, o->server.nd) */;
			entry.pid = o->server.pid;
			entry.chid = o->server.chid;
			entry.handle = o->server.handle;
			entry.file_type = o->server.file_type;
			break;
		}
	}
	pathmgr_node_detach(node);

	if(!o) {
		return -1;
	}

	if(proc_thread_pool_reserve() != 0) {
		errno = EAGAIN;
		return -1;
	}
	ret = _connect_entry(base, path, mode, oflag, sflag, subtype, testcancel, accessl, file_type, extra_type, extra_len, extra, response_len, response, status, &entry, 1);
	proc_thread_pool_reserve_done();
	return ret;
}

static int pathmgr_fdinfo(resmgr_context_t *ctp, io_fdinfo_t *msg, void *vocb) {
	OBJECT						*obp = (OBJECT *)vocb;
	char						*path;
	int							size;
	size_t						pathmax;
	int							len;
	unsigned					flags = msg->i.flags;
	NODE						*node, *prev;

	if(obp->hdr.type != OBJECT_SERVER) {
		return ENOSYS;
	}

	if((size = (ctp->msg_max_size - ctp->offset) - sizeof msg->o) < 0) {
		return EMSGSIZE;
	}
	size = min(size, msg->i.path_len);
	pathmax = size;

	memset(&msg->o, 0x00, sizeof msg->o);
	path = (char *)(&msg->o + 1);

	len = 1;
	if(pathmax) {
		if(proc_thread_pool_reserve() != 0) {
			return EAGAIN;
		}
		/* If local path requested or if netmgr fails, don't include network component */
		if(	(len = netmgr_ndtostr(
				(flags & _FDINFO_FLAG_LOCALPATH) ? ND2S_LOCAL_STR | ND2S_DIR_SHOW : ND2S_DIR_SHOW,
				obp->server.nd, path, pathmax)) <= 1) {
			len = 1;
			*path = '\0';
		} else { 
			int			n;

			if((n = strlen(path))) {
				if(path[--n] == '/') {
					len--;
					path[n] = '\0';
				}
			}
		}
		proc_thread_pool_reserve_done();
		if(len < pathmax) {
			pathmax -= len - 1;
			path += len - 1;
		} else {
			pathmax = 0;
		}
	}

	prev = 0;
	do {
		for(node = obp->hdr.node; node != node->parent; node = node->parent) {
			if(node->parent == prev || node->parent == node) {
				break;
			}
		}
		if(node->len) {
			len += straddstr("/", 0, &path, &pathmax);
			len += straddstr(node->name, node->len, &path, &pathmax);
		}
		prev = node;
	} while(node != obp->hdr.node);

	if(obp->hdr.flags & _RESMGR_FLAG_DIR) {
		len += straddstr("/", 0, &path, &pathmax);
	}

	_IO_SET_FDINFO_LEN(ctp, len);
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o + min(size, len));
}
	
static const resmgr_io_funcs_t resmgrlink_io_funcs = {
	_RESMGR_IO_NFUNCS,
	0,
	0,
	pathmgr_close_ocb,
	pathmgr_stat,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	pathmgr_fdinfo
};

int
server_create(OBJECT *obp, void *data) {
	struct _io_resmgr_link_extra	*extra = data;

	if(extra->file_type == _FTYPE_NAME) {
		obp->hdr.type = OBJECT_NAME;
	}
	obp->server.nd = extra->nd;
	obp->server.pid = extra->pid;
	obp->server.chid = extra->chid;
	obp->server.handle = extra->handle;
	obp->server.file_type = extra->file_type;
	return EOK;
}

static int 
pathmgr_resmgrlink(resmgr_context_t *ctp, io_link_t *msg, void *handle, struct _io_resmgr_link_extra *extra) {
	OBJECT						*obp;
	PROCESS *					prp;

	/* For FTYPE_NAME object, we let non-root users do the attach, but 
	   only if an attached object doesn't already exist in that space. */
	if (extra && extra->file_type == _FTYPE_NAME) {
		if (pathmgr_object_exists(msg->connect.path, OBJECT_NAME)) {
			return EEXIST;
		}
	} else if(!proc_isaccess(0, &ctp->info)) {
		return EPERM;
	}

	if (extra == NULL)
		return EINVAL;
	
	prp = proc_lookup_pid(ctp->info.pid);
	if (MEMPART_INSTALLED()) {
		CRASHCHECK(prp == NULL);	// FIX ME - need to resolve to a real process for accounting
	}
	if(!(obp = pathmgr_object_attach(prp, 0, msg->connect.path, OBJECT_SERVER, extra->flags, extra))) {
		return ENOMEM;
	}

	if(!(extra->flags & PATHMGR_FLAG_STICKY)) {
		if(resmgr_open_bind(ctp, obp, &resmgrlink_io_funcs) == -1) {
			pathmgr_object_detach(obp);
			pathmgr_object_done(obp);
			return errno;
		}
	}
	pathmgr_object_done(obp);

	//Namespace change notification, equivalent to procmgr_event_trigger
	procmgr_trigger(PROCMGR_EVENT_PATHSPACE);

	return EOK;
}

int
symlink_create(OBJECT *obp, void *data) {
	char						*path = data;
	int							len;

	len = strlen(path) + 1;
	if(obp == NULL) {
		return offsetof(struct symlink_object, name) + len;
	}

	obp->symlink.len = len;
	memcpy(obp->symlink.name, path, len);
	return EOK;
}

static int 
pathmgr_sys_symlink(resmgr_context_t *ctp, io_link_t *msg, void *handle, char *path) {
	OBJECT		*obp;
	PROCESS 	*prp;

	if(!proc_isaccess(0, &ctp->info)) {
		return EPERM;
	}

	if(pathmgr_object_exists(msg->connect.path, OBJECT_PROC_SYMLINK)) {
		return EBUSY;
	}
	
	prp = proc_lookup_pid(ctp->info.pid);
	if (MEMPART_INSTALLED()) {
		CRASHCHECK(prp == NULL);	// FIX ME - need to resolve to a real process for accounting
	}
	obp = pathmgr_object_attach(prp, 0, msg->connect.path, OBJECT_PROC_SYMLINK, 0, path);
	if(obp == NULL) {
		//may return null on no mem or link name > 478 chars, which is much more likley
		return ENAMETOOLONG;
	}
	pathmgr_object_done(obp);
	return EOK;
}

int 
pathmgr_dolink(resmgr_context_t *ctp, io_link_t *msg, void *handle, io_link_extra_t *extra) {
	switch(msg->connect.extra_type) {
	case _IO_CONNECT_EXTRA_RESMGR_LINK:
		// someone is making a new device
		return pathmgr_resmgrlink(ctp, msg, handle, &extra->resmgr);

	case _IO_CONNECT_EXTRA_PROC_SYMLINK:
		// someone is trying to make a symlink
		return pathmgr_sys_symlink(ctp, msg, handle, extra->path);

	default:
		break;
	}

	return ENOSYS;
}


static char *
add_piece(char *p, char *endp, char *src, unsigned len) {
	int		left;
	char	*r = p + len;

	left = endp - p;
	if(left < 0) left = 0;
	if(left < len) len = left;
	memcpy(p, src, len);
	return r;
}


/*
 get the fullpath of a node; 
 return len (include the last NULL character)
*/

int 
pathmgr_node2fullpath(NODE* node, char *path, int pathmax) {
	int							len;
	char						*p;
	char						*endp;
	NODE						*n;
	NODE						*prev;
	NODE						*parent;
	pid_t						mypid;
	int							save_netmgr_coid = 0; //to shut the compiler up
	extern int					__netmgr_send_private(int);

	mypid = getpid();
	if(mypid != PROCMGR_PID) {
		// We're being called from a loader thread - we need to make 
		// sure that __netmgr_send() uses a private connection id when
		// it tries to contact qnet.
		save_netmgr_coid = __netmgr_send_private(-1);
	} else if(proc_thread_pool_reserve() != 0) {
		// FUTURE - we could check to see if the object is off node
		// or if qnet is running (pathmgr_netmgr_pid() returns non-zero).
		// If so, we really don't have to fail the call, but could continue
		// on and just not try to show any network component since there
		// really won't be one.
		errno = EAGAIN;
		return -1;
	}

/* If local path requested or if netmgr fails, don't include network component */
	len = netmgr_ndtostr(ND2S_DIR_SHOW,
			node->object ? node->object->server.nd:ND_LOCAL_NODE, path, pathmax);

	if(mypid != PROCMGR_PID) {
		(void)__netmgr_send_private(save_netmgr_coid);
	} else {
		proc_thread_pool_reserve_done();
	}

	if(--len < 0) {
		len = 0;
	}
	p = path + len;
	endp = path + pathmax;

	if((len > 0) && (p <= endp) && (p[-1] == '/')) {
		--p;
	}

	prev = 0;
	do {
		n = node;
		for( ;; ) {
			parent = n->parent;
			if(parent == n) break;
			if(parent == node) break;
			if(parent == prev) break;
			n = parent;
		}
		if(n->len) {
			p = add_piece(p, endp, "/", 1);
			p = add_piece(p, endp, n->name, n->len);
		}
		prev = n;
	} while(n != node);
	len = (p - path) + 1; // len includes the trailing '\0'
	if(pathmax > 0) {
		// append the '\0' (if there's space)
		path[(min(len, pathmax))-1] = '\0';
	}

	return len;
}

__SRCVERSION("pathmgr_link.c $Rev: 205218 $");
