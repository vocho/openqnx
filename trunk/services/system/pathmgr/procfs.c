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
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <sys/dcmd_all.h>
#include <sys/image.h>
#include <sys/procfs.h>
#include <sys/fault.h>
#include <sys/ftype.h>
#include <kernel/debug.h>

#include "pathmgr_node.h"
#include "pathmgr_object.h"
#include "pathmgr_proto.h"
#include "apm.h"
#include <sys/pathmgr.h>

#include "procfs.h"

static dev_t				procfs_devno;


/** Proc Filesystem Callouts for pathmanager **/

/*
 * This link code needs to be stream lined ... and it would be nice
 * to get rid of the temporary buffer that is jammed in there.
 */
static int return_a_link(resmgr_context_t *ctp, struct _io_connect_link_reply *link_reply,
                  char * path, char * append, int eflag, int newlookup, int addslash, 
				  struct _io_connect_entry *entry) {
		char *p, buff[PATH_MAX + 1];
        int len;

        //This causes the return to only back out to libc before re-searching
        //with this new path that we have provided to it.
        if (newlookup) {
                _IO_SET_CONNECT_RET(ctp, _IO_CONNECT_RET_LINK);
        }

		p = (char *)(link_reply + 1);
        len = strlen(path);
		memcpy(buff, path, len + 1);
		if (append) {
			if (*append && *append != '/')
				buff[len++] ='/';
			memcpy(&buff[len], append, strlen(append) + 1);
			len += strlen(append);
		}
		len++;

		if (entry) {
			memcpy(p, entry, sizeof(*entry));
			p += sizeof(*entry);
		}
			
		if (addslash && *buff != '/')  {
			*p++ = '/';		
		}
        memcpy(p, buff, len);
		if (addslash && *buff != '/') {
			len++;
		}
		
        link_reply->eflag = eflag;
        link_reply->nentries = (entry) ? 1 : 0;
        link_reply->path_len = len;

		/*
		 We return the data in the following format:
		 {link reply}
		 [nentries of struct _io_connect_entry]
		 {path}
		*/
        return(_RESMGR_PTR(ctp, link_reply, sizeof(*link_reply) + 
											len + 
											((entry) ? sizeof(*entry) : 0)));
}

static NODE *proc_pathmgr_resolve(int type, char *path, const char **tail) {
	if (tail)
		*tail = NULL;

	while (*path && *path == '/') { path++; }

	//I should probably pass in PATHMGR_LOOKUP_ATTACH,
	//and then call pathmgr_node_detach() when I'm finished
	return(pathmgr_node_lookup(0, path, PATHMGR_LOOKUP_ATTACH, tail));
}


/*
 * This function does the bulk of the work in looking up and 
 * manipulating the pathmgr entries in terms of validating
 * them and re-directing them to the appropriate person.
 */
static int proc_pathmgr_validate(resmgr_context_t *ctp, io_open_t *msg, int type, char *path, void **handle) {
	NODE		*nop;
	OBJECT		*obj;
	char		*cresult;
	const char	*result;
	int			ret;

	if(handle) {
		*handle = 0;
	}

	if (!path || !*path) {	//Top level dir (/mount || /link) are directories
		msg->connect.mode = (msg->connect.mode & ~S_IFMT) | S_IFDIR;
		return EOK;
	}

	if (!(nop = proc_pathmgr_resolve(type, path, &result))) {
		return ENOENT;
	}
		
	/*
	 If we are looking at the mount directory, and we
	 don't have a full node match, then we hope that
	 the non-matching component is a nid,pid,chid/...
	 thingy so we can send off the request.
	*/
	ret = EOK;
	if ((type & PROCFS_FLAG_MOUNT) && result && *result) {
		char					*lastslash;
		struct _io_connect_entry entry;

		cresult = lastslash = (char *)result;	

		ret = ENOENT;
		/* This really licks, come up w/ a smaller method */
		entry.nd = strtoul(cresult, &cresult, 10);
		if (*cresult++ != ',') {
			goto nothere;
		}
		entry.pid = strtoul(cresult, &cresult, 10);
		if (*cresult++ != ',') {
			goto nothere;
		}
		entry.chid = strtoul(cresult, &cresult, 10);
		if (*cresult++ != ',') {
			goto nothere;
		}
		entry.handle = strtoul(cresult, &cresult, 10);
		if (*cresult++ != ',') {
			goto nothere;
		}
		//bstecher says this should handle -1 just fine
		entry.file_type = strtoul(cresult, &cresult, 10);
		if (*cresult != '\0' && *cresult++ != '/') {
			goto nothere;
		}

		for (obj = nop->object; obj; obj = obj->hdr.next) {
			if (obj->hdr.type != OBJECT_SERVER && obj->hdr.type != OBJECT_NAME) {
				continue;
			}
			if (obj->server.nd == entry.nd && obj->server.pid == entry.pid &&
				obj->server.chid == entry.chid && obj->server.handle == entry.handle &&
				obj->server.file_type == entry.file_type) {
				entry.key = 0;
				break;
			}
		}
		if (!obj) {
			goto nothere;
		}

		/* Procfs handles ftypes that can't handle themselves, top level */
#if 1
		if((obj->hdr.type == OBJECT_NAME) ||
		   (obj->hdr.flags & PATHMGR_FLAG_FTYPEONLY) && msg->connect.file_type != entry.file_type) {
			if(msg->connect.ioflag & _IO_FLAG_MASK) {
				ret = ENOTSUP;
			} else {
				msg->connect.mode &= ~S_IFMT;
				if(obj->hdr.flags & PATHMGR_FLAG_DIR) {
					msg->connect.mode |= S_IFDIR;
				}
				ret = EOK;
			}
		} else if(*cresult && !(obj->hdr.flags & PATHMGR_FLAG_DIR)) {
			ret = ENOTDIR;
		} else {
			/* Adjust the various path pieces so that we can re-direct them */
			entry.prefix_len = (cresult - msg->connect.path) + 1;
			ret = return_a_link(ctp, &msg->link_reply, msg->connect.path, NULL, msg->connect.eflag, 1, 1, &entry);
		}
#else
		if ((msg->connect.ioflag & _IO_FLAG_MASK) == 0 &&
				*cresult == '\0' && entry.file_type != _FTYPE_ANY) {
			if (obj->hdr.flags & PATHMGR_FLAG_DIR) {
				msg->connect.mode |= S_IFDIR;
			}
			ret = EOK;
		} else if (msg->connect.file_type != entry.file_type &&
				(obj->hdr.flags & PATHMGR_FLAG_FTYPEONLY)) {
			/* Only forward file types that match */
			ret = ENOTSUP;
		} else {
			/* Adjust the various path pieces so that we can re-direct them */
			entry.prefix_len = cresult - msg->connect.path + 1;
			ret = return_a_link(ctp, &msg->link_reply, msg->connect.path, NULL, msg->connect.eflag, 1, 1, &entry);
		}
#endif
		if(handle) {
			*handle = obj;
		}
	} else if (type & PROCFS_FLAG_LINK) {
		/*
		 * If we are looking at the link directory, then we
		 * will have a full match on the target, but will 
		 * need to re-direct it if it contains a symlink 
		 * node.
		 */
		for (obj = nop->object; obj; obj = obj->hdr.next) {
			if (obj->hdr.type == OBJECT_PROC_SYMLINK) {
				break;
			}
		}
		if (obj) {
			cresult = (char *)result;
			//kprintf("Perform a symlink re-direction to %s / %s \n", nop->name, (cresult) ? cresult : "NULL");	
			ret = return_a_link(ctp, &msg->link_reply,  nop->name, cresult, 
								 msg->connect.eflag, 1, 1, NULL);
		}
		if(handle) {
			*handle = obj;
		}
	} else {
		msg->connect.mode = (msg->connect.mode & ~S_IFMT) | S_IFDIR;
		if(handle) {
			*handle = nop;
		}
	}
	/* It is an internal directory node */
nothere:
	pathmgr_node_detach(nop);
	return ret;
}

