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


#define _FILE_OFFSET_BITS	64
#define _LARGEFILE64_SOURCE	1

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <gulliver.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/dcmd_all.h>
#include <sys/dcmd_blk.h>
#include <sys/dcmd_chr.h>
#include <sys/iomsg.h>
#include <sys/neutrino.h>
#include <sys/pathmsg.h>
#include <sys/resmgr.h>
#include <sys/siginfo.h>
#include <unistd.h>

#define ALIGNMSG(_b, _o, _i) (void *)((char *)_b + (((_o) + (_i) + _QNX_MSG_ALIGN - 1) & ~(_QNX_MSG_ALIGN - 1)))

static __inline void ENDIAN_SWAPFLOCK(struct flock *flocklocal)
{
	ENDIAN_SWAP16(&flocklocal->l_type);
	ENDIAN_SWAP16(&flocklocal->l_whence);
	ENDIAN_SWAP64(&flocklocal->l_start);
	ENDIAN_SWAP64(&flocklocal->l_len);
	ENDIAN_SWAP32(&flocklocal->l_pid);
	ENDIAN_SWAP32(&flocklocal->l_sysid);
}

static __inline void ENDIAN_SWAPSTAT(struct stat *statlocal)
{
	ENDIAN_SWAP64(&statlocal->st_ino);
	ENDIAN_SWAP64(&statlocal->st_size);
	ENDIAN_SWAP32(&statlocal->st_dev);
	ENDIAN_SWAP32(&statlocal->st_rdev);
	ENDIAN_SWAP32(&statlocal->st_uid);
	ENDIAN_SWAP32(&statlocal->st_gid);
	ENDIAN_SWAP32(&statlocal->st_mtime);
	ENDIAN_SWAP32(&statlocal->st_atime);
	ENDIAN_SWAP32(&statlocal->st_ctime);
	ENDIAN_SWAP32(&statlocal->st_mode);
	ENDIAN_SWAP32(&statlocal->st_nlink);
	ENDIAN_SWAP32(&statlocal->st_blocksize);
	ENDIAN_SWAP32(&statlocal->st_nblocks);
	ENDIAN_SWAP32(&statlocal->st_blksize);
	ENDIAN_SWAP64(&statlocal->st_blocks);
}

static __inline void ENDIAN_SWAPREADDIR(struct dirent *dp, int nbytes)
{
struct dirent_extra_stat	*extra;
int							n;

	while (nbytes > 0 && (n = dp->d_reclen) > 0) {
		for (extra = (struct dirent_extra_stat *)_DEXTRA_FIRST(dp); _DEXTRA_VALID(extra, dp); 
				extra = (struct dirent_extra_stat *)_DEXTRA_NEXT(extra)) {
			if (extra->d_type == _DTYPE_STAT || extra->d_type == _DTYPE_LSTAT)
				ENDIAN_SWAPSTAT(&extra->d_stat);
		}
		ENDIAN_SWAP64(&dp->d_ino);
		ENDIAN_SWAP64(&dp->d_offset);
		ENDIAN_SWAP16(&dp->d_reclen);
		ENDIAN_SWAP16(&dp->d_namelen);
		dp = (struct dirent *)((char *)dp + n);
		nbytes -= n;
	}
}

static __inline void ENDIAN_SWAPSTATVFS(struct statvfs *stv)
{
	ENDIAN_SWAP32(&stv->f_bsize);
	ENDIAN_SWAP32(&stv->f_frsize);
	ENDIAN_SWAP64(&stv->f_blocks);
	ENDIAN_SWAP64(&stv->f_bfree);
	ENDIAN_SWAP64(&stv->f_bavail);
	ENDIAN_SWAP64(&stv->f_files);
	ENDIAN_SWAP64(&stv->f_ffree);
	ENDIAN_SWAP64(&stv->f_favail);
	ENDIAN_SWAP32(&stv->f_fsid);
	ENDIAN_SWAP32(&stv->f_flag);
	ENDIAN_SWAP32(&stv->f_namemax);
}

