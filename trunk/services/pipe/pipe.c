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



#ifdef __USAGE
%C - POSIX pipe daemon

%C [-1] [-a <size>] [-P] [-s <size>] [-d] &

Options:
  -1    Unblock select for writing when _PC_PIPE_BUF available (default 1 byte)
  -a    Set atomic write size (_PC_PIPE_BUF, default 5120 bytes)
  -P    Synchronise modification of POSIX attributes to host filesystem
  -s    Set total pipe buffer size (default 5120 bytes)
  -d    Do not daemonize
#endif

struct _iofunc_ocb;
#define RESMGR_OCB_T struct _iofunc_ocb
#define IOFUNC_OCB_T struct _iofunc_ocb
struct pipe;
#define RESMGR_HANDLE_T struct pipe

#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <gulliver.h>
#include <pthread.h>
#include <share.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dcmd_blk.h>
#include <sys/dcmd_chr.h>
#include <sys/dispatch.h>
#include <sys/ftype.h>
#include <sys/iofunc.h>
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include <sys/procmgr.h>
#include <sys/resmgr.h>
#include <sys/siginfo.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#define PIPE_NAME		"/dev/pipe"
#define PIPE_ATOMIC		max(PIPE_BUF, _POSIX_PIPE_BUF)
#define PIPE_SIZE		PIPE_ATOMIC
#define PIPE_STACKSIZE	(12 * 1024)

#define OCB2PIPE(_ocb)	(pipe_t *)((_ocb)->attr)
#define LOCK(_hdr)		iofunc_attr_lock(&((_hdr)->attr))
#define UNLOCK(_hdr)	iofunc_attr_unlock(&((_hdr)->attr))
#define TRYLOCK(_hdr)	iofunc_attr_trylock(&((_hdr)->attr))
#define ATTR_DESTROY	0x00010000

typedef struct blocked {
	int					rcvid;
	int					scoid;
	int					coid;
	int					offset;
	size_t				size;
	size_t				nbytes;
	struct blocked		*link;
} blocked_t;

typedef struct pipe {
	iofunc_attr_t		attr;
	iofunc_notify_t		notify[IOFUNC_NOTIFY_OBAND - IOFUNC_NOTIFY_INPUT + 1];
	struct blocked		*waiting[IOFUNC_NOTIFY_OBAND - IOFUNC_NOTIFY_INPUT + 1];
	char				*name;
	int					fd;
	dev_t				devno;
	struct pipe			*link;
	int					rd;
	int					wr;
	char				*buffer;
} pipe_t;

void					*Dispatch;
thread_pool_t			*Pool;
iofunc_mount_t			Mount;
pipe_t					Pipes, Fifos;
resmgr_connect_funcs_t	PipeConnectFuncs, FifoConnectFuncs;
resmgr_io_funcs_t		PipeIoFuncs;
resmgr_attr_t			ResmgrAttr;
thread_pool_attr_t		PoolAttr;
pthread_attr_t			ThreadAttr;
struct sigevent			Event;
int						BufferSize, AtomicSize, StrictUpdate, SelWrite1;
int						NotifyCounts[IOFUNC_NOTIFY_OBAND - IOFUNC_NOTIFY_INPUT + 1];

/*
 *  Lookup a pipe/fifo (by unique dev/ino pair).
 */
pipe_t *lookup_pipe(pipe_t *head, dev_t dev, ino_t ino, int destroy)
{
pipe_t	*p;

	for (p = head->link; p != NULL; p = p->link) {
		if (p->devno == dev && p->attr.inode == ino) {
			iofunc_attr_lock(&p->attr);
			if ((p->attr.flags & ATTR_DESTROY) == destroy)
				break;
			iofunc_attr_unlock(&p->attr);
		}
	}
	return(p);
}

/*
 *  Synchronise the stat information of the active/closing FIFO
 *  back to its underlying host filesystem inode.
 */
void update_pipe(pipe_t *p, int force)
{
struct utimbuf	tm;

	if (force || (StrictUpdate && p->fd != -1)) {
		iofunc_time_update(&p->attr);
	}
	if (p->fd != -1 && (StrictUpdate || force)) {
		if (p->attr.flags & IOFUNC_ATTR_DIRTY_OWNER)
			fchown(p->fd, p->attr.uid, p->attr.gid);
		if (p->attr.flags & IOFUNC_ATTR_DIRTY_MODE)
			fchmod(p->fd, p->attr.mode);
		if (p->attr.flags & IOFUNC_ATTR_DIRTY_TIME)
			futime(p->fd, (tm.actime = p->attr.atime, tm.modtime = p->attr.mtime, &tm));
		p->attr.flags &= ~(IOFUNC_ATTR_DIRTY_OWNER | IOFUNC_ATTR_DIRTY_MODE | IOFUNC_ATTR_DIRTY_TIME);
	}
}

/*
 *  Create a new pipe/fifo, adding it into the linked list.
 */
