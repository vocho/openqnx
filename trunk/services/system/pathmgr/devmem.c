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

#define _LARGEFILE64_SOURCE 1
#include "externs.h"
#include <fcntl.h>
#include <share.h>
#include <sys/dcmd_all.h>
#include <sys/procfs.h>
#include "pathmgr_node.h"
#include "pathmgr_object.h"
#include "pathmgr_proto.h"

// Interface break - we really shouldn't be referencing mm fields here,
// but it's silly to go through the extra hassle.
#define OBJECT_SIZE(o)	((o)->mem.mm.size)

static int					mem_handle;
static dev_t				mem_devno;

struct mem_ocb {
	unsigned					pos;
	int							ioflag;
	int							tflags; // for typed memory objects
	union object				*object;
};

typedef struct {
	io_write_t	write;
	struct _xtype_offset	offset;
} io_pwrite_t;

typedef struct {
	io_read_t read;
	struct _xtype_offset	offset;
} io_pread_t;


static int 
mem_read_write(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, struct mem_ocb *ocb) {
	void					*addr;
	unsigned				nbytes;
	unsigned				size;
	PROCESS					*proc;
	union object			*obp;
	unsigned				*pos;
	unsigned				offset;
	size_t					skip;

	// IO_READ and IO_WRITE messages exactly overlay, as does XTYPE_OFFSET!

	if(!ocb) {
		nbytes = 0;
		goto ok;
	}
	if(!(ocb->ioflag & ((msg->type == _IO_READ) ? _IO_FLAG_RD : _IO_FLAG_WR))) {
		return EBADF;
	}
	nbytes = msg->read.i.nbytes;

	obp = ocb->object;
	if((msg->type == _IO_WRITE) && (ocb->ioflag & O_APPEND) && obp) {
		ocb->pos = OBJECT_SIZE(obp);
	}

	switch (msg->read.i.xtype & _IO_XTYPE_MASK)
	{
    	case _IO_XTYPE_NONE:
		    pos = &ocb->pos;
			skip = sizeof(msg->read.i);
			break;
	    case _IO_XTYPE_OFFSET:
			offset = ((io_pread_t*)msg)->offset.offset;
			pos = &offset;
			skip = sizeof(io_pread_t);
			break;
		default: /* this really shouldn't happen, but just in case */
		    return ENOSYS;
	}

	proc = sysmgr_prp;
	size = obp ? OBJECT_SIZE(obp) : UINT_MAX;
	if( *pos > size - nbytes || *pos + nbytes > size) {
		if(obp && msg->type == _IO_WRITE) {
			int					status;

			status = memmgr.resize(obp, *pos + nbytes);
			if(status != EOK) {
				return status;
			}
			size = OBJECT_SIZE(obp);
		}
		if(*pos >= size) {
			nbytes = 0;
		} else {
			nbytes = size - *pos;
		}
	}

	if(nbytes) {
		int						status, err;
		int						flags;
		part_id_t			mpid;

		if(proc_thread_pool_reserve() != 0) {
			return EAGAIN;
		}
		ProcessBind(SYSMGR_PID);

		proc_wlock_adp(proc);
		flags = MAP_SHARED;
		if(obp != NULL) {
			proc_mux_lock(&obp->mem.mm.mux);
			mpid = obp->hdr.mpid;
		} else {
			flags |= MAP_PHYS;
			mpid = part_id_t_INVALID;
		}
		status = memmgr.mmap(proc, 0, nbytes, PROT_READ|PROT_WRITE, flags,
						obp, *pos, 0, nbytes, NOFD, &addr, &size, mpid);
		if(obp != NULL) proc_mux_unlock(&obp->mem.mm.mux);
		proc_unlock_adp(proc);
		if(status != EOK) {
			ProcessBind(0);
			proc_thread_pool_reserve_done();
			return status;
		}

		SETIOV(ctp->iov + 0, addr, nbytes);

		if(msg->type == _IO_READ) {
			status = resmgr_msgwritev(ctp, ctp->iov + 0, 1, 0);
		} else {
			status = resmgr_msgreadv(ctp, ctp->iov + 0, 1, skip);
		}
		err = errno;

		proc_wlock_adp(proc);
		(void) memmgr.munmap(proc, (uintptr_t)addr, size, 0, mempart_getid(proc, sys_memclass_id));
		proc_unlock_adp(proc);

		if(status == -1) {
			ProcessBind(0);
			proc_thread_pool_reserve_done();
			return err;
		}

		*pos += nbytes;
		if(obp && msg->type == _IO_WRITE) {
			obp->mem.pm.mtime = time(0);
		}
		ProcessBind(0);
		proc_thread_pool_reserve_done();
	}

ok:
	_IO_SET_READ_NBYTES(ctp, nbytes);
	return EOK;
}