static __inline void ENDIAN_SWAPSIGEVENT(struct sigevent *event)
{
	ENDIAN_SWAP32(&event->sigev_notify);
	ENDIAN_SWAP32(&event->sigev_signo);
	ENDIAN_SWAP32(&event->sigev_value.sival_int);
	if (SIGEV_GET_TYPE(event) != SIGEV_THREAD) {
		ENDIAN_SWAP16(&event->sigev_code);
		ENDIAN_SWAP16(&event->sigev_priority);
	}
	else {
		ENDIAN_SWAP32(&event->sigev_notify_attributes);
	}
}

static __inline void ENDIAN_SWAPMSGINFO(struct _msg_info *info)
{
	ENDIAN_SWAP32(&info->nd);
	ENDIAN_SWAP32(&info->srcnd);
	ENDIAN_SWAP32(&info->pid);
	ENDIAN_SWAP32(&info->tid);
	ENDIAN_SWAP32(&info->chid);
	ENDIAN_SWAP32(&info->scoid);
	ENDIAN_SWAP32(&info->coid);
	ENDIAN_SWAP32(&info->msglen);
	ENDIAN_SWAP32(&info->srcmsglen);
	ENDIAN_SWAP32(&info->dstmsglen);
	ENDIAN_SWAP16(&info->priority);
	ENDIAN_SWAP16(&info->flags);
}

static __inline void ENDIAN_SWAPXTYPE(int xtype, void *data)
{
	switch (xtype) {
	case _IO_XTYPE_READCOND:
		ENDIAN_SWAP32(&((struct _xtype_readcond *)data)->min);
		ENDIAN_SWAP32(&((struct _xtype_readcond *)data)->time);
		ENDIAN_SWAP32(&((struct _xtype_readcond *)data)->timeout);
		break;
	case _IO_XTYPE_OFFSET:
		ENDIAN_SWAP64(&((struct _xtype_offset *)data)->offset);
		break;
	default: break;
	}
}

static __inline void ENDIAN_SWAPLINK(struct _io_connect_link_reply *lnk, struct _io_connect_entry *entry)
{
int		n;

	for (n = 0; n < lnk->nentries; ++n) {
		ENDIAN_SWAP32(&entry->nd);
		ENDIAN_SWAP32(&entry->pid);
		ENDIAN_SWAP32(&entry->chid);
		ENDIAN_SWAP32(&entry->handle);
		ENDIAN_SWAP32(&entry->key);
		ENDIAN_SWAP32(&entry->file_type);
		ENDIAN_SWAP16(&entry->prefix_len);
		++entry;
	}
	ENDIAN_SWAP32(&lnk->file_type);
	ENDIAN_SWAP16(&lnk->chroot_len);
	ENDIAN_SWAP32(&lnk->umask);
	ENDIAN_SWAP16(&lnk->nentries);
	ENDIAN_SWAP16(&lnk->path_len);
}

static __inline void ENDIAN_SWAPLINKXTRA(struct _io_connect *msg, io_link_extra_t *extra)
{
	switch (msg->extra_type) {
	case _IO_CONNECT_EXTRA_LINK:
		ENDIAN_SWAPMSGINFO(&extra->info);
		break;
	case _IO_CONNECT_EXTRA_RESMGR_LINK:
		ENDIAN_SWAP32(&extra->resmgr.nd);
		ENDIAN_SWAP32(&extra->resmgr.pid);
		ENDIAN_SWAP32(&extra->resmgr.chid);
		ENDIAN_SWAP32(&extra->resmgr.handle);
		ENDIAN_SWAP32(&extra->resmgr.flags);
		ENDIAN_SWAP32(&extra->resmgr.file_type);
		break;
	default: break;
	}
}

static __inline void ENDIAN_SWAPMOUNTXTRA(struct _io_connect *msg, io_mount_extra_t *extra)
{
	ENDIAN_SWAP32(&extra->flags);
	ENDIAN_SWAP32(&extra->nbytes);
	ENDIAN_SWAP32(&extra->datalen);
	ENDIAN_SWAPMSGINFO(&extra->extra.cl.info);
}