pipe_t *create_pipe(pipe_t *head, char *name, mode_t mode, struct _client_info *owner)
{
pipe_t	*p;

	if ((p = malloc(sizeof(pipe_t))) != NULL) {
		if ((p->name = name) == NULL || (p->name = strdup(name)) != NULL) {
			if ((p->buffer = malloc(BufferSize)) != NULL) {
				iofunc_attr_init(&p->attr, S_IFIFO | (mode & S_IPERMS), &head->attr, owner);
				p->devno = head->attr.mount->dev, p->attr.inode = (uintptr_t)p;
				IOFUNC_NOTIFY_INIT(p->notify);
				p->fd = -1;
				p->waiting[IOFUNC_NOTIFY_INPUT] = p->waiting[IOFUNC_NOTIFY_OUTPUT] = p->waiting[IOFUNC_NOTIFY_OBAND] = NULL;
				p->rd = p->wr = 0;
				iofunc_attr_lock(&p->attr);
				LOCK(head);
				p->link = head->link, head->link = p;
				++head->attr.nbytes;
				UNLOCK(head);
				return(p);
			}
			free(p->name);
		}
		free(p);
	}
	return(NULL);
}

/*
 *  Release a pipe/fifo, removing it from the linked list (HEAD is locked).
 */
int destroy_pipe(pipe_t *head, pipe_t *p, int error)
{
pipe_t	**pp;

	LOCK(head);
	for (pp = &head->link; *pp != p; pp = &(*pp)->link) {
		if (*pp == NULL) {
			UNLOCK(head);
			return(error);
		}
	}
	*pp = p->link;
	--head->attr.nbytes;
	UNLOCK(head);
	if (error == EOK)
		update_pipe(p, !0);
	if (p->fd != -1)
		close(p->fd);
	iofunc_notify_remove(NULL, p->notify);
	iofunc_attr_unlock(&p->attr);
	free(p->buffer);
	free(p->name);
	free(p);
	return(error);
}

/*
 *  Coordinate acquiring an external pipe/fifo with any internal already-
 *  open items, and return the new pipe synched up to the external one.
 */
int acquire_pipe(pipe_t *head, char *name, pipe_t **pipe)
{
pipe_t		*p;
io_stat_t	st;
int			error, fd;

	st.i.type = _IO_STAT, st.i.combine_len = sizeof(st), st.i.zero = 0;
	if ((fd = _connect(0, name, 0, O_ACCMODE, SH_DENYNO, _IO_CONNECT_COMBINE, !0, 0, _FTYPE_ANY, _IO_CONNECT_EXTRA_NONE, sizeof(st.i), &st.i, sizeof(st.o), &st.o, NULL)) == -1) {
		error = errno;
	}
	else {
		LOCK(head);
		if ((p = lookup_pipe(head, st.o.st_dev, st.o.st_ino, 0)) != NULL) {
			close(fd);
			*pipe = p, error = EOK;
		}
		else if ((p = create_pipe(head, name, st.o.st_mode, NULL)) != NULL) {
			p->devno = st.o.st_dev, p->attr.inode = st.o.st_ino;
			p->attr.uid = st.o.st_uid, p->attr.gid = st.o.st_gid;
			p->attr.mtime = st.o.st_mtime, p->attr.atime = st.o.st_atime, p->attr.ctime = st.o.st_ctime;
			p->attr.nlink = st.o.st_nlink, p->attr.rdev = st.o.st_rdev;
			p->attr.flags &= ~(IOFUNC_ATTR_DIRTY_MASK | IOFUNC_ATTR_MTIME | IOFUNC_ATTR_ATIME | IOFUNC_ATTR_CTIME);
			p->fd = fd;
			*pipe = p, error = EOK;
		}
		else {
			error = ENOMEM;
		}
		UNLOCK(head);
	}
	return(error);
}

/*
 *  Coordinate releasing a pipe/fifo with any concurrent active lookups,
 *  avoiding deadlocks between the two locking mechanisms or multiple frees.
 */
void release_pipe(pipe_t *p, int error)
{
pipe_t	*head, *find;
dev_t	dev;
ino_t	ino;

	if (!S_ISFIFO(p->attr.mode) || p->attr.count != 0) {
		iofunc_attr_unlock(&p->attr);
	}
	else {
		head = (p->name != NULL) ? &Fifos : &Pipes;
		p->attr.flags |= ATTR_DESTROY;
		if (TRYLOCK(head) != EBUSY) {
			destroy_pipe(head, p, error);
		}
		else {
			dev = p->devno, ino = p->attr.inode;
			iofunc_attr_unlock(&p->attr);
			LOCK(head);
			if ((find = lookup_pipe(head, dev, ino, ATTR_DESTROY)) != NULL) {
				if (find != p || p->attr.count != 0)
					iofunc_attr_unlock(&find->attr);
				else
					destroy_pipe(head, p, error);
			}
		}
		UNLOCK(head);
	}
}

/*
 *  Transfer data (message-pass) from the pipe to the client.
 */
int read_from_pipe(pipe_t *pipe, int rcvid, size_t nbytes, int offset)
{
iov_t	iov[2];
size_t	avail, contig;
int		nparts;

	avail = min(pipe->attr.nbytes, nbytes);
	if ((contig = BufferSize - pipe->rd) >= avail) {
		nparts = 1;
		SETIOV(&iov[0], &pipe->buffer[pipe->rd], avail);
	}
	else {
		nparts = 2;
		SETIOV(&iov[0], &pipe->buffer[pipe->rd], contig);
		SETIOV(&iov[1], &pipe->buffer[0], avail - contig);
	}
	if (offset == 0 && avail == nbytes) {
		if (MsgReplyv(rcvid, nbytes, iov, nparts) == -1)
			return(-errno);
	}
	else {
		if (MsgWritev(rcvid, iov, nparts, offset) == -1)
			return(-errno);
	}
	pipe->rd = (pipe->rd + avail) % BufferSize;
	pipe->attr.nbytes -= avail;
	pipe->attr.flags |= IOFUNC_ATTR_ATIME;
	return(avail);
}