static int 
mem_read(resmgr_context_t *ctp, io_read_t *msg, void *vocb) {
	return mem_read_write(ctp, (resmgr_iomsgs_t *)msg, (struct mem_ocb *)vocb);
}


static int 
mem_write(resmgr_context_t *ctp, io_write_t *msg, void *vocb) {
	return mem_read_write(ctp, (resmgr_iomsgs_t *)msg, (struct mem_ocb *)vocb);
}


static int 
mem_close_ocb(resmgr_context_t *ctp, void *reserved, void *vocb) {
	struct mem_ocb			*ocb = vocb;

	if(ocb) {
		OBJECT				*obp;

		if((obp = ocb->object)) {
			//RUSH3: Should get rid of memmgr_tymem_close() and
			//RUSH3: do everything it does in tymem_close()
			if(obp->hdr.type == OBJECT_MEM_TYPED) {
				memmgr_tymem_close(obp);
			}
			pathmgr_object_done(obp);
		}
		_sfree(ocb, sizeof *ocb);
	}
	return EOK;
}


static int 
mem_lseek(resmgr_context_t *ctp, io_lseek_t *msg, void *vocb) {
	struct mem_ocb			*ocb = vocb;
	unsigned				pos;
	unsigned				size;

	if(ocb == NULL) {
		return EOK;
	}
	pos = (off_t)msg->i.offset;
	switch(msg->i.whence) {
	case SEEK_SET:
		break;
	case SEEK_CUR:
		pos = ocb->pos + pos;
		break;
	case SEEK_END:
		size = ocb->object ? OBJECT_SIZE(ocb->object) : UINT_MAX;
		pos = size + pos;
		break;
	default:
		return EINVAL;
	}
	ocb->pos = pos;
	if(msg->i.combine_len & _IO_COMBINE_FLAG) {
		return EOK;
	}
	msg->o = ocb->pos;
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}


static int 
mem_stat(resmgr_context_t *ctp, io_stat_t *msg, void *vocb) {
	struct mem_ocb			*ocb = vocb;
	union object			*obp = ocb ? ocb->object : 0;
	size_t					size = ocb ? (obp ? OBJECT_SIZE(obp) : UINT_MAX) : 0;

	memset(&msg->o, 0x00, sizeof msg->o);
	msg->o.st_size = size;
	msg->o.st_nblocks = (size + (__PAGESIZE-1)) / __PAGESIZE;
	msg->o.st_blocksize = msg->o.st_blksize = __PAGESIZE;
	msg->o.st_ctime =
	msg->o.st_mtime =
	msg->o.st_atime =(obp ? obp->mem.pm.mtime : time(0));
	msg->o.st_ino = obp ? (uintptr_t)INODE_XOR(obp) : 1;
	if(ocb && obp) {
		msg->o.st_uid = obp->mem.pm.uid;
		msg->o.st_gid = obp->mem.pm.gid;
		msg->o.st_mode = (obp->mem.pm.mode != 0) ? obp->mem.pm.mode : S_IFNAM | S_IPERMS;
		msg->o.st_nlink = obp->hdr.refs;
		msg->o.st_dev = path_devno;
		msg->o.st_rdev = S_ISNAM(msg->o.st_mode) ? S_INSHD : 0;
	} else {
		//The /dev/mem device is set to rw root only, the /dev/shmem
		//device is set to rwx for all, since it is un-enforced. 
		msg->o.st_mode = ocb ? S_IFREG | 0600 : S_IFDIR | 0777;
		msg->o.st_nlink = ocb ? 1 : 2;
		msg->o.st_dev = mem_devno;
		msg->o.st_rdev = 0;
	}
	msg->o.st_dev |= (ctp->info.srcnd << ND_NODE_BITS);
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}


