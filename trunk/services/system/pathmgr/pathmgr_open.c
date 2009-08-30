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
#include <fcntl.h>
#include <dirent.h>
#include <sys/dcmd_all.h>
#include "pathmgr_node.h"
#include "pathmgr_object.h"
#include "pathmgr_proto.h"
#include <sys/pathmgr.h>

struct open_entry {
	struct open_entry 	*next;
	struct open_entry	**prev;
	NODE 				*node;
	unsigned			rcvid;
};

static SOUL	open_souls		INITSOUL(-1, struct open_entry,   2,  8,  ~0);
static struct open_entry	*pending_opens;


struct node_ocb {
	struct node_entry				*node;
	off_t							offset;
};

int devmem_check_perm(resmgr_context_t *ctp, mode_t mode, uid_t uid, gid_t gid, int check);

static int node_read(resmgr_context_t *ctp, io_read_t *msg, void *vocb) {
	struct node_entry				*node;
	unsigned						size;
	struct dirent					*d;
	struct node_ocb					*ocb = vocb;
	int								nbytes, skip;

	//Our NAME objects always return 0 bytes like a dev null
	if ((node = ocb->node) && node->object && node->object->hdr.type == OBJECT_NAME) {
		_IO_SET_READ_NBYTES(ctp, 0);
		return _RESMGR_PTR(ctp, NULL, 0);
	}

	// Lock access to the node so that it's children don't get deleted as
	// we're walking the list.
	pathmgr_node_access( ocb->node );

	//Need to be able to handle a seek here too ...
	if (!(node = ocb->node->child)) {
		pathmgr_node_complete( ocb->node );
		return EBADF;
	}
	skip = ocb->offset;

	size = min(msg->i.nbytes, ctp->msg_max_size);
	d = (struct dirent *)(void *)msg;
	nbytes = offsetof(struct dirent, d_name) + 1;

	/*
	 If we are in a valid proc maintained directory,
	 we want to make sure that we don't actually display
	 any of the hidden entries used for cwd operations.
	 -- also in the open code
	*/
	while(nbytes < size && node && node != (struct node_entry *)-1) {
		if (node->child_objects || node->object) {
			if (skip <= 0) {
				size_t  nmlen = strlen(node->name);
				if((nbytes + nmlen) >= size) {
					break;
				}
				memset(d, 0, sizeof(struct dirent));
				d->d_namelen = nmlen;
				d->d_reclen = (offsetof(struct dirent, d_name) + nmlen + 1 + 7) & ~7;
				memcpy(d->d_name, node->name, nmlen + 1);	// '\0' guaranteed to be copied
				d->d_ino = (ino_t)node;
				nbytes += d->d_reclen;
				d = (struct dirent *)((unsigned)d + d->d_reclen);
				ocb->offset++;
			}
			skip--;
		}
		node = node->sibling;
	}
	
	pathmgr_node_complete( ocb->node );

	nbytes -= offsetof(struct dirent, d_name) + 1;

	resmgr_endian_context(ctp, _IO_READ, S_IFDIR, 0);
	_IO_SET_READ_NBYTES(ctp, nbytes);
	return _RESMGR_PTR(ctp, msg, nbytes);
}

static int node_lseek(resmgr_context_t *ctp, io_lseek_t *msg, void *vocb) {
	struct node_ocb			*ocb = vocb;

	/* The only seek offset we support is to reset the file to 0*/
	if(msg->i.whence != SEEK_SET || msg->i.offset != 0) {
		return ENOSYS;
	}

	ocb->offset = 0;
	if(msg->i.combine_len & _IO_COMBINE_FLAG) {
		return EOK;
	}

	msg->o = ocb->offset;
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}


static int node_close_ocb(resmgr_context_t *ctp, void *reserved, void *vocb) {
	struct node_ocb					*ocb = vocb;

	pathmgr_node_detach(ocb->node);
	_sfree(ocb, sizeof *ocb);
	return EOK;
}