/*
 *  Transfer data (message-pass) from the client to the pipe.
 */
int write_to_pipe(pipe_t *pipe, int rcvid, size_t nbytes, int offset, resmgr_context_t *ctp)
{
iov_t	iov[2];
size_t	room, contig;
int		nparts;

	room = min(BufferSize - pipe->attr.nbytes, nbytes);
	if ((contig = BufferSize - pipe->wr) >= room) {
		nparts = 1;
		SETIOV(&iov[0], &pipe->buffer[pipe->wr], room);
	}
	else {
		nparts = 2;
		SETIOV(&iov[0], &pipe->buffer[pipe->wr], contig);
		SETIOV(&iov[1], &pipe->buffer[0], room - contig);
	}
	if (ctp != NULL && ctp->size >= offset + room && nparts == 1) {
		memcpy(GETIOVBASE(&iov[0]), (char *)ctp->msg + offset, GETIOVLEN(&iov[0]));
	}
	else {
		if (MsgReadv(rcvid, iov, nparts, offset) == -1)
			return(-errno);
	}
	pipe->wr = (pipe->wr + room) % BufferSize;
	pipe->attr.nbytes += room;
	pipe->attr.flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
	return(room);
}

/*
 *  Check if any blocked readers can be satisfied (to be blocked they
 *  must have read no data and a partial read will unblock them now).
 */
void check_readers(pipe_t *pipe, int trans)
{
blocked_t	*b, **bb;
size_t		avail;
int			chunk;

	bb = &pipe->waiting[IOFUNC_NOTIFY_INPUT];
	while ((avail = pipe->attr.nbytes) != 0 && (b = *bb) != NULL) {
		if ((chunk = read_from_pipe(pipe, b->rcvid, b->size, 0)) < 0)
			MsgError(b->rcvid, -chunk);
		else if (chunk != b->size)
			MsgReply(b->rcvid, chunk, NULL, 0);
		*bb = b->link;
		free(b);
	}
	if (avail != 0 && IOFUNC_NOTIFY_INPUT_CHECK(pipe->notify, avail, trans)) {
		iofunc_notify_trigger(pipe->notify, avail, IOFUNC_NOTIFY_INPUT);
	}
}

/*
 *  Check if any blocked writers can make progress (by writing whatever
 *  can now fit, honouring atomic constraints, and unblocking full writes).
 */
void check_writers(pipe_t *pipe)
{
blocked_t	*b, **bb;
size_t		room;
int			chunk;

	bb = &pipe->waiting[IOFUNC_NOTIFY_OUTPUT];
	while ((room = BufferSize - pipe->attr.nbytes) != 0 && (b = *bb) != NULL) {
		if (b->size > AtomicSize || b->nbytes <= room) {
			if ((chunk = write_to_pipe(pipe, b->rcvid, b->nbytes, b->offset, NULL)) < 0 && b->size != b->nbytes)
				chunk = b->nbytes = (b->size -= b->nbytes);
			if (chunk < 0 || !(b->nbytes -= chunk)) {
				if (chunk < 0)
					MsgError(b->rcvid, -chunk);
				else
					MsgReply(b->rcvid, b->size, NULL, 0);
				*bb = b->link;
				free(b);
			}
			else {
				b->offset += chunk;
				bb = &b->link;
			}
		}
		else {
			bb = &b->link;
		}
	}
	if (room != 0 && IOFUNC_NOTIFY_OUTPUT_CHECK(pipe->notify, room)) {
		iofunc_notify_trigger(pipe->notify, room, IOFUNC_NOTIFY_OUTPUT);
	}
}

/*
 *  Add a new blocking entry to the head/tail of the given list.
 */
blocked_t *block_client(blocked_t **blocked, resmgr_context_t *ctp, int offset, size_t size, size_t nbytes, int tail)
{
blocked_t	*b;

	if ((b = malloc(sizeof(blocked_t))) != NULL) {
		b->rcvid = ctp->rcvid;
		b->scoid = ctp->info.scoid, b->coid = ctp->info.coid;
		b->offset = offset, b->size = size, b->nbytes = nbytes;
		if (tail) {
			while (*blocked != NULL)
				blocked = &(*blocked)->link;
		}
		b->link = *blocked, *blocked = b;
	}
	return(b);
}

/*
 *  Unblock given clients (all, by rcvid, by scoid/coid) from the pipe
 *  and return the appropriate status indication to the client (handling
 *  special EINTR and EPIPE situations).
 */
void unblock_clients(blocked_t **blocked, resmgr_context_t *ctp, int match, int error)
{
blocked_t	*b;

	while ((b = *blocked) != NULL) {
		if ((ctp == NULL) || (match && b->rcvid == ctp->rcvid) || (!match && b->scoid == ctp->info.scoid && b->coid == ctp->info.coid)) {
			if (error == EOK) {
				MsgReply(b->rcvid, 0, NULL, 0);
			}
			else if (error == -EINTR) {
				if (b->size != b->nbytes)
					MsgReply(b->rcvid, b->size - b->nbytes, NULL, 0);
				else
					MsgError(b->rcvid, EINTR);
			}
			else {
				if (error == EPIPE)
					MsgDeliverEvent(b->rcvid, &Event);
				MsgError(b->rcvid, error);
			}
			*blocked = b->link;
			free(b);
		}
		else {
			blocked = &b->link;
		}
	}
}