static int 
mem_devctl(resmgr_context_t *ctp, io_devctl_t *msg, void *vocb) {
	struct mem_ocb				*ocb = vocb;
	OBJECT						*obp;
	union {
		int						oflag;
		int						mountflag;
		dcmd_memmgr_memobj		memobj;
	}						*data = (void *)(msg + 1);
	unsigned				nbytes = 0;

	switch((unsigned)msg->i.dcmd) {
	case DCMD_ALL_GETFLAGS:
		data->oflag = (ocb != NULL) ? (ocb->ioflag & ~O_ACCMODE) | ((ocb->ioflag - 1) & O_ACCMODE) : O_RDWR;
		nbytes = sizeof data->oflag;
		break;

	case DCMD_ALL_SETFLAGS:
		if(ocb) {
			ocb->ioflag = (ocb->ioflag & ~O_SETFLAG) | (data->oflag & O_SETFLAG);
		}
		break;

	case DCMD_ALL_GETMOUNTFLAGS: 
		data->mountflag = 0;
		nbytes = sizeof data->mountflag;
		break;

	default:
		if(ocb == NULL) return _RESMGR_DEFAULT;
		obp = ocb->object;
		if(obp == NULL) return _RESMGR_DEFAULT;
		return object_devctl(ctp, msg, obp);
	}

	if(nbytes) {
		msg->o.ret_val = 0;
		return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o + nbytes);
	}
	return EOK;
}


static int 
mem_utime(resmgr_context_t *ctp, io_utime_t *msg, void *vocb) {
	struct mem_ocb				*ocb = vocb;
	union object				*obp;

	if(ocb && (obp = ocb->object)) {
		if(msg->i.cur_flag) {
			obp->mem.pm.mtime = time(0);
		} else {
			obp->mem.pm.mtime = msg->i.times.modtime;
		}
	}
	return EOK;
}


static int 
mem_space(resmgr_context_t *ctp, io_space_t *msg, void *vocb) {
	struct mem_ocb				*ocb = vocb;
	unsigned					start, end;
	union object				*obj;
	int							status;

	if(!ocb || !(obj = ocb->object)) {
		return _RESMGR_DEFAULT;
	}
	start = (off_t)msg->i.start;
	switch(msg->i.whence) {
	case SEEK_SET:
		break;
	case SEEK_CUR:
		start = ocb->pos + start;
		break;
	case SEEK_END:
		start =	OBJECT_SIZE(ocb->object) + start;
		break;
	default:
		return EINVAL;
	}

	/* 
	 This is an ENORMOUS HUGE BIG UGLY HACK!  We take in 64 bit offsets
	 but all of the memory objects and memory functions use unsigned int
	 values for their counters and sized so that if we just pass the 64 
	 bit values directly then bad things tend to happen.  This is a stop-gap 
	 measure until we fix that.
	*/
	{
		uint64_t lend;
		lend = msg->i.len ? start + msg->i.len : OBJECT_SIZE(obj);
		if(lend > UINT_MAX) {
			return ENOMEM;
		}
	}

	end = msg->i.len ? start + msg->i.len : OBJECT_SIZE(obj);

	switch(msg->i.subtype) {
	case F_ALLOCSP64:
		if(end > OBJECT_SIZE(obj)) {
			status = memmgr_resize(obj, end);
			if(status != EOK) {
				return status;
			}
		}
		break;
	case F_FREESP64:
		if(end < OBJECT_SIZE(obj) || !(ocb->ioflag & _IO_FLAG_WR)) {
			return EINVAL;
		}
		if(start < OBJECT_SIZE(obj)) {
			status = memmgr_resize(obj, start);
			if(status != EOK) {
				return status;
			}
		}
		break;
	default:
		return EINVAL;
	}

	msg->o = OBJECT_SIZE(obj);
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}