#define TOP_BIT		(0x01000000)		/* Only partway up, I know */
static int proc_pathmgr_read(resmgr_context_t *ctp, io_read_t *msg, struct procfs_ocb *ocb) {
	int		nbytes, amount, count;
	int		max_npc_size;
	NODE	*nop;
	OBJECT	*obj;
	struct dirent *d;

	//Do a node lookup on the path ...
	if (!(nop = proc_pathmgr_resolve(ocb->flags, ocb->tail, 0))) {
		return EINVAL;
	}

	//Actually perform the stuffing ritual for directories
	max_npc_size = INT_STRLEN_MAXIMUM(pid_t) + 4*INT_STRLEN_MAXIMUM(_uint32) + 5;
	nbytes = min(msg->i.nbytes, ctp->msg_max_size - ((offsetof(struct dirent, d_name)
	         + max_npc_size + 7) & ~7));
	d = (struct dirent *)(void *)msg;
	amount = 0;
	obj = NULL;

	//Add the nodes at this level first
	if (!(ocb->offset & TOP_BIT)) {

		obj = (ocb->flags & PROCFS_FLAG_LINK) ? NULL : nop->object;
		for (count = 0; obj; obj = obj->hdr.next) {
			char  *p;
			
			if (obj->hdr.type != OBJECT_SERVER && obj->hdr.type != OBJECT_NAME) {
				continue;
			}

			if (count >= ocb->offset) {
				//We really need to put the name in here too, not just guess
				if (amount + sizeof(*d) + max_npc_size >= nbytes) {
					break;
				}

				memset(d, 0x00, sizeof *d);
				p = d->d_name;
				ultoa(obj->server.nd, p, 10);  p += strlen(p); *p++ = ',';
				ultoa(obj->server.pid, p, 10);  p += strlen(p); *p++ = ',';
				ultoa(obj->server.chid, p, 10);  p += strlen(p); *p++ = ',';
				ultoa(obj->server.handle, p, 10);  p += strlen(p); *p++ = ',';
				(void)ltoa(obj->server.file_type, p, 10);  p += strlen(p); *p++ = '\0';
				d->d_namelen = strlen(d->d_name) + 1;
				d->d_ino = ocb->offset + 1;
				d->d_reclen = (offsetof(struct dirent, d_name) + d->d_namelen + 1 + 7) & ~7;

				ocb->offset++;
				amount += d->d_reclen;
				d = (struct dirent *)((unsigned)d + d->d_reclen);
			}
			count++;
		}	
		/* We want to read the second section too */
		if (!obj || ocb->flags & PROCFS_FLAG_LINK) {	
			ocb->offset = TOP_BIT;
		}
	}

	if (ocb->offset & TOP_BIT) {
		NODE *tmpnode;
		count = TOP_BIT;

		// Lock access to the node so that it's children don't get deleted as
		// we're walking the list.
		pathmgr_node_access( nop );
		
		for (tmpnode = nop->child; tmpnode; tmpnode = tmpnode->sibling) {
			if (tmpnode->object) {
				if ((ocb->flags & PROCFS_FLAG_MOUNT) && 
					 tmpnode->object->hdr.type != OBJECT_SERVER && tmpnode->object->hdr.type != OBJECT_NAME)  {
					continue;
				}
				if ((ocb->flags & PROCFS_FLAG_LINK) && tmpnode->object->hdr.type != OBJECT_PROC_SYMLINK) {
					continue;
				}
			} else if (tmpnode->child_objects == 0) {
				continue;
			}

			if (count >= ocb->offset) {
				if (amount + sizeof(*d) + tmpnode->len  >= nbytes) {
					break;
				}

				memset(d, 0x00, sizeof *d);
				memcpy(d->d_name, tmpnode->name, tmpnode->len);
				d->d_name[tmpnode->len] = '\0';
				d->d_namelen = tmpnode->len + 1;
				d->d_ino = ocb->offset+1;
				d->d_reclen = (offsetof(struct dirent, d_name) + d->d_namelen + 1 + 7) & ~7;

				ocb->offset++;
				amount += d->d_reclen;
				d = (struct dirent *)((unsigned)d + d->d_reclen);
			}
			count++;
		}
		
		pathmgr_node_complete( nop );
	}
	
	resmgr_endian_context(ctp, _IO_READ, S_IFDIR, 0);
	_IO_SET_READ_NBYTES(ctp, amount);
	pathmgr_node_detach(nop);
	return _RESMGR_PTR(ctp, msg, amount);
}

/*******/

static int procfs_read(resmgr_context_t *ctp, io_read_t *msg, void *vocb) {
	struct procfs_ocb			*ocb = vocb;
	PROCESS						*prp = NULL;
	unsigned					size;
	void						*ptr;
	int							nbytes;
	uintptr_t					*pos, offset;
	struct dirent				*d;

	switch(msg->i.xtype & _IO_XTYPE_MASK) {
	case _IO_XTYPE_NONE:
		pos = &ocb->offset;
		break;
	case _IO_XTYPE_OFFSET:
		*(pos = &offset) = ((struct _xtype_offset *)(msg + 1))->offset;
		break;
	default:
		return ENOSYS;
	}

	/* 
	 Case #1 We are reading from a link or a mount point dir
	 Eventually we will have all these reads as callouts 
	*/
	if (ocb->flags & (PROCFS_FLAG_LINK | PROCFS_FLAG_MOUNT)) {
		return proc_pathmgr_read(ctp, msg, ocb);
	}

	/* Case #2 we are reading from one of the process id directories */
	if(ocb->pid && !(ocb->flags & PROCFS_FLAG_MASK)) {
		if(ocb->pid == -1) {
			return EIO;
		}
		if(!(ocb->flags & PROCFS_FLAG_AS)) {
			if(*pos) {
				_IO_SET_READ_NBYTES(ctp, 0);
				return _RESMGR_PTR(ctp, NULL, 0);
			}
			size = msg->i.nbytes;
			d = (struct dirent *)(void *)msg;
			memset(d, 0x00, sizeof *d);
			d->d_name[0] = 'a'; d->d_name[1] = 's'; d->d_name[2] = '\0';
			d->d_namelen = 2;
			d->d_ino = PID_TO_INO(ocb->pid, 0);
			d->d_reclen = (offsetof(struct dirent, d_name) + d->d_namelen + 1 + 7) & ~7;
			if(d->d_reclen > size) {
				return EMSGSIZE;
			}
			d->d_offset = ++*pos;
			resmgr_endian_context(ctp, _IO_READ, S_IFDIR, 0);
			_IO_SET_READ_NBYTES(ctp, d->d_reclen);
			return _RESMGR_PTR(ctp, msg, d->d_reclen);
		}

		if(!(ocb->ioflag & _IO_FLAG_RD)) {
			return EBADF;
		}

 		// CSCed94650: Avoid a deadlock between procnto and qnet
 		// caused by MsgReplying *over* qnet to a remote client when the
 		// targetted (and LOCKED) process is the local qnet itself.
 		// No easy way to tell if the target is qnet (net.prp not available)
 		// so we conservatively do it if the client is remote.

		//RUSH2: Once the new memmgr is online, we can redo this by
		//RUSH2: mmap'ing "/proc/<pid>/as" and avoiding the double copy...
 
 		if (ND_NODE_CMP(ND_LOCAL_NODE, ctp->info.nd) != 0) {
 			uintptr_t offset2 = 0;
 			// we reuse msg, so save off any msg->i fields we need.
 			unsigned desiredsize = msg->i.nbytes;
 			unsigned mappedsize;
 			unsigned copysize;
			struct _procfs_map_info mi;

 			while(1) {
 				{ // Acquire needed locks
 					if(!(prp = proc_lock_pid(ocb->pid))) {
 						return ESRCH;
 					}
					// We get a write lock even though we're only reading
					// because we might fault during the copy and end
					// up in fault_pulse(). That code needs a write lock
					// and we don't have any gear to tell it to promote
					// the read lock
 					if(proc_wlock_adp(prp) == -1) {
 						return proc_error(ESRCH, prp);
 					}
					proc_lock_owner_mark(prp);
 					ProcessBind(ocb->pid);
 				}
 
				ptr = (void *)*pos;
 				mappedsize = memmgr.mapinfo(prp, *pos, &mi, NULL, 0, NULL, NULL, NULL);
 				copysize   = min(desiredsize, min(mappedsize, ctp->msg_max_size));
				if ((copysize != 0) && ( mappedsize == (unsigned)-1 || !(mi.flags & PROT_READ) )) {
					ProcessBind(0);
					proc_unlock_adp(prp);
					proc_unlock(prp);
					return EINVAL;
				}
 				memcpy(msg, ptr, copysize);
 
 				{ // Release locks
 					ProcessBind(0);
 					proc_unlock_adp(prp);
 					proc_unlock(prp);
 				}
 
 				if(copysize == 0) {
 					// Done transferring data, locks released; flee.
 					// [True, this last memcpy was a NOP.  Rien fait.]
 					break;
 				}
 
 				if (resmgr_msgwrite(ctp, msg, copysize, offset2) == -1) {
 					return errno;
 				}
 
 				offset2     += copysize;
 				*pos        += copysize;
 				desiredsize -= copysize;
 			}
  
 			// We reply NULL,0 as the resmgr_msgwrite()s have done all the
 			// needed copying to client;
 			// We just want the client wakeup 'side-effect' of MsgReply.
 			// Total bytes copied, offset2, *is* returned to the client.
 			MsgReply(ctp->rcvid, offset2, NULL,0);
 			return _RESMGR_NOREPLY;
 		}

		if(!(prp = proc_lock_pid(ocb->pid))) {
			return ESRCH;
		}

		// We get a write lock even though we're only reading
		// because we might fault during the copy and end
		// up in fault_pulse(). That code needs a write lock
		// and we don't have any gear to tell it to promote
		// the read lock
		if(proc_wlock_adp(prp) == -1) {
			return proc_error(ESRCH, prp);
		}
		proc_lock_owner_mark(prp);
		
		ProcessBind(ocb->pid);

		ptr = (void *)*pos;
		size = memmgr.mapinfo(prp, *pos, NULL, NULL, 0, NULL, NULL, NULL);
		size = min(size, msg->i.nbytes);
		*pos += size;

		if(MsgReply(ctp->rcvid, size, ptr, size) == -1) {
			if(errno == EFAULT) {
				MsgError(ctp->rcvid, EIO);
			} else {
				MsgError(ctp->rcvid, errno);
			}
		}

		ProcessBind(0);
		proc_unlock_adp(prp);

		return proc_error(_RESMGR_NOREPLY, prp);
	}

	/* Case #3 We are reading from the root directory */
	size = min(msg->i.nbytes, ctp->msg_max_size - ((offsetof(struct dirent, d_name)
		+ INT_STRLEN_MAXIMUM(pid_t) + 1 + 7) & ~7));
	d = (struct dirent *)(void *)msg;
	nbytes = 0;

	//Jump over the self entry on remote queries
	if(*pos == 0 && ND_NODE_CMP(ND_LOCAL_NODE, ctp->info.nd) != 0) {
		++*pos;
	}

	while(nbytes < size) { 
		pid_t		pid;

		memset(d, 0x00, sizeof *d);

#if defined(SUPPORT_LINK) && defined(SHOW_MOUNT) 
#define EXTRA_ENTRIES 3
#elif defined(SHOW_MOUNT)
#define EXTRA_ENTRIES 2
#else
#define EXTRA_ENTRIES 1
#endif

		pid = 0;
		if (*pos >= EXTRA_ENTRIES) {
			uintptr_t	offset3;

			offset3 = *pos - (EXTRA_ENTRIES - 1);
			prp = QueryObject(_QUERY_PROCESS, offset3, 
									_QUERY_PROCESS_VECTOR, 0, &offset3, 0, 0);
			if(prp == NULL) break;
			pid = prp->pid;
			QueryObjectDone(prp);
			*pos = offset3 + (EXTRA_ENTRIES -1);
		}

		switch (*pos) {
		case 0:
			memcpy(d->d_name, "self", sizeof("self"));
			d->d_ino = PID_TO_INO(ctp->info.pid, PROCFS_FLAG_AS);
			break;
#ifdef SHOW_MOUNT
		case 1:
			memcpy(d->d_name, "mount", sizeof("mount"));
			d->d_ino = INODE_XOR(0); 
			break;
#endif
#ifdef SUPPORT_LINK
		case 2:
			memcpy(d->d_name, "link", sizeof("link"));
			d->d_ino = INODE_XOR(0); 
			break;
#endif
		default:	
			ultoa(pid, d->d_name, 10);
			d->d_ino = PID_TO_INO(pid, PROCFS_FLAG_AS);
			break;
		}

		d->d_namelen = strlen(d->d_name);
		d->d_reclen = (offsetof(struct dirent, d_name) + d->d_namelen + 1 + 7) & ~7;

		{//Clear the last bits ... great place for an easter egg to hide
			int extra;
			extra  = d->d_reclen - (offsetof(struct dirent, d_name) + d->d_namelen + 1);
			if (extra > 0) {
				memset(d->d_name + d->d_namelen + 1, '\0', extra);
			}
		}

		if((nbytes + d->d_reclen) >= size) {
			if(!nbytes) return EMSGSIZE;
			break;
		}

		d->d_offset = ++*pos;
		nbytes += d->d_reclen;
		d = (struct dirent *)((unsigned)d + d->d_reclen);
	}

	resmgr_endian_context(ctp, _IO_READ, S_IFDIR, 0);
	_IO_SET_READ_NBYTES(ctp, nbytes);
	return _RESMGR_PTR(ctp, msg, nbytes);
}