/*
 *  Handle the pipe/fifo open() requirements of waiting for both a
 *  read and write unless O_NONBLOCK specified (or not rd/wr access).
 */
int open_semantics(pipe_t *pipe, resmgr_context_t *ctp, int ioflag)
{
int		error;

	if (!(ioflag & (_IO_FLAG_RD | _IO_FLAG_WR)))
		error = EOK;
	else if (pipe->attr.rcount != 0 && pipe->attr.wcount != 0)
		unblock_clients(&pipe->waiting[IOFUNC_NOTIFY_OBAND], NULL, -1, error = EOK);
	else if (ioflag & O_NONBLOCK)
		error = EOK;
	else if (block_client(&pipe->waiting[IOFUNC_NOTIFY_OBAND], ctp, 0, 0, 0, 0) == NULL)
		error = ENOMEM;
	else
		error = _RESMGR_NOREPLY;
	return(error);
}

/*
 *  The open routine for a (unnamed) pipe; this coordinates with the
 *  mechanism used in the libc by "pipe()" (so also see openfd callout).
 *  This also services access to the "/dev/pipe" pathname, which can only
 *  be opened O_ACCMODE (what would read/write on it mean?).
 */
int resmgr_pipe_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
pipe_t				*pipe;
struct _client_info	info;
int					error, ioflag;

	ioflag = msg->connect.ioflag & (_IO_FLAG_RD | _IO_FLAG_WR);
	if (msg->connect.file_type == _FTYPE_ANY) {
		error = (ioflag != 0) ? EACCES : iofunc_open_default(ctp, msg, &handle->attr, extra);
	}
	else if (msg->connect.file_type == _FTYPE_PIPE) {
		if (iofunc_client_info(ctp, msg->connect.ioflag, &info) != EOK)
			error = ESRCH;
		else if (ioflag != _IO_FLAG_RD)
			error = EINVAL;
		else if ((pipe = create_pipe(&Pipes, NULL, Pipes.attr.mode, &info)) == NULL)
			error = ENOMEM;
		else if ((error = iofunc_open(ctp, msg, &pipe->attr, NULL, &info)) != EOK)
			release_pipe(pipe, error);
		else if ((error = iofunc_ocb_attach(ctp, msg, NULL, &pipe->attr, NULL)) != EOK)
			release_pipe(pipe, error);
		else
			iofunc_attr_unlock(&pipe->attr);
	}
	else {
		error = ENOSYS;
	}
	return(error);
}

/*
 *  Unlink for pipes; since pipes are unnamed this access must be from
 *  an attempt to remove "/dev/pipe" itself, which is non-sensical.
 */
int resmgr_pipe_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
	return(EACCES);
}

/*
 *  Read from a pipe or FIFO.
 */
int resmgr_pipe_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
pipe_t		*pipe;
blocked_t	*b;
size_t		nbytes, count;
int			error, nonblock, chunk;

	pipe = OCB2PIPE(ocb);
	if ((error = iofunc_read_verify(ctp, msg, ocb, &nonblock)) != EOK)
		return(error);
	if ((msg->i.xtype & _IO_XTYPE_MASK) == _IO_XTYPE_OFFSET)
		return(ESPIPE);
	else if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
		return(ENOSYS);
	if (!(nbytes = msg->i.nbytes))
		return(EOK);
	count = 0, chunk = 0;
	while (pipe->attr.nbytes != 0 && count < nbytes) {
		if ((chunk = read_from_pipe(pipe, ctp->rcvid, nbytes - count, count)) > 0)
			count += chunk;
		else if (!count)
			return(-chunk);
		else
			break;
		check_writers(pipe);
	}
	if (!count) {
		_IO_SET_READ_NBYTES(ctp, 0);
		if (!pipe->attr.wcount)
			error = EOK;
		else if (nonblock)
			error = EAGAIN;
		else if ((b = block_client(&pipe->waiting[IOFUNC_NOTIFY_INPUT], ctp, 0, nbytes, nbytes, !0)) == NULL)
			error = ENOMEM;
		else
			error = _RESMGR_NOREPLY;
	}
	else if (chunk == nbytes) {
		error = _RESMGR_NOREPLY;
	}
	else {
		_IO_SET_READ_NBYTES(ctp, count);
		error = EOK;
	}
	return(error);
}

/*
 *  Write to a pipe or FIFO.
 */
int resmgr_pipe_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
pipe_t		*pipe;
blocked_t	*b;
size_t		nbytes, count;
int			error, nonblock, chunk, offset, trans;

	pipe = OCB2PIPE(ocb);
	if ((error = iofunc_write_verify(ctp, msg, ocb, &nonblock)) != EOK)
		return(error);
	if ((msg->i.xtype & _IO_XTYPE_MASK) == _IO_XTYPE_OFFSET)
		return(ESPIPE);
	else if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
		return(ENOSYS);
	if (!pipe->attr.rcount)
		return(MsgDeliverEvent(ctp->rcvid, &Event), EPIPE);
	if (!(nbytes = msg->i.nbytes))
		return(EOK);
	count = chunk = 0, offset = ctp->offset + sizeof(io_write_t), trans = !pipe->attr.nbytes;
	if (nbytes > AtomicSize) {
		while (nbytes > count && pipe->attr.nbytes != BufferSize) {
			if ((chunk = write_to_pipe(pipe, ctp->rcvid, nbytes - count, offset + count, ctp)) > 0)
				count += chunk;
			else if (!count)
				return(-chunk);
			else
				break;
			check_readers(pipe, trans), trans = 0;
		}
	}
	else if (BufferSize - pipe->attr.nbytes >= nbytes) {
		if ((chunk = write_to_pipe(pipe, ctp->rcvid, nbytes, offset, ctp)) > 0)
			count += chunk;
		else
			return(-chunk);
		check_readers(pipe, trans);
	}
	if (!count && nonblock) {
		error = EAGAIN;
	}
	else if (count != nbytes && chunk >= 0 && !nonblock) {
		if ((b = block_client(&pipe->waiting[IOFUNC_NOTIFY_OUTPUT], ctp, offset + count, nbytes, nbytes - count, !0)) == NULL)
			error = ENOMEM;
		else
			error = _RESMGR_NOREPLY;
	}
	else {
		_IO_SET_WRITE_NBYTES(ctp, count);
		error = EOK;
	}
	return(error);
}