static int 
mem_change_attr(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, void *vocb) {
	iofunc_ocb_t		nocb;
	iofunc_attr_t		attr;
	struct mem_ocb		*ocb = vocb;
	union object		*obp;
	int					ret;

	if(!ocb || !(obp = ocb->object) || (!S_ISNAM(obp->mem.pm.mode) && !S_ISREG(obp->mem.pm.mode))) {
		return _RESMGR_DEFAULT;
	}

	memset(&nocb, 0, sizeof(nocb));
	nocb.ioflag = ocb->ioflag;
	memset(&attr, 0, sizeof(attr));
	attr.mode = obp->mem.pm.mode, attr.gid = obp->mem.pm.gid, attr.uid = obp->mem.pm.uid;

	switch(msg->type) {
	case _IO_CHOWN:
		if((ret = iofunc_chown(ctp, &msg->chown, &nocb, &attr)) == EOK) {
			obp->mem.pm.gid = attr.gid;
			obp->mem.pm.uid = attr.uid;
		}
		break;
	case _IO_CHMOD:
		if((ret = iofunc_chmod(ctp, &msg->chmod, &nocb, &attr)) == EOK) {
			obp->mem.pm.mode = attr.mode;
		}
		break;
	default:
		ret = _RESMGR_DEFAULT;
		break;
	}

	return ret;
}


static int 
mem_chown(resmgr_context_t *ctp, io_chown_t *msg, void *vocb) {
	return mem_change_attr(ctp, (resmgr_iomsgs_t *)msg, vocb);
}


static int 
mem_chmod(resmgr_context_t *ctp, io_chmod_t *msg, void *vocb) {
	return mem_change_attr(ctp, (resmgr_iomsgs_t *)msg, vocb);
}


static const resmgr_io_funcs_t mem_io_funcs = {
	_RESMGR_IO_NFUNCS,
	mem_read,
	mem_write,
	mem_close_ocb,
	mem_stat,
	0,
	mem_devctl,
	0,
	0,
	mem_lseek,
	mem_chmod,
	mem_chown,
	mem_utime,
	0,
	0,
	0,
	mem_space
};


int 
devmem_check_perm(resmgr_context_t *ctp, mode_t mode, uid_t uid, gid_t gid, int check) {
	struct _client_info info;
	iofunc_attr_t		attr;

	ConnectClientInfo(ctp->info.scoid, &info, NGROUPS_MAX);
	memset(&attr, 0, sizeof(attr));
	attr.mode = mode, attr.gid = gid, attr.uid = uid;

	return iofunc_check_access(ctp, &attr, check, &info);
} 


int 
mem_open(resmgr_context_t *ctp, io_open_t *msg, void *handle, void *extra) {
	OBJECT			*object = handle;
 	struct mem_ocb	*ocb;

	if(object) {
		/* Only set the ownership the first time around  */
		if(msg->connect.ioflag & O_CREAT && !object->mem.pm.mode) {
			struct _client_info info;
			ConnectClientInfo(ctp->info.scoid, &info, 0);
			object->mem.pm.uid = info.cred.euid;
			object->mem.pm.gid = info.cred.egid;
			object->mem.pm.mode = ((msg->connect.file_type == _FTYPE_SHMEM) ? S_IFNAM : S_IFREG) | (msg->connect.mode & ~S_IFMT);
		/* Only check perms on things which we have set permission on ... */
		} else if (object->mem.pm.mode != 0) {
			int ret = 0;
			ret |= (msg->connect.ioflag & _IO_FLAG_RD) ? S_IRUSR : 0;
			ret |= (msg->connect.ioflag & _IO_FLAG_WR) ? S_IWUSR : 0;
			if(ret != 0) {
				ret = devmem_check_perm(ctp, object->mem.pm.mode, 
							object->mem.pm.uid, object->mem.pm.gid, ret);
			}
			if(ret != 0) {
				return ret;
			}
		}

		if((msg->connect.ioflag & O_TRUNC) && OBJECT_SIZE(object)) {
			int				status;

			status = memmgr.resize(object, 0);
			if(status != 0) {
				return status;
			}
		}
	} else if((msg->connect.ioflag & (_IO_FLAG_RD | _IO_FLAG_WR)) && !proc_isaccess(0, &ctp->info)) {
		return EPERM;
	}

#if 0 /* This breaks spawn ... need to check to see if the is proper behaviour */
	/* Check our access mode all opens are assumed as read/write */
	if (object) {	
		int sflag = msg->connect.sflag & SH_MASK;

		if(((sflag == SH_DENYRW || sflag == SH_DENYRD) && object->mem.count > 1 /*rcount*/) ||
           ((sflag == SH_DENYRW || sflag == SH_DENYWR) && object->mem.count > 1 /*wcount*/)) {
			return EBUSY;
		}
	}
#endif


	if(!(ocb = _scalloc(sizeof *ocb))) {
		return ENOMEM;
	}

	ocb->ioflag = msg->connect.ioflag;
	ocb->object = (object != NULL) ? pathmgr_object_clone(object) : NULL;


	ctp->id = mem_handle;
	if(resmgr_open_bind(ctp, ocb, &mem_io_funcs) == -1) {
		if(object) {
			pathmgr_object_done(object);
		}
		return errno;
	}

	return EOK;
}