int resmgr_endian(resmgr_context_t *ctp, resmgr_iomsgs_t *msg)
{
struct _xendian_context	*xendian;

	if (ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
		xendian = &ctp->extra->xendian;
		ENDIAN_SWAP16(&msg->combine.type);
		ENDIAN_SWAP16(&msg->combine.combine_len);
		switch (msg->type) {
		case _IO_CONNECT:
		case _PATH_CHDIR:
		case _PATH_CHROOT:
			// "msg->connect.subtype" is "msg->combine.combine_len" (above)
			ENDIAN_SWAP32(&msg->connect.file_type);
			ENDIAN_SWAP16(&msg->connect.reply_max);
			ENDIAN_SWAP16(&msg->connect.entry_max);
			ENDIAN_SWAP32(&msg->connect.key);
			ENDIAN_SWAP32(&msg->connect.handle);
			ENDIAN_SWAP32(&msg->connect.ioflag);
			ENDIAN_SWAP32(&msg->connect.mode);
			ENDIAN_SWAP16(&msg->connect.sflag);
			ENDIAN_SWAP16(&msg->connect.access);
			ENDIAN_SWAP16(&msg->connect.path_len);
			ENDIAN_SWAP16(&msg->connect.extra_len);
			switch (msg->connect.subtype) {
			case _IO_CONNECT_LINK:
				ENDIAN_SWAPLINKXTRA(&msg->connect, ALIGNMSG(msg, offsetof(struct _io_connect, path), msg->open.connect.path_len));
				break;
			case _IO_CONNECT_MOUNT:
				ENDIAN_SWAPMOUNTXTRA(&msg->connect, ALIGNMSG(msg, offsetof(struct _io_connect, path), msg->open.connect.path_len));
				break;
			default: break;
			}
			xendian->hint = msg->connect.subtype;
			break;
		case _IO_READ:
			ENDIAN_SWAP32(&msg->read.i.nbytes);
			ENDIAN_SWAP32(&msg->read.i.xtype);
			ENDIAN_SWAPXTYPE(msg->read.i.xtype & _IO_XTYPE_MASK, (void *)(&msg->read + 1));
			xendian->hint = msg->read.i.xtype;
			break;
		case _IO_WRITE:
			ENDIAN_SWAP32(&msg->write.i.nbytes);
			ENDIAN_SWAP32(&msg->write.i.xtype);
			ENDIAN_SWAPXTYPE(msg->write.i.xtype & _IO_XTYPE_MASK, (void *)(&msg->write + 1));
			xendian->hint = msg->write.i.xtype;
			break;
		case _IO_STAT:
			break;
		case _IO_NOTIFY:
			ENDIAN_SWAP32(&msg->notify.i.action);
			ENDIAN_SWAP32(&msg->notify.i.flags);
			ENDIAN_SWAPSIGEVENT(&msg->notify.i.event);
			break;
		case _IO_DEVCTL:
			ENDIAN_SWAP32(&msg->devctl.i.dcmd);
			ENDIAN_SWAP32(&msg->devctl.i.nbytes);
			switch (msg->devctl.i.dcmd) {
			case DCMD_ALL_SETFLAGS:
				ENDIAN_SWAP32((uint32_t *)(&msg->devctl + 1));
				break;
			default: break;
			}
			xendian->hint = msg->devctl.i.dcmd;
			break;
		case _IO_PATHCONF:
			ENDIAN_SWAP16(&msg->pathconf.i.name);
			break;
		case _IO_LSEEK:
			ENDIAN_SWAP16(&msg->lseek.i.whence);
			ENDIAN_SWAP64(&msg->lseek.i.offset);
			break;
		case _IO_CHMOD:
			ENDIAN_SWAP32(&msg->chmod.i.mode);
			break;
		case _IO_CHOWN:
			ENDIAN_SWAP32(&msg->chown.i.uid);
			ENDIAN_SWAP32(&msg->chown.i.gid);
			break;
		case _IO_UTIME:
			ENDIAN_SWAP32(&msg->utime.i.cur_flag);
			ENDIAN_SWAP32(&msg->utime.i.times.actime);
			ENDIAN_SWAP32(&msg->utime.i.times.modtime);
			break;
		case _IO_OPENFD:
			ENDIAN_SWAP32(&msg->openfd.i.ioflag);
			ENDIAN_SWAP16(&msg->openfd.i.sflag);
			ENDIAN_SWAP16(&msg->openfd.i.xtype);
			ENDIAN_SWAPMSGINFO(&msg->openfd.i.info);
			ENDIAN_SWAP32(&msg->openfd.i.key);
			break;
		case _IO_FDINFO:
			ENDIAN_SWAP32(&msg->fdinfo.i.flags);
			ENDIAN_SWAP32(&msg->fdinfo.i.path_len);
			break;
		case _IO_LOCK:
			ENDIAN_SWAP32(&msg->lock.i.subtype);
			ENDIAN_SWAP32(&msg->lock.i.nbytes);
			ENDIAN_SWAPFLOCK((struct flock *)(&msg->lock + 1));
			break;
		case _IO_SPACE:
			ENDIAN_SWAP16(&msg->space.i.subtype);
			ENDIAN_SWAP16(&msg->space.i.whence);
			ENDIAN_SWAP64(&msg->space.i.start);
			ENDIAN_SWAP64(&msg->space.i.len);
			break;
		case _IO_MMAP:
			ENDIAN_SWAP32(&msg->mmap.i.prot);
			ENDIAN_SWAP64(&msg->mmap.i.offset);
			ENDIAN_SWAPMSGINFO(&msg->mmap.i.info);
			break;
		case _IO_MSG:
			ENDIAN_SWAP16(&msg->msg.i.mgrid);
			ENDIAN_SWAP16(&msg->msg.i.subtype);
			break;
		case _IO_DUP:
			ENDIAN_SWAPMSGINFO(&msg->dup.i.info);
			ENDIAN_SWAP32(&msg->dup.i.key);
			break;
		case _IO_CLOSE:
			break;
		case _IO_SYNC:
			ENDIAN_SWAP32(&msg->sync.i.flag);
			break;
		default:
			return(EENDIAN);
		}
		xendian->type = msg->type;
	}
	return(EOK);
}