static int procfs_write(resmgr_context_t *ctp, io_write_t *msg, void *vocb) {
	struct procfs_ocb			*ocb = vocb;
	PROCESS						*prp;
	unsigned					size;
	uintptr_t					*pos, offset;
	int							nbytes, skip;

	switch(msg->i.xtype & _IO_XTYPE_MASK) {
	case _IO_XTYPE_NONE:
		pos = &ocb->offset;
		skip = sizeof(io_write_t);
		break;
	case _IO_XTYPE_OFFSET:
		*(pos = &offset) = ((struct _xtype_offset *)(msg + 1))->offset;
		skip = sizeof(io_write_t) + sizeof(struct _xtype_offset);
		break;
	default:
		return ENOSYS;
	}

	if(ocb->pid <= 0 || (ocb->flags & PROCFS_FLAG_MASK)) {
		return EIO;
	}
	if(!(ocb->ioflag & _IO_FLAG_WR)) {
		return EBADF;
	}
	if(!(prp = proc_lock_pid(ocb->pid))) {
		return ESRCH;
	}

	// We get a write lock even though we're not manipulating the
	// aspace structures because we might fault during the copy and
	// end up in fault_pulse(). That code needs a write lock
	// and we don't have any gear to tell it to promote
	// the read lock.
	if(proc_wlock_adp(prp) == -1) {
		return proc_error(ESRCH, prp);
	}
	proc_lock_owner_mark(prp);

	ProcessBind(ocb->pid);

	size = memmgr.mapinfo(prp, *pos, NULL, NULL, 0, NULL, NULL, NULL);
	size = min(size, msg->i.nbytes);
	SETIOV(ctp->iov + 0, (void *)*pos, size);
	*pos += size;

	if((nbytes = resmgr_msgreadv(ctp, ctp->iov + 0, 1, skip)) == -1) {
		ProcessBind(0);
		proc_unlock_adp(prp);
		return proc_error(errno, prp);
	}

	ProcessBind(0);
	proc_unlock_adp(prp);

	_IO_SET_WRITE_NBYTES(ctp, nbytes);

	return proc_error(EOK, prp);
}

static int procfs_waitstop(PROCESS *prp, int rcvid) {
	struct {
		struct _io_devctl_reply			reply;
		procfs_status					status;
	}								msg;
	struct procfs_waiting			*wap;

	if(DebugProcess(NTO_DEBUG_THREAD_INFO, prp->pid, 0, (union nto_debug_data *)&msg.status) == -1) {
		return MsgError_r(rcvid, errno);
	}
	if((prp->flags & _NTO_PF_TERMING) || (msg.status.flags & _DEBUG_FLAG_STOPPED)) {
		msg.reply.zero = msg.reply.zero2 = msg.reply.ret_val = 0;
		msg.reply.nbytes = sizeof msg.status;
		return MsgReply_r(rcvid, 0, &msg, sizeof msg);
	}

	if(!(wap = _smalloc(sizeof *wap))) {
		return MsgError_r(rcvid, ENOMEM);
	}
	wap->rcvid = rcvid;
	wap->next = prp->debug_waiting;
	prp->debug_waiting = wap;
	return EOK;
}

/* 
 Dis-associate any procfs ocbs from this pid (called during 
 process termination).  After setting the pid to -1 the OCB may 
 be freed (context switch to the procfs close_ocb code) and as 
 a result can no longer be referenced by the prp.
*/
int proc_debug_destroy(resmgr_context_t *ctp, PROCESS *prp) {
	struct procfs_ocb			*ocb;

	while((ocb = prp->ocb_list)) {
		prp->ocb_list = ocb->next;
#ifndef DEBUG
		if(ocb->pid != prp->pid) {
			crash();
		}
#endif
		ocb->pid = -1;	/* Totally invalidate the thing */
	}
	return 0;
}

static int procfs_close_ocb(resmgr_context_t *ctp, void *reserved, void *vocb) {
	struct procfs_ocb			*ocb = vocb;
	struct procfs_ocb			*p, **pp;
	PROCESS						*prp;
   	
	if(ocb->pid > 0 && !(ocb->flags & PROCFS_FLAG_MASK)) {
		if((prp = proc_lock_pid(ocb->pid))) {
			for(pp = &prp->ocb_list; (p = *pp); pp = &p->next) {
				if(p == ocb) {
					*pp = p->next;
					break;
				}
			}

			if(ocb->ioflag & _IO_FLAG_WR) {
				(void)DebugDetach(prp->pid);
			}
			proc_unlock(prp);
		} 
	} 
	_sfree(ocb, sizeof *ocb + ((*ocb->tail) ? strlen(ocb->tail) : 0));
	return EOK;
}

static int procfs_unblock(resmgr_context_t *ctp, io_pulse_t *msg, void *vocb) {
	struct procfs_ocb			*ocb = vocb;
	PROCESS						*prp;

	if(ocb && !(ocb->flags & PROCFS_FLAG_MASK) && (prp = proc_lock_pid(ocb->pid))) {
		struct procfs_waiting		*p, **pp;
		struct _msg_info			info;

		if((MsgInfo(ctp->rcvid, &info) == -1) || !(info.flags & _NTO_MI_UNBLOCK_REQ)) {
			proc_unlock(prp);
			return _RESMGR_NOREPLY;
		}
		for(pp = &prp->debug_waiting; (p = *pp); pp = &p->next) {
			if(p->rcvid == ctp->rcvid) {
				*pp = p->next;
				_sfree(p, sizeof *p);
				break;
			}
		}
		proc_unlock(prp);
	}
	return EINTR;
}

static int procfs_lseek(resmgr_context_t *ctp, io_lseek_t *msg, void *vocb) {
	struct procfs_ocb				*ocb = vocb;
	unsigned						off;

	if(ocb->pid < 0 && !(ocb->flags & PROCFS_FLAG_MASK)) {
		return EIO;
	}
	if(ocb->pid == 0 && (msg->i.whence != SEEK_SET || msg->i.offset)) {
		return EIO;
	}
	switch(msg->i.whence) {
	case SEEK_SET:
		off = msg->i.offset;
		break;
	case SEEK_CUR:
		off = ocb->offset + msg->i.offset;
		break;
	case SEEK_END:
		off = ULONG_MAX + msg->i.offset;
		break;
	default:
		return EINVAL;
	}
	ocb->offset = off;

	if(msg->i.combine_len & _IO_COMBINE_FLAG) {
		return EOK;
	}

	msg->o = off;
	SETIOV(ctp->iov + 0, &msg->o, sizeof msg->o);
	return _RESMGR_NPARTS(1);
}