static const resmgr_connect_funcs_t mem_connect_funcs = {
	_RESMGR_CONNECT_NFUNCS,
	mem_open
};


static int 
shmem_open(resmgr_context_t *ctp, io_open_t *msg, void *handle, void *extra) {
	if (msg->connect.path_len + sizeof(*msg) >= ctp->msg_max_size) {
		return ENAMETOOLONG;
	}

	if((msg->connect.eflag & _IO_CONNECT_EFLAG_DIR) || msg->connect.path[0] == '\0') {
		if(msg->connect.ioflag & _IO_FLAG_WR) {
			return EISDIR;
		}
		if(resmgr_open_bind(ctp, 0, 0) == -1) {
			return errno;
		}
		return EOK;
	}
		
	_IO_SET_CONNECT_RET(ctp, _IO_CONNECT_RET_FTYPE);
	msg->ftype_reply.status = EOK;
	msg->ftype_reply.file_type = (msg->connect.file_type == _FTYPE_SHMEM) ? _FTYPE_SHMEM : _FTYPE_FILE;
	return(_RESMGR_PTR(ctp, msg, sizeof(struct _io_connect_ftype_reply)));
}


static const resmgr_connect_funcs_t shmem_connect_funcs = {
	_RESMGR_CONNECT_NFUNCS,
	shmem_open
};


//RUSH3: Instead of this, use "mem_io_funcs" and error check in the
//RUSH3: functions for OBJECT_MEM_TYPED?
static const resmgr_io_funcs_t tymem_io_funcs = {
	_RESMGR_IO_NFUNCS,
	0,
	0,
	mem_close_ocb,
	mem_stat,
	0,
	mem_devctl,
	0,
	0,
	0,
	mem_chmod,
	mem_chown,
	mem_utime,
	0,
	0,
	0,
	0
};


int 
tymem_open(resmgr_context_t *ctp, io_open_t *msg, void *handle, void *extra) {
	struct mem_ocb	*ocb;
	unsigned		tflags;
	unsigned		count;

	if (msg->connect.path_len + sizeof(*msg) >= ctp->msg_max_size) {
		return ENAMETOOLONG;
	}

	if((msg->connect.eflag & _IO_CONNECT_EFLAG_DIR) || msg->connect.path[0] == '\0') {
		if(msg->connect.ioflag & _IO_FLAG_WR) {
			return EISDIR;
		}
		//RUSH3: Simply fakes "/dev/tymem" as empty copy of "/dev/shmem".
		if(resmgr_open_bind(ctp, 0, &mem_io_funcs) == -1) {
			return errno;
		}
		return EOK;
	}
	if(msg->connect.extra_type != _IO_CONNECT_EXTRA_TYMEM) {
		return ENOTSUP;
	}
	if(msg->connect.extra_len != sizeof(uint32_t)) {
		return ENOTSUP;
	}
	if(msg->connect.ioflag & ~(O_ACCMODE | O_CREAT | O_NOCTTY)) {
		return EINVAL;
	}

#define ALL_TMEM	(POSIX_TYPED_MEM_ALLOCATE|POSIX_TYPED_MEM_ALLOCATE_CONTIG|POSIX_TYPED_MEM_MAP_ALLOCATABLE)
	tflags = *(uint32_t *)extra;
	count = 0;
	if(tflags & POSIX_TYPED_MEM_ALLOCATE) ++count;
	if(tflags & POSIX_TYPED_MEM_ALLOCATE_CONTIG) ++count;
	if(tflags & POSIX_TYPED_MEM_MAP_ALLOCATABLE) ++count;
	if((count > 1) || (tflags & ~ALL_TMEM)) {
		return EINVAL;
	}
	if(!(tflags & (POSIX_TYPED_MEM_ALLOCATE|POSIX_TYPED_MEM_ALLOCATE_CONTIG))) {
		if(!proc_isaccess(0, &ctp->info)) {
			// Only root processes are allowed to specify direct offsets
			return EPERM;
		}
	}

	ocb = _scalloc(sizeof *ocb);
	if(ocb == NULL) {
		errno = ENOMEM;
		goto fail1;
	}

	ocb->tflags = tflags;
	ocb->ioflag = msg->connect.ioflag;

	errno = memmgr_tymem_open(msg->connect.path, ocb->tflags, &ocb->object, proc_lookup_pid(ctp->info.pid));
	if(errno != EOK) goto fail2;

	//RUSH1: need permission checking on path

	ctp->id = mem_handle;
	if(resmgr_open_bind(ctp, ocb, &tymem_io_funcs) == -1) goto fail3;

	pathmgr_object_clone(ocb->object);
	return EOK;

fail3:
	memmgr_tymem_close(ocb->object);

fail2:
	_sfree(ocb, sizeof(*ocb));

fail1:
	return errno;
}