/*
 *  Last close of a pipe or FIFO; honour appropriate semantics.
 */
int resmgr_pipe_closeocb(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb)
{
pipe_t		*pipe;
unsigned	flags;

	pipe = OCB2PIPE(ocb);
	iofunc_attr_lock(&pipe->attr);
	flags = iofunc_ocb_detach(ctp, ocb);
	iofunc_ocb_free(ocb);
	if (S_ISFIFO(pipe->attr.mode) && flags != 0) {
		if (flags & IOFUNC_OCB_LAST_READER) {
			unblock_clients(&pipe->waiting[IOFUNC_NOTIFY_OUTPUT], NULL, -1, EPIPE);
			iofunc_notify_trigger(pipe->notify, INT_MAX, IOFUNC_NOTIFY_OUTPUT);
		}
		if (flags & IOFUNC_OCB_LAST_WRITER) {
			unblock_clients(&pipe->waiting[IOFUNC_NOTIFY_INPUT], NULL, -1, EOK);
			iofunc_notify_trigger(pipe->notify, INT_MAX, IOFUNC_NOTIFY_INPUT);
		}
		if (flags & IOFUNC_OCB_LAST_INUSE) {
			pipe->attr.nbytes = 0;
			pipe->rd = pipe->wr = 0;
		}
	}
	update_pipe(pipe, !0);
	release_pipe(pipe, EOK);
	return(EOK);
}

/*
 *  An fstat() of an open pipe or FIFO.
 */
int resmgr_pipe_stat(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb)
{
pipe_t	*pipe;
int		error;

	pipe = OCB2PIPE(ocb);
	update_pipe(pipe, !0);
	if ((error = iofunc_stat(ctp, &pipe->attr, &msg->o)) == EOK) {
		if (S_ISFIFO(pipe->attr.mode)) {
			msg->o.st_dev = (msg->o.st_dev & ~ND_NODE_MASK) | (pipe->devno & ND_NODE_MASK);
			msg->o.st_nblocks = ((BufferSize - 1) / (msg->o.st_blksize = AtomicSize)) + 1;
		}
		error = _RESMGR_PTR(ctp, &msg->o, sizeof(msg->o));
	}
	return(error);
}

/*
 *  Select/Poll of a pipe or FIFO.
 */
int resmgr_pipe_notify(resmgr_context_t *ctp, io_notify_t *msg, RESMGR_OCB_T *ocb)
{
pipe_t	*pipe;
int		error, trigger;

	pipe = OCB2PIPE(ocb);
	if (!S_ISFIFO(pipe->attr.mode)) {
		error = EBADF;
	}
	else {
		trigger = 0;
		if (pipe->attr.nbytes >= NotifyCounts[IOFUNC_NOTIFY_INPUT] || !pipe->attr.wcount)
			trigger |= _NOTIFY_COND_INPUT;
		if (BufferSize - pipe->attr.nbytes >= NotifyCounts[IOFUNC_NOTIFY_OUTPUT] || !pipe->attr.rcount)
			trigger |= _NOTIFY_COND_OUTPUT;
		error = iofunc_notify(ctp, msg, pipe->notify, trigger, NotifyCounts, NULL);
	}
	return(error);
}

/*
 *  A devctl to a pipe or FIFO.  Only FIONREAD is handled (although a
 *  statvfs on a FIFO is re-issued to its underlying host filesystem).
 */
int resmgr_pipe_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb)
{
pipe_t			*pipe;
uint32_t		*fionread;
struct statvfs	*stvfs;
int				error;

	pipe = OCB2PIPE(ocb);
	if (S_ISFIFO(pipe->attr.mode) && msg->i.dcmd == DCMD_CHR_ISCHARS) {
		fionread = (uint32_t *)_DEVCTL_DATA(msg->o);
		msg->o.ret_val = EOK;
		*fionread = !(ocb->ioflag & _IO_FLAG_RD) ? 0 : (ctp->info.flags & _NTO_MI_ENDIAN_DIFF) ? ENDIAN_RET32(pipe->attr.nbytes) : pipe->attr.nbytes;
		error = _RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + sizeof(*fionread));
	}
	else if (S_ISFIFO(pipe->attr.mode) && pipe->name != NULL && msg->i.dcmd == DCMD_FSYS_STATVFS) {
		stvfs = (struct statvfs *)_DEVCTL_DATA(msg->o);
		msg->o.ret_val = EOK;
		error = (fstatvfs(pipe->fd, stvfs) != -1) ? _RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + sizeof(*stvfs)) : errno;
	}
	else {
		error = iofunc_devctl(ctp, msg, ocb, &pipe->attr);
	}
	return(error);
}