static int procfs_stat(resmgr_context_t *ctp, io_stat_t *msg, void *vocb) {
	struct procfs_ocb			*ocb = vocb;
	PROCESS						*prp;

	/* Case #1 a pid/AS stat */
	if(ocb->pid && (ocb->flags & PROCFS_FLAG_AS)) {
		struct _procfs_map_info		info;
		uintptr_t					vaddr;

		if(ocb->pid == -1) {
			return EIO;
		}
		if(!(prp = proc_lock_pid(ocb->pid))) {
			return EBADF;
		}
		memset(&msg->o, 0x00, sizeof msg->o);
		msg->o.st_ino = PID_TO_INO(ocb->pid, 0);
		msg->o.st_size = 0;
		if(proc_rlock_adp(prp) != -1) {
			for(vaddr = 0; memmgr.mapinfo(prp, vaddr, &info, NULL, 0, NULL, NULL, NULL); vaddr = info.vaddr + info.size) {
				if(info.flags & MAP_SYSRAM) {
					msg->o.st_size += info.size;
				}
			}
			proc_unlock_adp(prp);
		}
		msg->o.st_gid = prp->cred->info.rgid;
		msg->o.st_uid = prp->cred->info.ruid;
		msg->o.st_mode = S_IFREG | (0666 & ~(procfs_umask));
		msg->o.st_nlink = 1;
		proc_unlock(prp);

	/* Case #2 all other stats */
	} else {
		memset(&msg->o, 0x00, sizeof msg->o);
		msg->o.st_size = 1;
		if(ocb->pid && !(ocb->flags & PROCFS_FLAG_MASK)) {
			msg->o.st_ino = PID_TO_INO(ocb->pid, PROCFS_FLAG_AS);
			msg->o.st_nlink = ocb->pid == ctp->info.pid ? 4 : 2;
			msg->o.st_mode = S_IFDIR | (0555 & ~(procfs_umask));
		} else if(ocb->flags & PROCFS_FLAG_PROC) {
			memsize_t st_size;
			prp = proc_lookup_pid(ctp->info.pid);
			st_size = free_mem(ctp->info.pid, mempart_getid(prp, sys_memclass_id));

			//NYI: What we really should do here is compile with
			//_FILE_OFFSET_BITS=64, but that'll cause code bloat
			//and destabilize things. Something for another day....
			//	bstecher
			msg->o.st_size = st_size;
#if _PADDR_BITS > 32
			msg->o.st_size_hi = st_size >> 32;
#endif
			msg->o.st_ino = ~0;
			msg->o.st_nlink = 2;
			msg->o.st_mode = S_IFDIR | 0555;
		} else {
			if ((ocb->flags & (PROCFS_FLAG_MOUNT|PROCFS_FLAG_DIR)) == PROCFS_FLAG_MOUNT) {
				msg->o.st_ino = ocb->pid;
				msg->o.st_mode = S_IFREG | 0444;
				msg->o.st_nlink = 1;
			} else {
				msg->o.st_ino = ocb->pid;
				msg->o.st_mode = S_IFDIR | 0555;
				msg->o.st_nlink = 2;
			}
		}
	}
	msg->o.st_ctime =
	msg->o.st_mtime =
	msg->o.st_atime = time(0);
	msg->o.st_dev = (ctp->info.srcnd << ND_NODE_BITS) | procfs_devno;
	msg->o.st_rdev = 0;

	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

/*
 * this function will create a private mapping from a shared mapping
 * The mapped address will be the same as for the shared mapping
 * but will be a private copy of <size> bytes
*/
static int memmgr_shared_to_private( PROCESS *prp, uintptr_t addr, size_t size) {
	procfs_mapinfo			info;
	struct {
		procfs_debuginfo	debug;
		char				buf[_POSIX_PATH_MAX + 1];
	}						dinfo;
	struct {
		struct _mem_debug_info	debug;
		char					buf[_POSIX_PATH_MAX + 1];
	}						minfo;
	void					*tmap, *nmap;
	size_t					map_size;
	int						status;
	part_id_t			mpid;

	size_t  r = memmgr.mapinfo(prp, addr & ~(__PAGESIZE-1), &info,
														&dinfo.debug, sizeof(dinfo) - offsetof(procfs_debuginfo, path),
														NULL, NULL, NULL);
	if ((r == 0) || ((uintptr_t)info.vaddr != (addr & ~(__PAGESIZE-1))) || (size > info.size)) {
		return -1;
	}

	/*
	 * create a temporary anoymous mapping for the page to be privatized
	 * and copy the shared page to it
	*/
	proc_lock_owner_mark(prp);
	ProcessBind(prp->pid);
	status = memmgr.mmap( 
		prp,
		0,
		size,
		PROT_WRITE|PROT_READ,
		MAP_PRIVATE|MAP_ANON,
		NULL, /* object */
		0, /* offset */
		0, /* align */
		0, /* prealloc */
		NOFD,
		&tmap,
		&map_size,
		mpid = mempart_getid(prp, sys_memclass_id)
	);
	if (status != EOK || MAP_FAILED == (void *)tmap || map_size != size) {
		ProcessBind(0);
		errno = status;
		return -1;
	}
	else
	{
		memcpy(tmap, (void *)((uintptr_t)info.vaddr), map_size);
	}

	/*
	 * now, unmap the shared page that will be privatized and create a new
	 * private mapping at the same address as the previous shared mapping
	 * and copy the original contents to it (from the temp mapping)
	*/
	status = memmgr.munmap(prp, info.vaddr, size, 0, mpid);
	if ( status != EOK ) {
		/* release temporary mapping and return a failure */
		(void)memmgr.munmap(prp, (uintptr_t)tmap, map_size, 0, mpid);
		ProcessBind(0);
		return -1;
	}

	status = memmgr.mmap( 
		prp,
		info.vaddr, /* address */
		size,
		(info.flags & PROT_MASK) | PROT_WRITE,
		(info.flags & ~(MAP_RESERVMASK|PG_MASK|PROT_MASK| MAP_SHARED)) | MAP_PRIVATE | MAP_ANON | MAP_FIXED,
		NULL, /* object */
		0, /* offset */
		0, /* align */
		0, /* prealloc */
		NOFD,
		&nmap,
		&map_size,
		mpid
	);
	if (status != EOK || nmap != (void *)((uintptr_t)info.vaddr) || map_size != size) {
		/* release temporary mapping and return a failure */
		(void)memmgr.munmap(prp, (uintptr_t)tmap, map_size, 0, mpid);
		ProcessBind(0);
		return -1;
	}
	else
	{
		memcpy(nmap, tmap, map_size);
	}

	/*
	 * now, release the temporary mapping and restablish the original protection
	 * attributes on the newly privatized page
	*/
	status = memmgr.munmap(prp, (uintptr_t)tmap, size, 0, mpid);
	if (status != EOK) {
		ProcessBind(0);
		return -1;
	}
	status = memmgr.mprotect(prp, (uintptr_t)nmap, map_size, (info.flags & PROT_MASK));
	if (status != EOK) {
		ProcessBind(0);
		return -1;
	}

	/* finally, copy the debug info */
	memset(&minfo, 0, sizeof(minfo.debug));
	minfo.debug.type = _MEM_DEBUG_INFO;
	minfo.debug.vaddr = (uintptr_t)nmap;
	minfo.debug.size = map_size;
	minfo.debug.flags = (info.flags & ~(MAP_RESERVMASK|PG_MASK|PROT_MASK| MAP_SHARED)) | MAP_PRIVATE;
	minfo.debug.old_vaddr = dinfo.debug.vaddr;
	STRLCPY(minfo.debug.path, dinfo.debug.path, sizeof(minfo) - offsetof(struct _mem_debug_info, path));

	(void)memmgr.debuginfo(prp, &minfo.debug);

	/* Phew! */
	ProcessBind(0);
	return 0;
}

static int procfs_regset_read( struct procfs_ocb *ocb, PROCESS *prp, int regset_id, void *buf, int *nbytes )
{
	switch(regset_id) {
	case REGSET_GPREGS:
		if(DebugProcess(NTO_DEBUG_GET_GREG, ocb->pid, ocb->tid, buf) == -1) {
			return errno;
		}
		*nbytes = sizeof(CPU_REGISTERS);
		return EOK;
	default:
		if(!(prp = proc_lock_pid(ocb->pid))) {
			return ESRCH;
		}
		switch(regset_id) {
		case REGSET_FPREGS:
			ProcessBind(ocb->pid);
			if(DebugProcess(NTO_DEBUG_GET_FPREG, ocb->pid, ocb->tid, buf) == -1) {
				ProcessBind(0);
				return proc_error(errno, prp);
			}
			ProcessBind(0);
			*nbytes = sizeof(FPU_REGISTERS);
			break;
		case REGSET_ALTREGS:
			ProcessBind(ocb->pid);
			if(DebugProcess(NTO_DEBUG_GET_ALTREG, ocb->pid, ocb->tid, buf) == -1) {
				ProcessBind(0);
				return proc_error(errno, prp);
			}
			ProcessBind(0);
			*nbytes = sizeof(ALT_REGISTERS);
			break;
		case REGSET_PERFREGS:
			ProcessBind(ocb->pid);
			if(DebugProcess(NTO_DEBUG_GET_PERFREG, ocb->pid, ocb->tid, buf) == -1) {
				ProcessBind(0);
				return proc_error(errno, prp);
			}
			ProcessBind(0);
			*nbytes = sizeof(PERF_REGISTERS);
			break;
		default:
			return proc_error(EINVAL, prp);
		}
	}
	return proc_error(EOK, prp);
}

static int procfs_regset_write( struct procfs_ocb *ocb, PROCESS *prp, int regset_id, void *buf, int *nbytes )
{
	switch(regset_id) {
	/* The first set of registers don't need any locking */
	case REGSET_GPREGS:
		if(DebugProcess(NTO_DEBUG_SET_GREG, ocb->pid, 0, buf) == -1) {
			return errno;
		}
		*nbytes = sizeof(CPU_REGISTERS);
		return EOK;
	default:
		if(!(prp = proc_lock_pid(ocb->pid))) {
			return ESRCH;
		}
		switch(regset_id) {
		case REGSET_FPREGS:
			ProcessBind(ocb->pid);
			if(DebugProcess(NTO_DEBUG_SET_FPREG, ocb->pid, 0, buf) == -1) {
				ProcessBind(0);
				return proc_error(errno, prp);
			}
			ProcessBind(0);
			*nbytes = sizeof(FPU_REGISTERS);
			break;
		case REGSET_ALTREGS:
			ProcessBind(ocb->pid);
			if(DebugProcess(NTO_DEBUG_SET_ALTREG, ocb->pid, 0, buf) == -1) {
				ProcessBind(0);
				return proc_error(errno, prp);
			}
			ProcessBind(0);
			*nbytes = sizeof(ALT_REGISTERS);
			break;
		case REGSET_PERFREGS:
			ProcessBind(ocb->pid);
			if(DebugProcess(NTO_DEBUG_SET_PERFREG, ocb->pid, 0, buf) == -1) {
				ProcessBind(0);
				return proc_error(errno, prp);
			}
			ProcessBind(0);
			*nbytes = 0;
			break;
		default:
			return proc_error(EINVAL, prp);
		}
	}
	return proc_error(EOK, prp);
}

static int procfs_devctl(resmgr_context_t *ctp, io_devctl_t *msg, void *vocb) {
	struct procfs_ocb			*ocb = vocb;
	union {
		int							oflag;
		int							mountflag;
		procfs_sysinfo				sysinfo;
		procfs_info					info;
		procfs_mapinfo				mapinfo;
		procfs_debuginfo			mapdebug;
		procfs_signal				signal;
		procfs_status				status;
		procfs_run					run;
		procfs_break				brk;
		procfs_greg					greg;
		procfs_fpreg				fpreg;
		procfs_altreg				altreg;
		procfs_irq					irq;
		procfs_timer				timer;
		procfs_regset				regset;
		procfs_threadctl			threadctl;
		procfs_channel				channel;
		struct sigevent				event;
		uint32_t					flags;
		pthread_t					tid;
		part_id_t				mempart_id;
		part_list_t				mpart_list;
	}							*ioctl = (void *)(msg + 1);
	int							nbytes = 0;
	PROCESS						*prp;
	int							ret_val = 0;
	int							combine = msg->i.combine_len & _IO_COMBINE_FLAG;

	switch((unsigned)msg->i.dcmd) {
	case DCMD_ALL_GETFLAGS:
	case DCMD_ALL_SETFLAGS:
	case DCMD_ALL_GETMOUNTFLAGS:
		break;

	case DCMD_PROC_SYSINFO:
		// syspage data is not endian-safe; client (pidin) must swap!
		// [alternative is to fail this EENDIAN as below]
#if 0
		if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			return EENDIAN;
		}
#endif
		break;

	//A process is needed, we do validation elsewhere
	case DCMD_PROC_INFO:
	case DCMD_PROC_CURTHREAD:
	case DCMD_PROC_GETGREG:
	case DCMD_PROC_GETFPREG:
	case DCMD_PROC_GETALTREG:
	case DCMD_PROC_GETREGSET:
	case DCMD_PROC_STATUS:
	case DCMD_PROC_TIDSTATUS:
	case DCMD_PROC_GET_BREAKLIST:
	case DCMD_PROC_MAPDEBUG:
	case DCMD_PROC_MAPDEBUG_BASE:
	case DCMD_PROC_MAPINFO:
	case DCMD_PROC_PAGEDATA:
	case DCMD_PROC_TIMERS:
	case DCMD_PROC_CHANNELS:
	case DCMD_PROC_IRQS:
	case DCMD_PROC_THREADCTL:	//Verify permission later
	case DCMD_PROC_GET_MEMPART_LIST:
	case DCMD_PROC_ADD_MEMPARTID:
	case DCMD_PROC_DEL_MEMPARTID:
	case DCMD_PROC_CHG_MEMPARTID:
		if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			return EENDIAN;
		}
		if (!ocb || (ocb->flags & (PROCFS_FLAG_LINK | PROCFS_FLAG_MOUNT))) {
			return EINVAL ;
		}
		break;

	default:

		// The external check hook can/should reject EENDIAN if necessary;
		// the default internal handling is that this is unsupported
		// by procfs unless special-cased in the above switch/case.
		if (procfs_devctl_check_hook) {
			ret_val = procfs_devctl_check_hook(ctp, msg, ocb);
			if ( !(ret_val == EOK || ret_val == ENOSYS) ) {
				return ret_val;
			}
		}
		if ( procfs_devctl_check_hook == NULL || ret_val == ENOSYS ) {
			// The external check hook can/should reject EENDIAN if necessary;
			// the default internal handling is that this is unsupported
			// by procfs unless special-cased in the above switch/case.
			if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
				 return EENDIAN;
			}
			if(!ocb || (ocb->flags & PROCFS_FLAG_MASK)) {
				return ENOSYS;
			}
			// We require access to be able to perform
			if(!(ocb->ioflag & _IO_FLAG_WR)) {
				 return EBADF;
			}
		}
	    
		break;
	}
		
	prp = 0;
	switch((unsigned)msg->i.dcmd) {
	case DCMD_ALL_GETFLAGS:
		ioctl->oflag = ocb ? (ocb->ioflag & ~O_ACCMODE) | ((ocb->ioflag - 1) & O_ACCMODE) : O_RDONLY;
		nbytes = sizeof ioctl->oflag;
		break;

	case DCMD_ALL_SETFLAGS:
		if(ocb) {
			ocb->ioflag = (ocb->ioflag & ~O_SETFLAG) | (ioctl->oflag & O_SETFLAG);
		}
		break;

	case DCMD_ALL_GETMOUNTFLAGS: {
		ioctl->mountflag = 0;
		nbytes = sizeof ioctl->mountflag;
		break;
	}

	case DCMD_PROC_SYSINFO:
		ioctl = (void *)_syspage_ptr;
		ret_val = nbytes = _syspage_ptr->total_size;
		break;

	case DCMD_PROC_INFO:
		if(DebugProcess(NTO_DEBUG_PROCESS_INFO, ocb->pid, 0, (union nto_debug_data *)&ioctl->info) == -1) {
			return errno;
		}
		nbytes = sizeof ioctl->info;
		break;

	case DCMD_PROC_FREEZETHREAD:
		if(DebugProcess(NTO_DEBUG_FREEZE, ocb->pid, ioctl->tid, 0) == -1) {
			return errno;
		}
		break;

	case DCMD_PROC_THAWTHREAD:
		if(DebugProcess(NTO_DEBUG_THAW, ocb->pid, ioctl->tid, 0) == -1) {
			return errno;
		}
		break;

	case DCMD_PROC_SET_FLAG:
		if(DebugProcess(NTO_DEBUG_SET_FLAG, ocb->pid, 0, (union nto_debug_data *)&ioctl->flags) == -1) {
			return errno;
		}
		break;
			
	case DCMD_PROC_CLEAR_FLAG:
		if(DebugProcess(NTO_DEBUG_CLEAR_FLAG, ocb->pid, 0, (union nto_debug_data *)&ioctl->flags) == -1) {
			return errno;
		}
		break;

	case DCMD_PROC_RUN:
		if(ioctl->run.flags & _DEBUG_RUN_ARM) {
			ioctl->run.flags &= ~_DEBUG_RUN_ARM;
			ocb->rcvid = ctp->rcvid;
		}
		if(DebugProcess(NTO_DEBUG_RUN, ocb->pid, 0, (union nto_debug_data *)&ioctl->run) == -1) {
			ocb->rcvid = 0;
			return errno;
		}
		break;

	case DCMD_PROC_CURTHREAD:
		if(ocb->ioflag & _IO_FLAG_WR) {
			if((ret_val = DebugProcess(NTO_DEBUG_CURTHREAD, ocb->pid, ioctl->tid, 0)) == -1) {
				return errno;
			}
		} else {
			ocb->tid = ioctl->tid;
		}
		break;

	case DCMD_PROC_EVENT:
		ocb->event = ioctl->event;
		break;

	case DCMD_PROC_STATUS:
		ioctl->status.tid = ocb->tid;		// Set to use current thread
		/* Fall Through */
	case DCMD_PROC_TIDSTATUS:
		nbytes = sizeof ioctl->status;
		if(DebugProcess(NTO_DEBUG_THREAD_INFO, ocb->pid, ioctl->status.tid, (union nto_debug_data *)&ioctl->status) == -1) {
			return errno;
		}
		break;

	case DCMD_PROC_SIGNAL:
		if(SignalKill(ND_LOCAL_NODE, ocb->pid, ioctl->signal.tid, ioctl->signal.signo, ioctl->signal.code, ioctl->signal.value) == -1) {
			return errno;
		}
		break;

	case DCMD_PROC_GETREGSET:
		if ( (ret_val = procfs_regset_read( ocb, prp, ioctl->regset.id, ioctl->regset.buf, &nbytes )) != EOK ) {
			return ret_val;
		}
		nbytes += sizeof(ioctl->regset.id);
		ret_val = nbytes;
		break;

	case DCMD_PROC_SETREGSET:
		if ( (ret_val = procfs_regset_write( ocb, prp, ioctl->regset.id, ioctl->regset.buf, &nbytes )) != EOK ) {
			return ret_val;
		}
		nbytes += sizeof(ioctl->regset.id);
		ret_val = nbytes;
		break;

	case DCMD_PROC_GETGREG:
#ifndef NDEBUG
		if(sizeof *msg + sizeof(CPU_REGISTERS) > ctp->msg_max_size) {
			crash();
		}
#endif
		if ( (ret_val = procfs_regset_read( ocb, prp, REGSET_GPREGS, &ioctl->greg, &nbytes )) != EOK ) {
			return ret_val;
		}
		ret_val = nbytes;
		break;

	case DCMD_PROC_SETGREG:
#ifndef NDEBUG
		if(sizeof *msg + sizeof(CPU_REGISTERS) > ctp->msg_max_size) {
			crash();
		}
#endif
		if ( (ret_val = procfs_regset_write( ocb, prp, REGSET_GPREGS, &ioctl->greg, &nbytes )) != EOK ) {
			return ret_val;
		}
		ret_val = nbytes;
		break;

	case DCMD_PROC_GETFPREG:
#ifndef NDEBUG
		if(sizeof *msg + sizeof(FPU_REGISTERS) > ctp->msg_max_size) {
			crash();
		}
#endif
		if ( (ret_val = procfs_regset_read( ocb, prp, REGSET_FPREGS, &ioctl->fpreg, &nbytes )) != EOK ) {
			return ret_val;
		}
		ret_val = nbytes;
		break;

	case DCMD_PROC_SETFPREG:
#ifndef NDEBUG
		if(sizeof *msg + sizeof(FPU_REGISTERS) > ctp->msg_max_size) {
			crash();
		}
#endif
		if ( (ret_val = procfs_regset_write( ocb, prp, REGSET_FPREGS, &ioctl->fpreg, &nbytes )) != EOK ) {
			return ret_val;
		}
		ret_val = nbytes;
		break;

	case DCMD_PROC_GETALTREG:
#ifndef NDEBUG
		if(sizeof *msg + sizeof(ALT_REGISTERS) > ctp->msg_max_size) {
			crash();
		}
#endif
		if ( (ret_val = procfs_regset_read( ocb, prp, REGSET_ALTREGS, &ioctl->altreg, &nbytes )) != EOK ) {
			return ret_val;
		}
		ret_val = nbytes;
		break;

	case DCMD_PROC_SETALTREG:
#ifndef NDEBUG
		if(sizeof *msg + sizeof(ALT_REGISTERS) > ctp->msg_max_size) {
			crash();
		}
#endif
		if ( (ret_val = procfs_regset_write( ocb, prp, REGSET_ALTREGS, &ioctl->altreg, &nbytes )) != EOK ) {
			return ret_val;
		}
		ret_val = nbytes;
		break;

	default:
		if (procfs_devctl_hook) {
			ret_val = procfs_devctl_hook(ctp, msg, ocb);
			if ( ret_val != ENOSYS ) {
				return ret_val;
			}
			ret_val = EOK;
		}
		if(!(prp = proc_lock_pid(ocb->pid))) {
			return ESRCH;
		}
		switch((unsigned)msg->i.dcmd) {
		case DCMD_PROC_THREADCTL:
			//Reply with as much data as we can from the structure. We don't read any more than
			//the thread buffer worth for performance so adjust other data as required.  This
			//will reply with the same ioctl->threadctl that was passed in (ie tid/cmd in the
  			//top 8 bytes) due to the way the Kerext call is made.

			if(msg->i.nbytes > ctp->msg_max_size) {	
				nbytes = ctp->msg_max_size;
			} else {
				nbytes = msg->i.nbytes; 
			}

			//Switch access based on sub-command
			switch(ioctl->threadctl.cmd) {
			case _NTO_TCTL_NAME: {
				struct _thread_name *tn = (struct _thread_name *)&ioctl->threadctl.data;
				int lenadj = ((char *)tn - (char *)msg) + sizeof(*tn);

				if(tn->name_buf_len > (ctp->msg_max_size - lenadj)) {
					tn->name_buf_len = ctp->msg_max_size - lenadj;
				}	

				if(tn->new_name_len >= 0 && !(ocb->ioflag & _IO_FLAG_WR)) {
					return proc_error(EBADF, prp);
				}
				}
				break;

			case _NTO_TCTL_RUNMASK_GET_AND_SET:
				if(*(unsigned *)ioctl->threadctl.data != 0 &&
				    (ocb->ioflag & _IO_FLAG_WR) == 0) {
					/*
					 * Can't alter without
					 * write perms.
					 */
					return proc_error(EBADF, prp);
				}
				break;

			case _NTO_TCTL_RUNMASK_GET_AND_SET_INHERIT: {
				unsigned *rmaskp;
				int rsize, *rsizep, i;

				if((ocb->ioflag & _IO_FLAG_WR) != 0) {
					break;
				}

				rsize = RMSK_SIZE(NUM_PROCESSORS);
				rsizep = (int *)ioctl->threadctl.data;
				rmaskp = (unsigned *)(rsizep + 1);
				/*
				 * Validate what they've passed in up
				 * to what we're interested in.  The
				 * rsize they've passed in is itself
				 * validated later.
				 */
				rsize = min(rsize, *rsizep);
				rsize = max(rsize, 0);
				/* to catch both runmask and inherit mask */
				rsize *= 2;
				for(i = 0; i < rsize; i++) {
					if(rmaskp[i] != 0) {
						/*
						 * Can't alter without
						 * write perms.
						 */
						return proc_error(EBADF, prp);
					}
				}
				break;
				}

			default:
				if(!(ocb->ioflag & _IO_FLAG_WR)) {
					return proc_error(EBADF, prp);
				}	
				break;
			}

			ret_val = KerextThreadCtl( prp, ioctl->threadctl.tid, ioctl->threadctl.cmd, ioctl->threadctl.data );
			if ( ret_val == -1 ) {
				return proc_error(errno, prp);
			}
			break;

		case DCMD_PROC_MAPDEBUG_BASE:
		{
			size_t space;
			ioctl->mapdebug.vaddr = prp->base_addr;
			if ( prp->debug_name != NULL ) {
				if (msg->i.nbytes <= offsetof(procfs_debuginfo, path)) {
					return proc_error(EINVAL, prp);
				}
				space = min(ctp->msg_max_size - sizeof(*msg), msg->i.nbytes);
				CRASHCHECK(space < offsetof(procfs_debuginfo, path));
				STRLCPY(ioctl->mapdebug.path, prp->debug_name, space  - offsetof(procfs_debuginfo, path));
				msg->o.ret_val = 0;
				SETIOV(ctp->iov + 0, &msg->o, sizeof msg->o + offsetof(procfs_debuginfo, path) + strlen(ioctl->mapdebug.path) + 1);
				return proc_error(_RESMGR_NPARTS(1), prp);
			} else {
				return proc_error(ENXIO, prp);
			}
			/* NOTREACHED */
			break;
		}

		case DCMD_PROC_MAPDEBUG:
		{
			size_t space;
			if(proc_rlock_adp(prp) == -1) {
				return proc_error(ENXIO, prp);
			}
			if (msg->i.nbytes <= offsetof(procfs_debuginfo, path)) {
				return proc_error(EINVAL, prp);
			}
			space = min(ctp->msg_max_size - sizeof(*msg), msg->i.nbytes);
			CRASHCHECK(space < offsetof(procfs_debuginfo, path));
			if(memmgr.mapinfo(prp, ioctl->mapdebug.vaddr, NULL, &ioctl->mapdebug, space - offsetof(procfs_debuginfo, path), NULL, NULL, NULL) == 0)
			{
				proc_unlock_adp(prp);
				return proc_error(ENXIO, prp);
			}
			proc_unlock_adp(prp);
			msg->o.ret_val = 0;
			SETIOV(ctp->iov + 0, &msg->o, sizeof msg->o + offsetof(procfs_debuginfo, path) + strlen(ioctl->mapdebug.path) + 1);
			return proc_error(_RESMGR_NPARTS(1), prp);
		}

		case DCMD_PROC_PAGEDATA:
		case DCMD_PROC_MAPINFO: {
			procfs_mapinfo					*p;
			int								num;
			uintptr_t						vaddr;
			size_t							size;
			unsigned						offset;
			int								dcmd = msg->i.dcmd;

			offset = sizeof msg->o;
			vaddr = 0;
			size = 0;
			if(proc_rlock_adp(prp) == -1) {
				return proc_error(ENXIO, prp);
			}
			do {
				p = &ioctl->mapinfo;
				nbytes = 0;
				num = (ctp->msg_max_size - sizeof *msg) / sizeof *p;
				while(num-- && (size = memmgr.mapinfo(prp, vaddr, p, NULL, 0, NULL, NULL, NULL))) {
					ret_val++;
					nbytes += sizeof *p;
					vaddr = (uintptr_t)p->vaddr + p->size;
					if(dcmd == DCMD_PROC_MAPINFO) {
						p->flags &= ~PG_MASK;
					}
					p++;
				}
				if(nbytes) {
					// Unlock to avoid deadlock
					proc_unlock_adp(prp);
					if(resmgr_msgwrite(ctp, &ioctl->mapinfo, nbytes, offset) == -1) {
						return proc_error(errno, prp);
					}
					offset += nbytes;
					if(proc_rlock_adp(prp) == -1) {
						return proc_error(ENXIO, prp);
					}
				}
			} while(size);
			proc_unlock_adp(prp);
			nbytes = 0;
			break;
		}

		case DCMD_PROC_IRQS: {
			procfs_irq						*p;
			struct interrupt_query_entry	entry, *entryp = NULL;
			int								num;
			unsigned						offset;
			unsigned						id;

			offset = sizeof msg->o;
			id = 0;
			do {
				p = &ioctl->irq;
				nbytes = 0;
				num = (ctp->msg_max_size - sizeof *msg) / sizeof *p;
				while(num && 
						((entryp = QueryObject(_QUERY_INTERRUPT, id, 0, 0, &id, &entry, sizeof(entry))) != NULL)) {
					if (entry.pid == prp->pid) {
						p->pid = entry.pid;
						p->tid = entry.tid;
						p->handler = entry.info.handler;
						p->area = entry.info.area;
						p->flags = entry.info.flags;
						p->level = entry.info.level;
						p->mask_count = entry.info.mask_count;
						p->id = entry.info.id;
						p->vector = entry.vector;
						p->event = entry.event;

						ret_val++;
						nbytes += sizeof *p;
						num--;
						p++;
					}
					id++;
				}
				if(nbytes) {
					if(resmgr_msgwrite(ctp, &ioctl->irq, nbytes, offset) == -1) {
						return proc_error(errno, prp);
					}
					offset += nbytes;
				}
			} while(entryp != NULL);
			nbytes = 0;
			break;
		}

		case DCMD_PROC_TIMERS: {
			procfs_timer					*p;
			int								num;
			unsigned						offset;
			int								id;

			offset = sizeof msg->o;
			id = 0;
			do {
				p = &ioctl->timer;
				nbytes = 0;
				num = (ctp->msg_max_size - sizeof *msg) / sizeof *p;
				while(num-- && 
						((id = TimerInfo(prp->pid, id, _NTO_TIMER_SEARCH, &p->info)) != -1)) {
					p->id = id;
					if (p->info.event.sigev_notify == SIGEV_SIGNAL_CODE && p->info.event.sigev_code == SI_TIMER) { /* ???? Not safe.  Shouldn't there be a way of remembering this better? */
					    p->info.event.sigev_notify = SIGEV_SIGNAL;
					    p->info.event.sigev_code = 0;
					}
					ret_val++;
					nbytes += sizeof *p;
					p++;
					id++;
				}
				if(nbytes) {
					if(resmgr_msgwrite(ctp, &ioctl->timer, nbytes, offset) == -1) {
						return proc_error(errno, prp);
					}
					offset += nbytes;
				}
			} while(id != -1);
			nbytes = 0;
			break;
		}

		case DCMD_PROC_CHANNELS: {
			procfs_channel					*p;
			CHANNEL							chan, *chp = NULL;
			int								num;
			unsigned						offset;
			unsigned						id;

			offset = sizeof msg->o;
			id = 0;
			do {
				p = &ioctl->channel;
				nbytes = 0;
				num = (ctp->msg_max_size - sizeof *msg) / sizeof *p;
				while(num && 
	((chp = QueryObject(_QUERY_PROCESS, prp->pid, _QUERY_PROCESS_CHANCONS, id, &id, &chan, sizeof(chan))) != NULL )) {
					if ( chp->type == TYPE_CHANNEL ) {

						(void) DebugChannel( chp, p );

						ret_val++;
						nbytes += sizeof *p;
						num--;
						p++;
					}
					id++;
				}
				if(nbytes) {
					if(resmgr_msgwrite(ctp, &ioctl->channel, nbytes, offset) == -1) {
						return proc_error(errno, prp);
					}
					offset += nbytes;
				}
			} while(chp != NULL);
			break;
		 }

		case DCMD_PROC_STOP:
			if(DebugProcess(NTO_DEBUG_STOP, ocb->pid, 0, 0) == -1) {
				return proc_error(errno, prp);
			}
			/* Fall Through */
		case DCMD_PROC_WAITSTOP:
			(void)procfs_waitstop(prp, ctp->rcvid);
			return proc_error(_RESMGR_NOREPLY, prp);

		case DCMD_PROC_BREAK: {
			procfs_mapinfo					info;

			if(ioctl->brk.type & _DEBUG_BREAK_SOFT) {
				// This flag is for output only to /proc user
				return proc_error(EINVAL, prp);
			}

			// Make sure address is valid, and get info on region
			if(proc_rlock_adp(prp) == -1) {
				return proc_error(ENXIO, prp);
			}
			if(memmgr.mapinfo(prp, ioctl->brk.addr, &info, NULL, 0, NULL, NULL, NULL) == 0
					|| (uintptr_t)info.vaddr > ioctl->brk.addr) {
				proc_unlock_adp(prp);
				return proc_error(ENXIO, prp);
			}

			if((NUM_PROCESSORS > 1) && (ioctl->brk.type & _DEBUG_BREAK_EXEC)) {

				// If breakpoint is in a shared region, we need to fix it.
				if((info.flags & MAP_TYPE) == MAP_SHARED) {

					// If possible make a private mapping overtop of the
					// shared mapping.
					if((info.flags & PROT_WRITE) ||
						/* must upgrade to a address space write lock */
						(proc_rlock_promote_adp(prp) == -1) ||
						memmgr_shared_to_private(prp, ioctl->brk.addr, __PAGESIZE) != EOK) {

						proc_unlock_adp(prp);
						return proc_error(EINVAL, prp);
					}
				}
			}

			// Make sure debug call has the address space available...
			ProcessBind(ocb->pid);
			if(DebugProcess(NTO_DEBUG_BREAK, ocb->pid, 0, (union nto_debug_data *)&ioctl->brk) == -1) {
				ret_val = errno;
				ProcessBind(0);
				proc_unlock_adp(prp);
				return proc_error(ret_val, prp);
			}
			ProcessBind(0);
			proc_unlock_adp(prp);
			nbytes = sizeof ioctl->brk;
			break;
		}

		case DCMD_PROC_GET_BREAKLIST: {
			debug_breaklist_t 	breaklist;
			int					offset;
		
			nbytes = msg->i.nbytes;		//Number of bytes user has ready
			ret_val = 0;				//Total number of breakpoints (set later)
			offset = 0;					

			do {
				breaklist.offset = offset;
				breaklist.breakpoints = &ioctl->brk;

				if(ctp->msg_max_size - sizeof(*msg) < nbytes) {
					breaklist.count = (ctp->msg_max_size - sizeof(*msg)) / sizeof(*breaklist.breakpoints);
				} else {
					breaklist.count = nbytes / (int)sizeof(*breaklist.breakpoints);
				}

				//Make sure we have addressing and lock down of structures we need
				if(proc_rlock_adp(prp) == -1) {
					return proc_error(ENXIO, prp);
				}
				ProcessBind(ocb->pid);
				if(DebugProcess(NTO_DEBUG_GET_BREAKLIST, ocb->pid, 0, (union nto_debug_data*)&breaklist) == -1) {
					ret_val = errno;
					ProcessBind(0);
					proc_unlock_adp(prp);
					return proc_error(ret_val, prp);
				}
				ProcessBind(0);
				proc_unlock_adp(prp);
				
				//Fill in the total number of breakpoints to return to the user 
				if(offset == 0) {
					ret_val = breaklist.total_count;
				}

				// if we have no breakpoints to write then return
				if(breaklist.count == 0) {
					break;
				}

				//Reply to the user with the data as we collect it
				if(resmgr_msgwrite(ctp, breaklist.breakpoints, 
										breaklist.count * sizeof(*breaklist.breakpoints), 
									    sizeof(msg->o) + offset * sizeof(*breaklist.breakpoints)) == -1) {
					return proc_error(errno, prp);
				}

				nbytes -= breaklist.count * sizeof(*breaklist.breakpoints);
				offset += breaklist.count;	

			} while((offset < ret_val) && (nbytes >= sizeof(*breaklist.breakpoints)));

			//We did our own msgwrites() to put out the data, just return the total count
			nbytes = 0;
			break;
		}

		case DCMD_PROC_GET_MEMPART_LIST:
		{
			/*
			 * ioctl->mpart_list.num_entries contains the number of entries that
			 * the caller has provided space for. Upon return,
			 * Upon return, ioctl->mpart_list.num_entries will be set as follows
			 * 	- if the actual number of entries is <= the requested number
			 * 	  then the actual number will be filled in and returned
			 * 	- if the actual number of entries is > the requested number then
			 * 	  ioctl->mpart_list.num_entries will be filled in and a negative
			 * 	  value for the remainder will be returned.
			*/
			int  r;
			int  n = ioctl->mpart_list.num_entries;
	
			if (!MEMPART_INSTALLED())
				return proc_error(ENOSYS, prp);

			if (n < 0)
				return proc_error(EINVAL, prp);

			r = MEMPART_GETLIST(prp, &ioctl->mpart_list, n, mempart_flags_t_GETLIST_ALL, NULL);
			ret_val = nbytes = PART_LIST_T_SIZE(ioctl->mpart_list.num_entries);
			if (r > 0) {
				ioctl->mpart_list.num_entries = -r;
			}

			break;
		}
		
		/* add/remove a parition association
		 * FIX ME - what happens if I delete a previous partition association
		 * and objects still exist which were accounted against that partition.
		 * I know that soul and heap allocations will be released back to their
		 * proper list/heap, but what about memory allocated with mmap() that
		 * needs to be munmap()'d. Do I account back to the partition properly
		 * even if the process releasing the allocation (via munmap() is no
		 * longer associated with the partition? Need to check this out as it
		 * may be the reason for partition deletion without process/object
		 * associations that still have a size > 0
		*/
		case DCMD_PROC_ADD_MEMPARTID:
		case DCMD_PROC_DEL_MEMPARTID:
		{
			int  r;

			if (!MEMPART_INSTALLED()) {
				return proc_error(ENOSYS, prp);
			}

			if ((r = MEMPART_VALIDATE_ID(ioctl->mempart_id)) == EOK)
			{
				/* FIX ME - should not be able to disassociate from a partitions
				 * for which objects have been already created. This will lead
				 * to incredible confusion. Also, currently this operation
				 * applies only to the calling process */
				if ((unsigned)msg->i.dcmd == DCMD_PROC_DEL_MEMPARTID)
				{
					if (mempart_get_classid(ioctl->mempart_id) == sys_memclass_id) {
						r = EPERM;
					}
					else {
						r = ProcessDisassociate(prp, ioctl->mempart_id);
					}
				}
				else
				{
					/* we don't consider reassociationing with the same partition an error */
					if ((r = ProcessAssociate(prp, ioctl->mempart_id, part_flags_NONE)) == EALREADY) {
						r = EOK;
					}
				}
			}
			return proc_error(r, prp);
		}
		
		/*
		 * change which partition a process is associated with
		 * 
		 * Note that when a partition was originally associated, a heap was also
		 * implicitly associated as well as a soul list for each soul type. Any
		 * memory explicitly allocated or souls that were allocated whether from
		 * a shared soul list/heap or not will have been accounted to the original
		 * partition association. Once the association changes, new allocations
		 * will be accounted to the new partition but all remamining objects will
		 * continue to be accounted to the previous partition until they are
		 * released. 
		*/
		case DCMD_PROC_CHG_MEMPARTID:
		{
			int  r;

			if (!MEMPART_INSTALLED()) {
				return proc_error(ENOSYS, prp);
			}

			if ((r = MEMPART_VALIDATE_ID(ioctl->mempart_id)) == EOK)
			{
				if (mempart_getid(prp, mempart_get_classid(ioctl->mempart_id)) == NULL) {
					r = EINVAL;
				}
				r = ProcessReassociate(prp, ioctl->mempart_id, part_flags_NONE);
			}
			return proc_error(r, prp);
		}

		default:
			return proc_error(ENOSYS, prp);
		}

	}

	if(combine && ret_val == 0 && nbytes == 0) {
		return proc_error(EOK, prp);
	}

	msg->o.ret_val = ret_val;
	if((void *)ioctl != (void *)(msg + 1)) {
		SETIOV(ctp->iov + 0, &msg->o, sizeof msg->o);
		SETIOV(ctp->iov + 1, ioctl, nbytes);
		return proc_error(_RESMGR_NPARTS(2), prp);
	} else {
		SETIOV(ctp->iov + 0, &msg->o, sizeof msg->o + nbytes);
		return proc_error(_RESMGR_NPARTS(1), prp);
	}
}