static int node_devctl(resmgr_context_t *ctp, io_devctl_t *msg, void *vocb) {
	union {
		int								oflag;
		int								mountflag;
	}								*data = (void *)(msg + 1);
	unsigned						nbytes = 0;

	switch((unsigned)msg->i.dcmd) {
	case DCMD_ALL_GETFLAGS:
		data->oflag = O_RDONLY;
		nbytes = sizeof data->oflag;
		break;

	case DCMD_ALL_SETFLAGS:
		break;

	case DCMD_ALL_GETMOUNTFLAGS: {
		data->mountflag = 0;
		nbytes = sizeof data->mountflag;
		break;
	}

	default:
		return _RESMGR_DEFAULT;
	}

	if(nbytes) {
		msg->o.ret_val = 0;
		return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o + nbytes);
	}
	return EOK;
}

static int node_stat(resmgr_context_t *ctp, io_stat_t *msg, void *vocb) {
	union object					*o;
	struct node_ocb					*ocb = vocb;

	memset(&msg->o, 0x00, sizeof msg->o);
	msg->o.st_ino = INODE_XOR(ocb->node);
	msg->o.st_ctime =
	msg->o.st_mtime =
	msg->o.st_atime = time(0);
	msg->o.st_dev = (ctp->info.srcnd << ND_NODE_BITS) | path_devno;
	msg->o.st_rdev = 0;

	o = ocb->node->object;
	if((!o || o->hdr.type != OBJECT_PROC_SYMLINK) && ocb->node->child) {
		msg->o.st_nlink = 2;
		msg->o.st_mode = S_IFDIR | 0555;
		return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
	}

	if(o) {
		switch(o->hdr.type) {
		case OBJECT_PROC_SYMLINK:
			msg->o.st_size = o->symlink.len - 1;
			msg->o.st_nlink = 1;
			msg->o.st_mode = S_IFLNK | 0777;
			return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
		case OBJECT_NAME:
			msg->o.st_nlink = 1;
			msg->o.st_mode = S_IFNAM | 0666;
			return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
		default:
			break;
		}
	}

	return ENOSYS;
}

static const resmgr_io_funcs_t node_funcs = {
	_RESMGR_IO_NFUNCS,
	node_read,          /* read */
	0,                  /* write */
	node_close_ocb,     /* close_ocb */
	node_stat,          /* stat */
	0,                  /* notify */
	node_devctl,        /* devctl */
	0,                  /* unblock */
	0,                  /* pathconf */
	node_lseek,         /* lseek */
};


static int 
complete_open(resmgr_context_t *ctp, struct open_entry *oep, int ret) {
	NODE				*nop;
	struct open_entry	*chk;
	struct open_entry	*next;
	struct open_entry	*todo;
	int reserve_thread_fail=0;

	todo = NULL;
	nop = oep->node;
	pathmgr_node_access(nop);
	LINK2_REM(oep);
	proc_object_free(&open_souls, oep);
	if(nop->flags & NODE_FLAG_DEFERRED) {
		// Find all the deferred operations for this node.
		chk = pending_opens;
		while(chk != NULL) {
			next = chk->next;
			if(chk->node == nop) {
				LINK2_REM(chk);
				chk->next = todo;
				todo = chk;
			}
			chk = next;
		}
		nop->flags &= ~NODE_FLAG_DEFERRED;
	}
	pathmgr_node_complete(nop);
	pathmgr_node_detach_flags(nop, NODE_FLAG_UNLINKING);
	if(todo != NULL) {
		if(proc_thread_pool_reserve() != 0) {
			// ***** NOTE *****
			// the thrad pool reserve operation failed
			// this means we are running low on memory
			// but we cannot leave the pending opens hanging
			// so we continue on here
			// this reservation is mainly a performance improvement
			// not a necessity. Without this reservation, other 
			// unrelated operations, would have to wait for this to 
			// complete if there were no other threads around.
			reserve_thread_fail = 1;
		}
		// We're going to be reusing ctp below to restart the deferred
		// operations, so we need to do the reply for this op first to
		// free up the message buffer.
		switch((unsigned)ret) {
		case _RESMGR_NOREPLY:
			break;
		case _RESMGR_DEFAULT:
			ret = ENOSYS;
			/* Fall through */
		default:
			if(ret <= 0) {
				(void)resmgr_msgreplyv(ctp, ctp->iov, -ret);
			} else {
				MsgError(ctp->rcvid, ret);
			}
			break;
		}
		ret = _RESMGR_NOREPLY;
		do {	
			next = todo->next;
			// Restart the deferred operation on the node
			resmgr_msg_again(ctp, todo->rcvid);	
			proc_object_free(&open_souls, todo);
			todo = next;
		} while(todo != NULL);
		// if the original thread pool reservation did not fail
		// release the reservation here
		if (!reserve_thread_fail) {
			proc_thread_pool_reserve_done();
		}
	}
	return ret;
}