/*
 *  Unblocking logic for a pipe or FIFO (unblock based on rcvid).
 */
int resmgr_pipe_unblock(resmgr_context_t *ctp, io_pulse_t *msg, RESMGR_OCB_T *ocb)
{
pipe_t				*pipe;
struct _msg_info	info;

	pipe = OCB2PIPE(ocb);
	if (MsgInfo(ctp->rcvid, &info) != -1 && (info.flags & _NTO_MI_UNBLOCK_REQ)) {
		iofunc_attr_lock(&pipe->attr);
		unblock_clients(&pipe->waiting[IOFUNC_NOTIFY_INPUT], ctp, !0, EINTR);
		unblock_clients(&pipe->waiting[IOFUNC_NOTIFY_OUTPUT], ctp, !0, -EINTR);
		unblock_clients(&pipe->waiting[IOFUNC_NOTIFY_OBAND], ctp, !0, EINTR);
		iofunc_attr_unlock(&pipe->attr);
	}
	return(_RESMGR_NOREPLY);
}

/*
 *  Handle the PC_PIPE_BUF pathconf query.
 */
int resmgr_pipe_pathconf(resmgr_context_t *ctp, io_pathconf_t *msg, RESMGR_OCB_T *ocb)
{
pipe_t	*pipe;
int		error;

	pipe = OCB2PIPE(ocb);
	if (S_ISFIFO(pipe->attr.mode) && msg->i.name == _PC_PIPE_BUF) {
		_IO_SET_PATHCONF_VALUE(ctp, AtomicSize);
		error = EOK;
	}
	else {
		error = iofunc_pathconf(ctp, msg, ocb, &pipe->attr);
	}
	return(error);
}

/*
 *  Seeking on pipes or FIFOs is illegal.
 */
int resmgr_pipe_lseek(resmgr_context_t *ctp, io_lseek_t *msg, RESMGR_OCB_T *ocb)
{
	return(ESPIPE);
}

/*
 *  Coordination with libc "pipe()" for the second half of the pipe pair.
 */
static int xtrapermsopenfd(resmgr_context_t *ctp, io_openfd_t *msg, RESMGR_OCB_T *ocb, int perms)
{
int		save, error;

	save = ocb->ioflag, ocb->ioflag |= perms;
	error = iofunc_openfd(ctp, msg, ocb, ocb->attr);
	ocb->ioflag = save;
	return(error);
}
static io_open_t *fakeopen(io_open_t *msg)
{
	memset(msg, 0, sizeof(io_open_t));
	msg->connect.type = _IO_CONNECT, msg->connect.subtype = _IO_CONNECT_OPEN;
	msg->connect.file_type = _FTYPE_PIPE;
	msg->connect.ioflag = _IO_FLAG_RD, msg->connect.access = _IO_FLAG_RD | _IO_FLAG_WR;
	msg->connect.sflag = SH_DENYNO;
	msg->connect.path_len = 1;
	return(msg);
}
int resmgr_pipe_openfd(resmgr_context_t *ctp, io_openfd_t *msg, RESMGR_OCB_T *ocb)
{
pipe_t		*pipe;
io_open_t	omsg;
int			error, ioflag;

	pipe = OCB2PIPE(ocb);
	ioflag = msg->i.ioflag & (_IO_FLAG_RD | _IO_FLAG_WR);
	if (msg->i.xtype == _IO_OPENFD_NONE) {
		if (S_ISDIR(pipe->attr.mode) || pipe->name != NULL)
			error = ENOSYS;
		else if (ioflag != _IO_FLAG_RD)
			error = EOPNOTSUPP;
		else
			error = resmgr_pipe_open(ctp, fakeopen(&omsg), &Pipes, NULL);
	}
	else if (msg->i.xtype == _IO_OPENFD_PIPE) {
		if (pipe->attr.wcount != 0)
			error = EBUSY;
		else if (ioflag != _IO_FLAG_WR)
			error = EOPNOTSUPP;
		else
			error = xtrapermsopenfd(ctp, msg, ocb, ioflag);
	}
	else {
		error = ENOSYS;
	}
	return(error);
}

/*
 *  Provide open details and name for the pipe or FIFO.
 */
int resmgr_pipe_fdinfo(resmgr_context_t *ctp, io_fdinfo_t *msg, RESMGR_OCB_T *ocb)
{
pipe_t	*pipe;
char	*name;
int		error, ulen, xlen, len;

	pipe = OCB2PIPE(ocb);
	ulen = msg->i.path_len, xlen = ctp->msg_max_size - ctp->offset - sizeof(msg->o);
	name = (char *)(&msg->o + 1);
	if ((error = iofunc_fdinfo(ctp, ocb, &pipe->attr, &msg->o.info)) == EOK) {
		if (S_ISNAM(pipe->attr.mode))
			len = sprintf(name, "%.*s", xlen, Pipes.name) + 1;
		else if (pipe->name == NULL)
			len = sprintf(name, "(pipe)") + 1;
		else
			len = sprintf(name, "/%.*s", xlen - 1, pipe->name) + 1;
		_IO_SET_FDINFO_LEN(ctp, len);
		error = _RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + min(ulen, len));
	}
	return(error);
}

/*
 *  Any close of a pipe or FIFO (unblock based on scoid/coid).
 */