static const resmgr_io_funcs_t procfs_io_funcs = {
	_RESMGR_IO_NFUNCS,
	procfs_read,
	procfs_write,
	procfs_close_ocb,
	procfs_stat,
	0,
	procfs_devctl,
	procfs_unblock,
	0,
	procfs_lseek
};

static int procfs_open(resmgr_context_t *ctp, io_open_t *msg, void *handle, void *extra) {
	struct procfs_ocb			*ocb;
	char						*path;
	pid_t						pid;
	PROCESS						*prp;
	unsigned					flags;

    if (msg->connect.path_len + sizeof(*msg) >= ctp->msg_max_size) {
		return ENAMETOOLONG;
	}

	// If opening "/proc" succed with an ocb of NULL
	pid = 0;
	prp = 0;
	flags = 0;
	if(*(path = msg->connect.path)) {
		if(!strncmp(path, "self", 4)) {
			if(ND_NODE_CMP(ND_LOCAL_NODE, ctp->info.nd) != 0) {
				return ENOENT;
			}
			path += 4;
			pid = ctp->info.pid;
			flags |= PROCFS_FLAG_SELF;
#ifdef SUPPORT_LINK
		} else if (!strncmp(path, "link", 4)) {
			path += 4;
			flags |= PROCFS_FLAG_LINK;
#endif
		} else if (!strncmp(path, "mount", 5)) {
			path += 5;
			flags |= PROCFS_FLAG_MOUNT;
		} else if (*path < '0' || *path > '9' || (pid = strtoul(path, &path, 10)) == 0) {
			return ENOENT;
		}

		if (pid) {
			if(!strncmp(path, "/as", 3)) {
				flags |= PROCFS_FLAG_AS;
				path += 3;
				if(*path == '/' || (*path == '\0' && (msg->connect.eflag & _IO_CONNECT_EFLAG_DIR))) {
					return ENOTDIR;
				}
			}
			if((pid == ctp->info.pid) && (ND_NODE_CMP(ND_LOCAL_NODE, ctp->info.nd)) == 0) {
				flags |= PROCFS_FLAG_SELF;
			}
			if(*path) {
				return ENOENT;
			}
			if(!(prp = proc_lock_pid(pid))) {
				return ENOENT;
			}
			/* This is a weak attempt to validate permissions */
			if(flags & PROCFS_FLAG_AS) {
				int			  mode;

				mode = 0;
				if(msg->connect.ioflag & _IO_FLAG_RD) {
					mode |= S_IREAD;
				}
				if(msg->connect.ioflag & _IO_FLAG_WR) {
					mode |= S_IWRITE;
				}
				if (mode) {
					struct _client_info info;
					iofunc_attr_t attr;

					iofunc_attr_init(&attr, S_IFREG|(0666 & ~(procfs_umask)), NULL, NULL);
					attr.uid = prp->cred->info.euid;
					attr.gid = prp->cred->info.egid;
					if (ConnectClientInfo(ctp->info.scoid, &info, NGROUPS_MAX) != EOK ||
						iofunc_check_access(ctp, &attr, mode, &info) != EOK) {
						return proc_error(EPERM, prp);
					}
				}
			}
		} else if (flags & (PROCFS_FLAG_LINK | PROCFS_FLAG_MOUNT)) {
			int					ret;
			void				*handlep;

			if ((ret = proc_pathmgr_validate(ctp, msg, flags, path, &handlep)) != EOK) {
				return ret;
			}
			if (S_ISDIR(msg->connect.mode)) {
				flags |= PROCFS_FLAG_DIR;
			}
			pid = INODE_XOR(handlep); 
		} else if (*path) {
			return ENOENT;
		}
	} else {
		flags |= PROCFS_FLAG_PROC;
	}

	if(!(ocb = _scalloc(sizeof *ocb + ((path && *path) ? strlen(path) : 0)))) {
		return proc_error(ENOMEM, prp);
	}

	ocb->pid = pid; //PID value is used as inode
	ocb->ioflag = msg->connect.ioflag;
	ocb->event.sigev_notify = SIGEV_NONE;
	ocb->flags = flags;
	ocb->tail[0] = '\0';
	if (path && *path) {
		memcpy(ocb->tail, path, strlen(path) + 1);
	}

	if(ocb->ioflag & _IO_FLAG_WR) {
		struct procfs_ocb			*ocb2;

		if(!prp || !(ocb->flags & PROCFS_FLAG_AS) || (prp->flags & _NTO_PF_ZOMBIE)) {
			_sfree(ocb, sizeof *ocb);
			return proc_error(EINVAL, prp);
		}
		
		for(ocb2 = prp->ocb_list; ocb2; ocb2 = ocb2->next) {
			if(ocb2->ioflag & _IO_FLAG_WR) {
				_sfree(ocb, sizeof *ocb);
				return proc_error(EBUSY, prp);
			}
		}

		if(!(flags & PROCFS_FLAG_SELF) && prp->pid != SYSMGR_PID && DebugAttach(prp->pid, _DEBUG_FLAG_RLC) == -1) {
			_sfree(ocb, sizeof *ocb);
			return proc_error(errno, prp);
		}
	}

	if(resmgr_open_bind(ctp, ocb, 0) == -1) {
		if(ocb->ioflag & _IO_FLAG_WR) {
			CRASHCHECK(prp == NULL);
			(void)DebugDetach(prp->pid);
		}
		_sfree(ocb, sizeof *ocb);
		return proc_error(errno, prp);
	}

	if(prp) {
		ocb->next = prp->ocb_list;
		prp->ocb_list = ocb;
	}

	return proc_error(EOK, prp);
}