static int reply_redirect(resmgr_context_t *ctp, io_open_t *msg, OBJECT *obp) {
	unsigned						eflag;
	uint32_t						remotend;
	struct _io_connect_link_reply	*linkp = (void *)msg;
	struct _server_info				*info = (void *)(linkp + 1);

	remotend = ctp->info.nd;

	_IO_SET_CONNECT_RET(ctp, _IO_CONNECT_RET_MSG);
	eflag = msg->connect.eflag;

	memset(linkp, 0x00, sizeof *linkp);
	memset(info, 0x00, sizeof *info);

	linkp->eflag = eflag;
	linkp->path_len = 0;		//This is key, no message stream comes after the info!

	//All we need for the new fd is the nid/pid/chid trio
	if(proc_thread_pool_reserve() != 0) {
		return EAGAIN;
	}
	info->nd = netmgr_remote_nd(remotend, obp->server.nd);
	proc_thread_pool_reserve_done();
	info->pid = obp->server.pid;
	info->chid = obp->server.chid;

	//Do I need to do a MsgKeyData(ctp->rcvid, _NTO_KEYDATA_CALCULATE, ... )

	return _RESMGR_PTR(ctp, msg, sizeof *linkp + sizeof *info);
}


enum { PATHMGR_SERVICE_OPEN, PATHMGR_SERVICE_CREAT, PATHMGR_SERVICE_LS };
static int service_error(io_open_t *msg, int action)
{
	if (action != PATHMGR_SERVICE_OPEN) {
		switch (msg->connect.file_type) {
		case _FTYPE_ANY:
			break;
		case _FTYPE_FILE:
		case _FTYPE_SHMEM:
		case _FTYPE_NAME:
			break;
		case _FTYPE_SOCKET:
			if (action != PATHMGR_SERVICE_CREAT)
				return(ENOSYS);
			break;
		default:
			return(ENOSYS);
		}
	}
	else if (msg->connect.file_type != _FTYPE_ANY) {
		return(ENOSYS);
	}
	return(ENOENT);
}