static const resmgr_connect_funcs_t tymem_connect_funcs = {
	_RESMGR_CONNECT_NFUNCS,
	tymem_open,
};


void 
devmem_init(void) {
	resmgr_attr_t	rattr;

	// allocate mem directory entry
	memset(&rattr, 0x00, sizeof(rattr));
	rattr.flags = RESMGR_FLAG_CROSS_ENDIAN;

	mem_handle = resmgr_attach(dpp, NULL, "/dev/mem", 0, 0, &mem_connect_funcs, &mem_io_funcs, 0);
	resmgr_attach(dpp, &rattr, "/dev/shmem", 0, _RESMGR_FLAG_DIR, &shmem_connect_funcs, &mem_io_funcs, 0);
	//RUSH3: Decide if "/dev/tymem" should be a fully usable directory
	//RUSH3: structure
	resmgr_attach(dpp, NULL, "/dev/tymem", _FTYPE_TYMEM, _RESMGR_FLAG_DIR, &tymem_connect_funcs, &tymem_io_funcs, 0);
	rsrcdbmgr_proc_devno("dev", &mem_devno, -1, 0);
}


int 
devmem_check(const resmgr_io_funcs_t *funcs, mem_map_t *msg, void *handle, OBJECT **pobp) {
	struct mem_ocb	*ocb = handle;

	if(funcs == &tymem_io_funcs) {
		// Typed memory
		if(msg != NULL) {
			unsigned	flags = 0;

			if(ocb->tflags & POSIX_TYPED_MEM_ALLOCATE) {
				flags |= IMAP_TYMEM_ALLOCATE;
			}
			if(ocb->tflags & POSIX_TYPED_MEM_ALLOCATE_CONTIG) {
				flags |= IMAP_TYMEM_ALLOCATE_CONTIG;
			}
			if(ocb->tflags & POSIX_TYPED_MEM_MAP_ALLOCATABLE) {
				flags |= IMAP_TYMEM_MAP_ALLOCATABLE;
			}
			msg->i.flags |= flags;
		}
	} else if(funcs != &mem_io_funcs) {
		// not "/dev/mem" or "/dev/shmem/..."
		return -1;
	}
	*pobp = ocb->object;

	if(msg != NULL) {
		if((ocb->ioflag & _IO_FLAG_RD) == 0) {
			return EACCES;
		}
		if((msg->i.flags & MAP_TYPE) == MAP_SHARED) {
			if((ocb->ioflag & _IO_FLAG_WR) == 0) {
				if(msg->i.prot & PROT_WRITE) {
					//RUSH3: Is this test needed anymore - vmm_mmap() should catch it now
					return EACCES;
				}
				// Indicate that we can't allow PROT_WRITE later...
				msg->i.flags |= IMAP_OCB_RDONLY;
			}
			if(*pobp == NULL) {
				msg->i.flags |= MAP_PHYS;
			}

		}
	}

	return EOK;
}


int 
devmem_check_ocb_phys(resmgr_context_t *ctp, void *vocb) {
	struct mem_ocb *	ocb = vocb;
	struct _msg_info	info;

	return ((_resmgr_iofuncs(ctp, &info) == &mem_io_funcs) && (ocb->object != NULL));
}

__SRCVERSION("devmem.c $Rev: 198540 $");