int resmgr_pipe_closedup(resmgr_context_t *ctp, io_close_t *msg, RESMGR_OCB_T *ocb)
{
pipe_t		*pipe;

	pipe = OCB2PIPE(ocb);
	iofunc_attr_lock(&pipe->attr);
	unblock_clients(&pipe->waiting[IOFUNC_NOTIFY_INPUT], ctp, 0, EBADF);
	unblock_clients(&pipe->waiting[IOFUNC_NOTIFY_OUTPUT], ctp, 0, EBADF);
	unblock_clients(&pipe->waiting[IOFUNC_NOTIFY_OBAND], ctp, 0, EBADF);
	iofunc_notify_remove(ctp, pipe->notify);
	iofunc_attr_unlock(&pipe->attr);
	return(iofunc_close_dup_default(ctp, msg, ocb));
}

/*
 *  Override of default OCB locking as convenient point at which to
 *  provide the 'StrictUpdate' semantics for FIFOs.
 */
int resmgr_pipe_lockocb(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb)
{
	return(iofunc_attr_lock(ocb->attr));
}
int resmgr_pipe_unlockocb(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb)
{
	update_pipe(OCB2PIPE(ocb), 0);
	return(iofunc_attr_unlock(ocb->attr));
}

/*
 *  Open of a FIFO (redirected from a host filesystem).
 */
int resmgr_fifo_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
pipe_t	*pipe;
int		error;

	if (msg->connect.file_type != _FTYPE_PIPE) {
		error = ENOSYS;
	}
	else if ((error = acquire_pipe(&Fifos, msg->connect.path, &pipe)) == EOK) {
		if ((error = iofunc_open(ctp, msg, &pipe->attr, NULL, NULL)) != EOK)
			release_pipe(pipe, error);
		else if ((msg->connect.ioflag & (_IO_FLAG_RD | _IO_FLAG_WR)) == _IO_FLAG_WR && !pipe->attr.rcount && msg->connect.ioflag & O_NONBLOCK)
			release_pipe(pipe, error = ENXIO);
		else if ((error = iofunc_ocb_attach(ctp, msg, NULL, &pipe->attr, NULL)) != EOK)
			release_pipe(pipe, error);
		else if ((error = open_semantics(pipe, ctp, msg->connect.ioflag)) != EOK && error != _RESMGR_NOREPLY)
			release_pipe(pipe, (iofunc_ocb_detach(ctp, _resmgr_ocb(ctp, &ctp->info)), error));
		else
			iofunc_attr_unlock(&pipe->attr);
	}
	return(error);
}

/*
 *  Shutting down, so unblock any clients (open, read, write, select)
 *  before termination.
 */
void cleanup(pipe_t *head)
{
	while ((head = head->link) != NULL) {
		unblock_clients(&head->waiting[IOFUNC_NOTIFY_INPUT], NULL, -1, EBADF);
		unblock_clients(&head->waiting[IOFUNC_NOTIFY_OUTPUT], NULL, -1, EBADF);
		unblock_clients(&head->waiting[IOFUNC_NOTIFY_OBAND], NULL, -1, EBADF);
		iofunc_notify_trigger(head->notify, INT_MAX, IOFUNC_NOTIFY_INPUT);
		iofunc_notify_trigger(head->notify, INT_MAX, IOFUNC_NOTIFY_OUTPUT);
		iofunc_notify_trigger(head->notify, INT_MAX, IOFUNC_NOTIFY_OBAND);
	}
}

/*
 *  Parse a string (recognising common suffixes) into a numeric size;
 *  values <= 0 are invalid (as this configures various buffer sizes).
 */
int parsesize(const char *str)
{
char	*cp;
long	number;
char	suffix;

	number = strtol(str, &cp, 0);
	if (number <= 0)
		return(0);
	suffix = toupper(*cp);
	if (suffix == 'B')
		number *= 1, ++cp;
	else if (suffix == 'K')
		number <<= 10, ++cp;
	else if (suffix == 'M')
		number <<= 20, ++cp;
	return((*cp == '\0') ? number : 0);
}

/*
 *  Display message regarding a fatal initialisation error, and exit.
 */