int pathmgr_open(resmgr_context_t *ctp, io_open_t *msg, void *handle, void *extra) {
	NODE						*nop;
	struct node_ocb				*ocb;
	const char					*result;

	if (msg->connect.path_len + sizeof(*msg) >= ctp->msg_max_size) {
		return ENAMETOOLONG;
	}

	nop = pathmgr_node_lookup(0, msg->connect.path, 
			 /* Break out if we hit non-server object, attach to the 
				node to prevent it's deletion and avoid returning 
				empty nodes (for links, we could probably remove this) */
			 PATHMGR_LOOKUP_NOSERVER | PATHMGR_LOOKUP_ATTACH | PATHMGR_LOOKUP_NOEMPTIES |
			 /* When looking up links, ignore the autocreated entries,
				this isn't perfect, but it avoids the obvious case when
				we have overlaid two links.  It would be better to be
				able to specify one specific node to lookup as an 
				alternative. Add to the TODO list. */
			 ((ctp->id == link_root_id) ? PATHMGR_LOOKUP_NOAUTO : 0), 
			 &result);

	if(nop) {
		OBJECT				*o;
		struct open_entry	*oep;
		struct open_entry	*chk;
		
		oep = proc_object_alloc(&open_souls);
		if(oep == NULL) {
			pathmgr_node_detach(nop);
			return EAGAIN;
		}
		pathmgr_node_access(nop);
		o = nop->object;
		LINK2_BEG(pending_opens, oep, struct open_entry);
		oep->node = nop;
		oep->rcvid = ctp->rcvid;
		if(msg->connect.subtype == _IO_CONNECT_UNLINK) {
			nop->flags |= NODE_FLAG_UNLINKING;
		}
		if(nop->flags & NODE_FLAG_UNLINKING) {
			for(chk = pending_opens->next; chk != NULL; chk = chk->next) {
				if(chk->node == nop) {
					// Somebody (maybe us) is attempting to unlink this
					// node while somebody else is trying to do something
					// with it - defer this operation until things are
					// settled.
					nop->flags |= NODE_FLAG_DEFERRED;
					pathmgr_node_complete(nop);
					pathmgr_node_detach(nop);
					return _RESMGR_NOREPLY;
				}
			}
		}

		/* Only look for server/symlink entries (ie non-server) */
		if (ctp->id == link_root_id) { 
			if (o && o->hdr.type != OBJECT_PROC_SYMLINK) {
				pathmgr_node_complete(nop);
				return complete_open(ctp, oep, ENOENT);
			}
		} else {						
			/* Only look for non-link entries here */
			while (o && (o->hdr.type == OBJECT_PROC_SYMLINK)) {
				o = o->hdr.next;
			}
			/* If we don't have an object but we still have path then drop out
			   right away since code later on doesn't handle this situation */
			if (!o && result[0] != '\0') {
				pathmgr_node_complete(nop);
				return complete_open(ctp, oep, service_error(msg, PATHMGR_SERVICE_OPEN));
			}
		}
		pathmgr_node_complete(nop);

		if(o) {
			switch(o->hdr.type) {
			case OBJECT_PROC_SYMLINK:
				if(result[0] == '\0') {
					if(msg->connect.subtype == _IO_CONNECT_READLINK) {
						return complete_open(ctp, oep, reply_symlink(ctp, 0, &msg->link_reply, &o->symlink, 0, 0));
					}
					if(S_ISLNK(msg->connect.mode) && !(msg->connect.eflag & _IO_CONNECT_EFLAG_DIR)) {
						break;
					}
				}
				return complete_open(ctp, oep, reply_symlink(ctp, msg->connect.eflag, &msg->link_reply, &o->symlink, msg->connect.path, result));

			case OBJECT_MEM_SHARED:
				if((msg->connect.eflag & _IO_CONNECT_EFLAG_DIR) || result[0] != '\0') {
					return complete_open(ctp, oep, ENOTDIR);
				}

				if( (msg->connect.subtype != _IO_CONNECT_OPEN)  &&
					(msg->connect.subtype != _IO_CONNECT_COMBINE) &&
					(msg->connect.subtype != _IO_CONNECT_COMBINE_CLOSE) ) {
					break;
				}

				if((msg->connect.ioflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
					return complete_open(ctp, oep, EEXIST);
				}

				return complete_open(ctp, oep, mem_open(ctp, msg, o, extra));

			case OBJECT_NAME:
				if (result[0] == '\0' && msg->connect.file_type == _FTYPE_NAME) {
					return complete_open(ctp, oep, reply_redirect(ctp, msg, o));
				}
				if (result[0] == '\0') {
					break;
				}
				return complete_open(ctp, oep, ENOENT);

			case OBJECT_SERVER:
				if(nop->child && result[0] == '\0') {
					break;
				}
				return complete_open(ctp, oep, service_error(msg, PATHMGR_SERVICE_OPEN));

			default:
				return complete_open(ctp, oep, ENOENT);
			}
		}
		switch(msg->connect.subtype) {
		case _IO_CONNECT_READLINK:
			return complete_open(ctp, oep, ENOSYS);

		case _IO_CONNECT_UNLINK:
			if(o == NULL) {
				return complete_open(ctp, oep, nop->child ? ENOTEMPTY : ENOENT);
			}
			switch(o->hdr.type) {
			case OBJECT_NAME:
			case OBJECT_SERVER:
				if(msg->connect.eflag & _IO_CONNECT_EFLAG_DIR) {
					return complete_open(ctp, oep, (o->server.hdr.flags & PATHMGR_FLAG_DIR) ? ENOSYS : ENOTDIR);
				}
				if(!(o->server.hdr.flags & PATHMGR_FLAG_STICKY)) {
					return complete_open(ctp, oep, (o->server.hdr.flags & PATHMGR_FLAG_DIR) ? EBUSY : ENOSYS);
				}
				pathmgr_object_detach(o);
				return complete_open(ctp, oep, EOK);

			case OBJECT_PROC_SYMLINK:
				if(!proc_isaccess(0, &ctp->info)) {
					return complete_open(ctp, oep, EPERM);
				}
				pathmgr_object_detach(o);
				return complete_open(ctp, oep, EOK);
			case OBJECT_MEM_SHARED:
				/* @@@ We should really have an object callout for this */
				if(o->mem.pm.mode && devmem_check_perm(ctp, o->mem.pm.mode, o->mem.pm.uid, o->mem.pm.gid, S_ISUID)) {
					return complete_open(ctp, oep, EPERM);
				}
				pathmgr_object_detach(o);
				return complete_open(ctp, oep, EOK);
			default:
				break;
			}
			return complete_open(ctp, oep, ENOSYS);

		case _IO_CONNECT_MKNOD:
			return complete_open(ctp, oep, EEXIST);

		default:
			break;
		}

		/*
		 We don't want to show proc's hidden entries, so if that
		 is all the object that there is then don't allow opening.
		 Also if we have a longer path, we should exit out now since
		 a long path should have matched some sort of object.
		*/
		if (!o && nop->child_objects == 0) {
			return complete_open(ctp, oep, ENOENT) ;
		}
		if(service_error(msg, PATHMGR_SERVICE_LS) == ENOSYS) {
			return complete_open(ctp, oep, ENOSYS);
		}

		if(!(ocb = _scalloc(sizeof *ocb))) {
			return complete_open(ctp, oep, ENOMEM);
		}
		ocb->node = pathmgr_node_clone(nop);
		if(resmgr_open_bind(ctp, ocb, &node_funcs) == -1) {
			pathmgr_node_detach(ocb->node);
			return complete_open(ctp, oep, errno);
		}
		return complete_open(ctp, oep, EOK);
	}

	// nop == NULL at this point 

	if(msg->connect.subtype == _IO_CONNECT_MKNOD) {
		return ENOSYS;
	}
	if(!(msg->connect.ioflag & O_CREAT)) {
		return service_error(msg, PATHMGR_SERVICE_CREAT);
	}

	switch(msg->connect.file_type) {
	case _FTYPE_FILE:
	case _FTYPE_SHMEM: {
		OBJECT						*obp;
		int							status;
		PROCESS *					prp;
		
		prp = proc_lookup_pid(ctp->info.pid);
		if (MEMPART_INSTALLED()) {
			CRASHCHECK(prp == NULL);	// FIX ME - need to resolve to a real process for accounting
		}
		if(!(obp = pathmgr_object_attach(prp, 0, msg->connect.path, OBJECT_MEM_SHARED, 0, 0))) {
			return ENOMEM;
		}
		status = mem_open(ctp, msg, obp, extra);
		if(status != EOK) {
			pathmgr_object_detach(obp);
		}
		pathmgr_object_done(obp);
		return status;

	}

   case _FTYPE_TYMEM:
		return tymem_open(ctp, msg, NULL, extra);			   
		
	default:
		break;
	}
	return service_error(msg, PATHMGR_SERVICE_OPEN);
}

int pathmgr_dounlink(resmgr_context_t *ctp, io_unlink_t *msg, void *handle, void *extra) {
	return pathmgr_open(ctp, (io_open_t *)msg, handle, extra);
}

int pathmgr_domknod(resmgr_context_t *ctp, io_mknod_t *msg, void *handle, void *reserved) {
	return pathmgr_open(ctp, (io_open_t *)msg, handle, reserved);
}

int pathmgr_readlink(resmgr_context_t *ctp, io_readlink_t *msg, void *handle, void *extra) {
	return pathmgr_open(ctp, (io_open_t *)msg, handle, extra);
}


__SRCVERSION("pathmgr_open.c $Rev: 174910 $");