static const resmgr_connect_funcs_t procfs_connect_funcs = {
	_RESMGR_CONNECT_NFUNCS,
	procfs_open,
};                                              

static int pulse(message_context_t *ctp, int code, unsigned flags, void *handle) {
	PROCESS						*prp;
	union sigval				value = ctp->msg->pulse.value;

	if((prp = proc_lock_pid(value.sival_int))) {
		struct procfs_waiting	*wap;
		struct procfs_ocb		*ocb;

		for(ocb = prp->ocb_list; ocb; ocb = ocb->next) {
			if(ocb->rcvid) {
				int	rcvid = ocb->rcvid;

				ocb->rcvid = 0; //clear before DeliverEvent to avoid race
				(void)MsgDeliverEvent(rcvid, &ocb->event);
			}
		}
		if((wap = prp->debug_waiting)) {
			prp->debug_waiting = wap->next;
			(void)procfs_waitstop(prp, wap->rcvid);
			_sfree(wap, sizeof *wap);
		}
		proc_unlock(prp);
	}
	return 0;
}

void procfs_init(void) {

	resmgr_attach(dpp, NULL, "/proc", _FTYPE_ANY, _RESMGR_FLAG_DIR,
		&procfs_connect_funcs, &procfs_io_funcs, 0);
	DebugInstall(pulse_attach(dpp, MSG_FLAG_ALLOC_PULSE, 0, pulse, NULL));
	rsrcdbmgr_proc_devno(_MAJOR_FSYS, &procfs_devno, -1, 0);
}

__SRCVERSION("procfs.c $Rev: 209169 $");