int resmgr_msgreplyv(resmgr_context_t *ctp, struct iovec *iov, int parts)
{
resmgr_iomsgs_t			*msg;
struct _xendian_context	*xendian;

	if (ctp->info.flags & _NTO_MI_ENDIAN_DIFF && parts != 0) {
		msg = (resmgr_iomsgs_t *)GETIOVBASE(&iov[0]);
		xendian = &ctp->extra->xendian;
		switch (xendian->type) {
		case _IO_CONNECT:
			if (ctp->status & _IO_CONNECT_RET_FLAG) {
				switch (ctp->status & _IO_CONNECT_RET_TYPE_MASK) {
				case _IO_CONNECT_RET_FTYPE:
					ENDIAN_SWAP16(&msg->open.ftype_reply.status);
					ENDIAN_SWAP32(&msg->open.ftype_reply.file_type);
					break;
				case _IO_CONNECT_RET_LINK:
					ENDIAN_SWAPLINK(&msg->readlink.link_reply, (GETIOVLEN(&iov[0]) > sizeof(struct _io_connect_link_reply)) ? &msg->readlink.link_reply + 1 : GETIOVBASE(&iov[1]));
					break;
				default: break;
				}
			}
			else {
				switch (xendian->hint) {
				case _IO_CONNECT_READLINK:
					ENDIAN_SWAPLINK(&msg->readlink.link_reply, (GETIOVLEN(&iov[0]) > sizeof(struct _io_connect_link_reply)) ? &msg->readlink.link_reply + 1 : GETIOVBASE(&iov[1]));
					break;
				default: break;
				}
			}
			break;
		case _IO_READ:
			if (S_ISDIR(xendian->mode)) {
				ENDIAN_SWAPREADDIR((struct dirent *)msg, ctp->status);
			}
			else if (S_ISNAM(xendian->mode) && (xendian->hint & _IO_XTYPE_MASK) == _IO_XTYPE_MQUEUE) {
				ENDIAN_SWAP32((uint32_t *)msg);
			}
			break;
		case _IO_WRITE:
			break;
		case _IO_STAT:
			ENDIAN_SWAPSTAT(&msg->stat.o);
			break;
		case _IO_NOTIFY:
			ENDIAN_SWAP32(&msg->notify.o.flags);
			ENDIAN_SWAP32(&msg->notify.o.flags2);
			ENDIAN_SWAPSIGEVENT(&msg->notify.o.event);
			break;
		case _IO_DEVCTL:
			ENDIAN_SWAP32(&msg->devctl.o.ret_val);
			ENDIAN_SWAP32(&msg->devctl.o.nbytes);
			switch (xendian->hint) {
			case DCMD_ALL_GETFLAGS:
			case DCMD_ALL_GETMOUNTFLAGS:
				ENDIAN_SWAP32((uint32_t *)(&msg->devctl + 1));
				break;
			case DCMD_FSYS_STATVFS:
				ENDIAN_SWAPSTATVFS((struct statvfs *)(&msg->devctl + 1));
				break;
			default: break;
			}
			break;
		case _IO_PATHCONF:
			break;
		case _IO_LSEEK:
			ENDIAN_SWAP64(&msg->lseek.o);
			break;
		case _IO_CHMOD:
			break;
		case _IO_CHOWN:
			break;
		case _IO_UTIME:
			break;
		case _IO_OPENFD:
			break;
		case _IO_FDINFO:
			ENDIAN_SWAP32(&msg->fdinfo.o.info.mode);
			ENDIAN_SWAP32(&msg->fdinfo.o.info.ioflag);
			ENDIAN_SWAP64(&msg->fdinfo.o.info.offset);
			ENDIAN_SWAP64(&msg->fdinfo.o.info.size);
			ENDIAN_SWAP32(&msg->fdinfo.o.info.flags);
			ENDIAN_SWAP16(&msg->fdinfo.o.info.sflag);
			ENDIAN_SWAP16(&msg->fdinfo.o.info.count);
			ENDIAN_SWAP16(&msg->fdinfo.o.info.rcount);
			ENDIAN_SWAP16(&msg->fdinfo.o.info.wcount);
			ENDIAN_SWAP16(&msg->fdinfo.o.info.rlocks);
			ENDIAN_SWAP16(&msg->fdinfo.o.info.wlocks);
			break;
		case _IO_LOCK:
			ENDIAN_SWAPFLOCK((struct flock *)(&msg->lock + 1));
			break;
		case _IO_SPACE:
			ENDIAN_SWAP64(&msg->space.o);
			break;
		case _IO_MMAP:
			ENDIAN_SWAP32(&msg->mmap.o.flags);
			ENDIAN_SWAP64(&msg->mmap.o.offset);
			ENDIAN_SWAP32(&msg->mmap.o.coid);
			ENDIAN_SWAP32(&msg->mmap.o.fd);
			break;
		case _IO_MSG:
			break;
		case _IO_DUP:
			break;
		case _IO_CLOSE:
			break;
		case _IO_SYNC:
			break;
		default:
			(void) MsgError_r(ctp->rcvid, errno = EENDIAN);
			return(-1);
		}
	}
	return(MsgReplyv(ctp->rcvid, ctp->status, iov, parts));
}

int resmgr_msgreply(resmgr_context_t *ctp, void *ptr, int len)
{
struct iovec	iov;

	SETIOV(&iov, ptr, len);
	return(resmgr_msgreplyv(ctp, &iov, 1));
}

void resmgr_endian_context(resmgr_context_t *ctp, int type, int mode, int hint)
{
struct _xendian_context	*xendian;

	xendian = &ctp->extra->xendian;
	if (type != 0)
		xendian->type = type;
	if ((mode & S_IFMT) != 0)
		xendian->mode = mode;
	if (hint != _IO_XTYPE_MASK)
		xendian->hint = hint;
}

__SRCVERSION("resmgr_endian.c $Rev: 153052 $");