void fatal(const char *errmsg, ...)
{
extern char	*__progname;
va_list		args;

	fprintf(stderr, "%s: ", __progname);
	va_start(args, errmsg);
	vfprintf(stderr, errmsg, args);
	va_end(args);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
sigset_t	signals;
int			opt;
int			daemonize = 1;

	BufferSize = PIPE_SIZE, AtomicSize = PIPE_ATOMIC;
	StrictUpdate = 0, SelWrite1 = !0;
	while ((opt = getopt(argc, argv, ":1a:Ps:d")) != -1 || optind < argc) {
		switch (opt) {
		case '1':
			SelWrite1 = 0;
			break;
		case 'a':
			if (!(AtomicSize = parsesize(optarg)))
				fatal("invalid atomic write size");
			break;
		case 'P':
			StrictUpdate = !0;
			break;
		case 's':
			if (!(BufferSize = parsesize(optarg)))
				fatal("invalid pipe buffer size");
			break;
		case ':':
			fatal("missing argument for '-%c'", optopt);
			break;
		case 'd':
			daemonize = 0;
			break;
		case '?':
			fatal("unknown option '-%c'", optopt);
			break;
		case -1:
			fatal("unknown operand '%s'", argv[optind]);
			break;
		}
	}
	BufferSize = max(BufferSize, AtomicSize);

	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &PipeConnectFuncs, _RESMGR_IO_NFUNCS, &PipeIoFuncs);
	PipeConnectFuncs.open = resmgr_pipe_open;
	PipeConnectFuncs.unlink = resmgr_pipe_unlink;
	PipeIoFuncs.read = resmgr_pipe_read;
	PipeIoFuncs.write = resmgr_pipe_write;
	PipeIoFuncs.close_ocb = resmgr_pipe_closeocb;
	PipeIoFuncs.stat = resmgr_pipe_stat;
	PipeIoFuncs.notify = resmgr_pipe_notify;
	PipeIoFuncs.devctl = resmgr_pipe_devctl;
	PipeIoFuncs.unblock = resmgr_pipe_unblock;
	PipeIoFuncs.pathconf = resmgr_pipe_pathconf;
	PipeIoFuncs.lseek = resmgr_pipe_lseek;
	PipeIoFuncs.openfd = resmgr_pipe_openfd;
	PipeIoFuncs.fdinfo = resmgr_pipe_fdinfo;
	PipeIoFuncs.close_dup = resmgr_pipe_closedup;
	if (StrictUpdate) {
		PipeIoFuncs.lock_ocb = resmgr_pipe_lockocb;
		PipeIoFuncs.unlock_ocb = resmgr_pipe_unlockocb;
	}
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &FifoConnectFuncs, 0, NULL);
	FifoConnectFuncs.open = resmgr_fifo_open;

	if ((Dispatch = dispatch_create()) == NULL)
		fatal("unable to allocate resmgr context - %s", strerror(errno));
	memset(&ResmgrAttr, 0, sizeof(ResmgrAttr));
	ResmgrAttr.flags = RESMGR_FLAG_CROSS_ENDIAN;
	ResmgrAttr.nparts_max = 1, ResmgrAttr.msg_max_size = sizeof(struct _io_connect) + _POSIX_PATH_MAX;
	SIGEV_SIGNAL_THREAD_INIT(&Event, SIGPIPE, 0, SI_USER);
	NotifyCounts[IOFUNC_NOTIFY_INPUT] = NotifyCounts[IOFUNC_NOTIFY_OBAND] = 1;
	NotifyCounts[IOFUNC_NOTIFY_OUTPUT] = SelWrite1 ? 1 : AtomicSize;

	memset(&Mount, 0, sizeof(Mount));
	Mount.conf = IOFUNC_PC_CHOWN_RESTRICTED | IOFUNC_PC_NO_TRUNC | IOFUNC_PC_SYNC_IO;
	Mount.blocksize = AtomicSize;
    memset(&Pipes, 0, sizeof(Pipes));
    iofunc_attr_init(&Pipes.attr, S_IFNAM | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, NULL, NULL);
    Pipes.attr.mount = &Mount;
    Pipes.attr.flags |= IOFUNC_ATTR_SYNTHETIC;
	if ((Pipes.attr.rdev = resmgr_attach(Dispatch, &ResmgrAttr, Pipes.name = PIPE_NAME, _FTYPE_PIPE, 0, &PipeConnectFuncs, &PipeIoFuncs, &Pipes)) == -1)
		fatal("unable to register as pipe handler - %s", strerror(errno));
	resmgr_devino(Pipes.attr.rdev, &Mount.dev, &Pipes.attr.inode);
    memset(&Fifos, 0, sizeof(Fifos));
    iofunc_attr_init(&Fifos.attr, S_IFDIR | S_IPERMS, NULL, NULL);
    Fifos.attr.mount = &Mount;
	if ((Fifos.attr.rdev = resmgr_attach(Dispatch, &ResmgrAttr, Fifos.name = NULL, _FTYPE_PIPE, _RESMGR_FLAG_DIR | _RESMGR_FLAG_AFTER | _RESMGR_FLAG_FTYPEONLY, &FifoConnectFuncs, &PipeIoFuncs, &Fifos)) == -1)
		fatal("unable to register as fifo handler - %s", strerror(errno));

	memset(&PoolAttr, 0, sizeof(PoolAttr));
	pthread_attr_init(PoolAttr.attr = &ThreadAttr);
	pthread_attr_setdetachstate(&ThreadAttr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&ThreadAttr, PIPE_STACKSIZE);
	pthread_attr_setstacklazy(&ThreadAttr, PTHREAD_STACK_NOTLAZY);
	PoolAttr.lo_water = 2, PoolAttr.hi_water = 4, PoolAttr.maximum = 12;
	PoolAttr.increment = 1;
	PoolAttr.context_alloc = resmgr_context_alloc, PoolAttr.context_free = resmgr_context_free;
	PoolAttr.block_func = resmgr_block, PoolAttr.unblock_func = resmgr_unblock;
	PoolAttr.handler_func = resmgr_handler;
	PoolAttr.handle = Dispatch;
	if ((Pool = thread_pool_create(&PoolAttr, 0)) == NULL)
		fatal("unable to allocate thread pool - %s", strerror(errno));

	if(daemonize) {
		procmgr_daemon(EXIT_SUCCESS, PROCMGR_DAEMON_NOCLOSE | PROCMGR_DAEMON_NODEVNULL);
	}
	sigfillset(&signals), sigprocmask(SIG_SETMASK, &signals, NULL);
	if (thread_pool_start(Pool) == -1)
		fatal("unable to start thread pool - %s", strerror(errno));

	sigemptyset(&signals), sigaddset(&signals, SIGTERM);
	while (SignalWaitinfo(&signals, NULL) == -1)
		;
	thread_pool_destroy(Pool);
	cleanup(&Pipes), cleanup(&Fifos);
	return(EXIT_SUCCESS);
}

__SRCVERSION("pipe.c $Rev: 169545 $");
