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





struct _iofunc_ocb;
#define RESMGR_OCB_T struct _iofunc_ocb
#define IOFUNC_OCB_T struct _iofunc_ocb
struct semaphore;
#define RESMGR_HANDLE_T struct semaphore

#include "externs.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <mqueue.h>
#include <semaphore.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/conf.h>
#include <sys/dcmd_misc.h>
#include <sys/dispatch.h>
#include <sys/iofunc.h>
#include <sys/iomsg.h>
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <sys/resmgr.h>
#include <sys/resource.h>
#include <sys/sysmgr.h>
#include <unistd.h>

/* POSIX allows either behaviour, historical QNX clips to 0, I like -ve */
#undef  SEM_NEG_COUNT

#define OCB2SEM(_ocb)	((semaphore_t *)((_ocb)->attr))

typedef struct blocked {
    int                 rcvid;
    int                 scoid;
    int                 coid;
	unsigned			priority;
    struct blocked      *link;
} blocked_t;

typedef struct semaphore {
	iofunc_attr_t		attr;
	unsigned			initial;
	char				*name;
	blocked_t			*blocked;
	struct semaphore	*link;
} semaphore_t;

typedef struct {
	struct _client_info	*cred;
	iofunc_attr_t		*attr;
	uint32_t			*initial;
	uint32_t			*ioflag;
	int					verification;
} creation_t;

static int	resmgr_sem_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra);
static int	resmgr_sem_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *extra);
static int	resmgr_sem_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
static int	resmgr_sem_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
static int	resmgr_sem_close(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb);
static int	resmgr_sem_stat(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb);
static int	resmgr_sem_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);
static int	resmgr_sem_unblock(resmgr_context_t *ctp, io_pulse_t *msg, RESMGR_OCB_T *ocb);
static int	resmgr_dir_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra);
static int	resmgr_dir_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *extra);
static int	resmgr_dir_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
static int	resmgr_dir_close(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb);
static int	resmgr_dir_stat(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb);

static iofunc_mount_t			Mount;
static iofunc_funcs_t			OcbFuncs;
static semaphore_t				NamedSemaphores;
static resmgr_connect_funcs_t	SemConnectFuncs, DirConnectFuncs;
static resmgr_io_funcs_t		SemIoFuncs, DirIoFuncs;
static resmgr_attr_t			ResmgrAttr;
static int						DirPathname, PrefixLength, MaxSems;
static pthread_mutex_t			SemaphoreMutex = PTHREAD_MUTEX_INITIALIZER;

/*
 *  Compensate for the (IMHO, buggy) _connect_object()/_unlink_object()
 *  name munging in the libc sem_open/unlink() covers.
 */
static char *name_sem(char *name)
{
	if (*name == '\0')
		return(NULL);
	if (strncmp(&NamedSemaphores.name[1], name, PrefixLength) || name[PrefixLength] != '/')
		return(name);
	return(&name[PrefixLength + 1]);
}

/*
 *  Manipulate a free list of 'blocked_t' items (avoid malloc/free).
 */
blocked_t *blocked_item(blocked_t *b)
{
	if (b != NULL) {
		b->link = NamedSemaphores.blocked, NamedSemaphores.blocked = b;
	}
	else if ((b = NamedSemaphores.blocked) != NULL) {
		NamedSemaphores.blocked = b->link;
	}
	else {
		b = _smalloc(sizeof(blocked_t));
	}
	return(b);
}

/*
 *  Enqueue a blocked sem_wait() client (FIFO by priority).
 */
static blocked_t *enqueue_client(semaphore_t *sem, resmgr_context_t *ctp)
{
blocked_t	*blk, **head;

	if ((blk = blocked_item(NULL)) != NULL) {
    	blk->rcvid = ctp->rcvid;
    	blk->scoid = ctp->info.scoid;
    	blk->coid = ctp->info.coid;
		blk->priority = ctp->info.priority;
#ifdef _POSIX_PRIORITY_SCHEDULING
		for (head = &sem->blocked; *head != NULL && (*head)->priority >= blk->priority; head = &(*head)->link)
			;
#else
		head = &sem->blocked;
#endif
		blk->link = *head, *head = blk;
		--sem->attr.nbytes;
	}
	return(blk);
}

/*
 *  Dequeue a blocked sem_wait() client (the head of the queue).
 */
static blocked_t *dequeue_client(semaphore_t *sem, int *rcvid, unsigned *pri)
{
blocked_t	*blk;

	if ((blk = sem->blocked) != NULL) {
		*rcvid = blk->rcvid, *pri = blk->priority;
		++sem->attr.nbytes;
		sem->blocked = blk->link;
		(void)blocked_item(blk);
	}
	return(blk);
}

/*
 *  Unblock all/specific clients (semaphore close or interrupt).  Remember
 *  that hanging off 'NamedSemaphores' is actually a free list of items!
 */
static void unblock_clients(semaphore_t *sem, resmgr_context_t *ctp, int match, int error)
{
blocked_t	**head, *b;

	if (!S_ISDIR(sem->attr.mode)) {
		head = &sem->blocked;
		while ((b = *head) != NULL) {
			if ((ctp == NULL) || (match && b->rcvid == ctp->rcvid) || (!match && b->scoid == ctp->info.scoid && b->coid == ctp->info.coid)) {
				if (error == EOK)
					MsgReply(b->rcvid, 0, NULL, 0);
				else
					MsgError(b->rcvid, error);
				++sem->attr.nbytes;
				*head = b->link;
				(void)blocked_item(b);
			}
			else {
				head = &b->link;
			}
		}
	}
}

/*
 *  Create a new semaphore, and link into the list of created semaphores.
 */
static semaphore_t *create_sem(semaphore_t *head, const char *name, struct _client_info *cred, iofunc_attr_t *attr, uint32_t *initial)
{
semaphore_t	*sem;

	if (head->attr.nbytes >= MaxSems) {
		errno = ENFILE;
	}
	else if ((sem = _smalloc(sizeof(semaphore_t))) != NULL) {
		size_t  slen = strlen(name) + 1;
		if ((sem->name = _smalloc(slen)) != NULL) {
			memcpy(sem->name, name, slen);	// '\0' guaranteed to be copied
			memcpy(&sem->attr, attr, sizeof(iofunc_attr_t));
			sem->attr.nbytes = sem->initial = ((initial != NULL) ? *initial : head->initial);
			sem->blocked = NULL;
			sem->link = head->link, head->link = sem;
			++head->attr.nbytes;
			return(sem);
		}
		else {
			errno = ENOMEM;
		}
		_sfree(sem, sizeof(semaphore_t));
	}
	else {
		errno = ENOMEM;
	}
	return(NULL);
}

/*
 *  Lookup the given semaphore in the list of created semaphores (and
 *  optional create a new one - atomic within mutex lock).
 */
static semaphore_t *lookup_sem(semaphore_t *head, const char *name, creation_t *create)
{
semaphore_t	*s;

	for (s = head->link; s != NULL; s = s->link)
		if (!strcmp(name, s->name))
			return(s);
	if (create == NULL)
		errno = ENOENT;
	else if (create->verification != EOK)
		errno = create->verification;
	else if ((s = create_sem(head, name, create->cred, create->attr, create->initial)) != NULL)
		*create->ioflag &= ~O_EXCL;
	return(s);
}

/*
 *  Cleanup the given semaphore (as a result of unlink() or close() handling).
 */
static void destroy_sem(semaphore_t *head, semaphore_t *sem, int force)
{
semaphore_t	**sp;

	if (head != NULL) {
		for (sp = &head->link; *sp != sem; sp = &(*sp)->link)
			;
		*sp = sem->link;
		--head->attr.nbytes;
	}
	if ((!sem->attr.nlink && !sem->attr.count) || force) {
		unblock_clients(sem, NULL, -1, EBADF);
		_sfree(sem->name, strlen(sem->name) + 1);
		_sfree(sem, sizeof(semaphore_t));
	}
}

/*
 *  Construct a stat structure from the given semaphore.
 */
static void detail_sem(resmgr_context_t *ctp, semaphore_t *sem, struct stat *st, int namespace)
{
	memset(st, 0, sizeof(struct stat));
	st->st_ino = (uintptr_t)sem;
#ifdef SEM_NEG_COUNT
	st->st_size = sem->attr.nbytes;
#if !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
	st->st_size_hi = (st->st_size < 0) ? -1 : 0;
#endif
#else
	st->st_size = (sem->attr.nbytes >= 0) ? sem->attr.nbytes : 0;
#endif
	st->st_dev = (ctp->info.srcnd << ND_NODE_BITS) | sem->attr.mount->dev;
	st->st_rdev = namespace ? S_INSEM : sem->attr.rdev;
	st->st_uid = sem->attr.uid, st->st_gid = sem->attr.gid;
	st->st_ctime = sem->attr.ctime, st->st_mtime = sem->attr.mtime, st->st_atime = sem->attr.atime;
	st->st_mode = sem->attr.mode, st->st_nlink = sem->attr.nlink;
}

/*
 *  Provide the nth item from the list of active named semaphores (readdir).
 */
static int get_nth_entry(semaphore_t *head, int index, semaphore_t **nth)
{
	while ((head = head->link) != NULL && --index >= 0)
		;
	return(*nth = head, index < 0);
}

/*
 *  Perform readdir() processing - list all the active named semaphores
 *  in a single ("/dev/sem") pseduo-directory.
 */
static int list_sem(resmgr_context_t *ctp, semaphore_t *dir, off64_t *offset, struct dirent *dp, int dpsize, int xtra, int *nbytes)
{
semaphore_t					*sem;
struct dirent_extra_stat	*dextra;
size_t							nmlen;

	*nbytes = 0;
	for (;;) {
		if (!get_nth_entry(dir, *offset, &sem))
			return(EOK);
		nmlen = strlen(sem->name);
		if (dpsize < ((offsetof(struct dirent, d_name) + nmlen + 1 + 7) & ~7))
			break;
		memset(dp, 0, sizeof(struct dirent));
		dp->d_ino = (uintptr_t)sem;
		dp->d_offset = *offset;
		dp->d_namelen = nmlen;
		memcpy(dp->d_name, sem->name, nmlen + 1);
		dextra = (struct dirent_extra_stat *)_DEXTRA_FIRST(dp);
		dp->d_reclen = (char *)dextra - (char *)dp;
		if (xtra && dpsize >= ((dp->d_reclen + sizeof(struct dirent_extra_stat) + 7) & ~7)) {
			dextra->d_datalen = sizeof(struct stat);
			dextra->d_type = _DTYPE_LSTAT;
			detail_sem(ctp, sem, &dextra->d_stat, 1);
			dextra = (struct dirent_extra_stat *)_DEXTRA_NEXT(dextra);
			dp->d_reclen = (char *)dextra - (char *)dp;
		}
		*nbytes += dp->d_reclen, dpsize -= dp->d_reclen;
		dp = (struct dirent *)((char *)dp + dp->d_reclen);
		++*offset;
	}
	return((*nbytes != 0) ? EOK : EMSGSIZE);
}

/*
 *  Handle client/libc "sem_open()".
 */
int resmgr_sem_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
semaphore_t			*sem;
char				*name;
creation_t			create;
struct _client_info	cred;
iofunc_attr_t		attr;
int					error;

	if ((msg->connect.extra_type != _IO_CONNECT_EXTRA_NONE && msg->connect.extra_type != _IO_CONNECT_EXTRA_SEM) || msg->connect.file_type != _FTYPE_SEM)
		return(ENOSYS);
	if ((error = iofunc_client_info(ctp, msg->connect.ioflag, &cred)) != EOK)
		return(error);
	if ((name = name_sem(msg->connect.path)) == NULL)
		return(EINVAL);
	if (msg->connect.ioflag & O_CREAT) {
		create.ioflag = &msg->connect.ioflag;
		msg->connect.mode = S_IFNAM | (msg->connect.mode & ~S_IFMT);
		create.verification = iofunc_open(ctp, msg, create.attr = &attr, &handle->attr, create.cred = &cred);
		(void)iofunc_time_update(&attr);
		if (msg->connect.extra_type != _IO_CONNECT_EXTRA_SEM)
			create.initial = NULL;
		else if (msg->connect.extra_len != sizeof(uint32_t))
			create.verification = ENOSYS;
		else if (*(create.initial = (uint32_t *)extra) > SEM_VALUE_MAX)
			create.verification = EINVAL;
	}
	else {
		create.verification = ENOENT;
	}
	_mutex_lock(&SemaphoreMutex);
	if ((sem = lookup_sem(handle, name, &create)) == NULL)
		error = errno;
	else if ((error = iofunc_open(ctp, msg, &sem->attr, NULL, &cred)) == EOK)
		error = iofunc_ocb_attach(ctp, msg, NULL, &sem->attr, NULL);
	_mutex_unlock(&SemaphoreMutex);
	return((error != EOK || (ctp->info.flags & _NTO_MI_ENDIAN_DIFF)) ? error : resmgr_sem_stat(ctp, (io_stat_t *)msg, _resmgr_ocb(ctp, &ctp->info)));
}

/*
 *  Handle client/libc "sem_unlink()" (alternate implementation).
 */
int resmgr_sem_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
	return(resmgr_dir_unlink(ctp, msg, handle, extra));
}

/*
 *  Handle client/libc "sem_wait()/sem_trywait()".
 */
int resmgr_sem_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
semaphore_t	*sem;
int			error, nonblock;

	sem = OCB2SEM(ocb);
	if ((error = iofunc_read_verify(ctp, msg, ocb, &nonblock)) != EOK)
		return(error);
	if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
		return(ENOSYS);
	if (msg->i.nbytes != 0)
		return(EMSGSIZE);
	_mutex_lock(&SemaphoreMutex);
	if (sem->attr.nbytes > 0) {
		--sem->attr.nbytes;
		_IO_SET_READ_NBYTES(ctp, 0);
		error = EOK;
	}
	else if (nonblock) {
		error = EAGAIN;
	}
	else if (enqueue_client(sem, ctp) == NULL) {
		error = ENOMEM;
	}
	else {
		error = _RESMGR_NOREPLY;
	}
	_mutex_unlock(&SemaphoreMutex);
	return(error);
}

/*
 *  Handle client/libc "sem_post()".
 */
int resmgr_sem_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
semaphore_t	*sem;
unsigned	pri;
int			error, nonblock, rcvid;

	sem = OCB2SEM(ocb);
	if ((error = iofunc_write_verify(ctp, msg, ocb, &nonblock)) != EOK)
		return(error);
	if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
		return(ENOSYS);
	if (msg->i.nbytes != 0)
		return(EMSGSIZE);
	_mutex_lock(&SemaphoreMutex);
	if (sem->attr.nbytes >= SEM_VALUE_MAX) {
		error = EAGAIN;
	}
	else if (sem->attr.nbytes >= 0 || dequeue_client(sem, &rcvid, &pri) == NULL) {
		++sem->attr.nbytes;
		_IO_SET_WRITE_NBYTES(ctp, 0);
		error = EOK;
	}
	else {
		_mutex_unlock(&SemaphoreMutex);
		if (pri > ctp->info.priority) {
			MsgReply(rcvid, 0, NULL, 0);
			MsgReply(ctp->rcvid, 0, NULL, 0);
		}
		else {
			MsgReply(ctp->rcvid, 0, NULL, 0);
			MsgReply(rcvid, 0, NULL, 0);
		}
		return(_RESMGR_NOREPLY);
	}
	_mutex_unlock(&SemaphoreMutex);
	return(error);
}

/*
 *  Handle client/libc "sem_close()".
 */
int resmgr_sem_close(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb)
{
semaphore_t	*sem;

	sem = OCB2SEM(ocb);
	_mutex_lock(&SemaphoreMutex);
	(void)iofunc_ocb_detach(ctp, ocb);
	(*sem->attr.mount->funcs->ocb_free)(ocb);
	unblock_clients(sem, ctp, 0, EBADF);
	destroy_sem(NULL, sem, 0);
	_mutex_unlock(&SemaphoreMutex);
	return(EOK);
}

/*
 *  Handle client/libc "sem_getvalue()" (alternate implementation).
 */
int resmgr_sem_stat(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb)
{
	detail_sem(ctp, OCB2SEM(ocb), &msg->o, 0);
	return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)));
}

/*
 *  Handle client/libc "sem_getvalue()" (historic implementation).
 */
int	resmgr_sem_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb)
{
semaphore_t		*sem;
struct mq_attr	*value;
int				error;

	sem = OCB2SEM(ocb);
	switch (msg->i.dcmd) {
	case DCMD_MISC_MQGETATTR:
		memset(value = (struct mq_attr *)_DEVCTL_DATA(msg->i), 0, sizeof(struct mq_attr));
#ifdef SEM_NEG_COUNT
		value->mq_curmsgs = sem->attr.nbytes;
#else
		value->mq_curmsgs = (sem->attr.nbytes >= 0) ? sem->attr.nbytes : 0;
#endif
		if (ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			ENDIAN_SWAP32(&value->mq_curmsgs);
		}
		error = _RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + sizeof(struct mq_attr));
		break;
	default:
		error = iofunc_devctl_default(ctp, msg, ocb);
		break;
	}
	return(error);
}

/*
 *  Handle unblocking (client sem_wait()).
 */
int	resmgr_sem_unblock(resmgr_context_t *ctp, io_pulse_t *msg, RESMGR_OCB_T *ocb)
{
struct _msg_info	info;

	if (MsgInfo(ctp->rcvid, &info) != -1 && (info.flags & _NTO_MI_UNBLOCK_REQ)) {
		_mutex_lock(&SemaphoreMutex);
		unblock_clients(OCB2SEM(ocb), ctp, 1, EINTR);
		_mutex_unlock(&SemaphoreMutex);
	}
	return(_RESMGR_NOREPLY);
}

/*
 *  Handle filesystem/pathname "open()".  The directory can be opened
 *  for readdir but the semaphores cannot be accessed except for stat.
 */
int resmgr_dir_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
semaphore_t			*sem;
char				*name;
struct _client_info	cred;
int					error;

	if (msg->connect.extra_type != _IO_CONNECT_EXTRA_NONE || msg->connect.file_type != _FTYPE_ANY)
		return(ENOSYS);
	if ((error = iofunc_client_info(ctp, msg->connect.ioflag, &cred)) != EOK)
		return(error);
	if ((name = name_sem(msg->connect.path)) != NULL && msg->connect.ioflag & (_IO_FLAG_RD | _IO_FLAG_WR))
		return(EACCES);
	_mutex_lock(&SemaphoreMutex);
	sem = (name == NULL) ? handle : lookup_sem(handle, name, NULL);
	if (sem == NULL)
		error = (msg->connect.ioflag & O_CREAT) ? EACCES : ENOENT;
	else if ((error = iofunc_open(ctp, msg, &sem->attr, NULL, &cred)) == EOK)
		error = iofunc_ocb_attach(ctp, msg, NULL, &sem->attr, NULL);
	_mutex_unlock(&SemaphoreMutex);
	return(error);
}

/*
 *  Handle filesystem/pathname "unlink()" (and historic sem_unlink()).
 */
int resmgr_dir_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
semaphore_t			*sem;
char				*name;
struct _client_info	cred;
int					error;

	if (msg->connect.extra_type != _IO_CONNECT_EXTRA_NONE)
		return(ENOSYS);
	if ((error = iofunc_client_info(ctp, msg->connect.ioflag, &cred)) != EOK)
		return(error);
	if ((name = name_sem(msg->connect.path)) == NULL)
		return(S_ISDIR(msg->connect.mode) ? EBUSY : ENOENT);
	_mutex_lock(&SemaphoreMutex);
	if ((sem = lookup_sem(handle, name, NULL)) == NULL)
		error = ENOENT;
	else if ((error = iofunc_unlink(ctp, msg, &sem->attr, &handle->attr, &cred)) == EOK)
		destroy_sem(handle, sem, 0);
	_mutex_unlock(&SemaphoreMutex);
	return(error);
}

/*
 *  Handle filesystem/pathname "readdir()".
 */
int resmgr_dir_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
struct _client_info	cred;
int					error, xtra, nbytes;

	if ((error = iofunc_read_verify(ctp, msg, ocb, NULL)) != EOK)
		return(error);
	if (!S_ISDIR(ocb->attr->mode))
		return(ENOTDIR);
	if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
		return(ENOSYS);
	xtra = (msg->i.xtype & _IO_XFLAG_DIR_EXTRA_HINT) != 0 && iofunc_client_info(ctp, ocb->ioflag, &cred) == EOK && iofunc_check_access(NULL, ocb->attr, S_IXUSR, &cred) == EOK;
	_mutex_lock(&SemaphoreMutex);
	error = list_sem(ctp, OCB2SEM(ocb), &ocb->offset, (struct dirent *)msg, min(msg->i.nbytes, ctp->msg_max_size - ctp->offset), xtra, &nbytes);
	_mutex_unlock(&SemaphoreMutex);
	ocb->attr->flags |= IOFUNC_ATTR_ATIME;
	resmgr_endian_context(ctp, _IO_READ, S_IFDIR, 0);
	_IO_SET_READ_NBYTES(ctp, nbytes);
	return((error != EOK) ? error : _RESMGR_PTR(ctp, msg, nbytes));
}

/*
 *  Handle filesystem/pathname "close()".
 */
int resmgr_dir_close(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb)
{
	return(resmgr_sem_close(ctp, reserved, ocb));
}

/*
 *  Handle filesystem/pathname "fstat()".
 */
int resmgr_dir_stat(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb)
{
	(void)iofunc_time_update(ocb->attr);
	if (S_ISDIR(ocb->attr->mode)) {
		(void)iofunc_stat(ctp, ocb->attr, &msg->o);
	}
	else {
		detail_sem(ctp, OCB2SEM(ocb), &msg->o, 1);
	}
	return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)));
}

/*
 *  Specific OCB allocation/free routines.
 */
static RESMGR_OCB_T *resmgr_ocb_calloc(resmgr_context_t *ctp, iofunc_attr_t *attr)
{
RESMGR_OCB_T	*ocb;

	if ((ocb = _scalloc(sizeof(RESMGR_OCB_T))) != NULL)
		return(ocb);
	errno = ENFILE;
	return(NULL);
}
static void resmgr_ocb_free(RESMGR_OCB_T *ocb)
{
	_sfree(ocb, sizeof(RESMGR_OCB_T));
}

/*
 *  Initialise the named semaphore subsystem.
 */
void namedsem_init(void)
{
extern int	max_fds;

	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &SemConnectFuncs, _RESMGR_IO_NFUNCS, &SemIoFuncs);
	SemConnectFuncs.open = resmgr_sem_open, SemConnectFuncs.unlink = resmgr_sem_unlink;
	SemIoFuncs.read = resmgr_sem_read, SemIoFuncs.write = resmgr_sem_write;
	SemIoFuncs.close_ocb = resmgr_sem_close, SemIoFuncs.stat = resmgr_sem_stat;
	SemIoFuncs.devctl = resmgr_sem_devctl, SemIoFuncs.unblock = resmgr_sem_unblock;
	SemIoFuncs.lock_ocb = NULL, SemIoFuncs.unlock_ocb = NULL;
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &DirConnectFuncs, _RESMGR_IO_NFUNCS, &DirIoFuncs);
	DirConnectFuncs.open = resmgr_dir_open, DirConnectFuncs.unlink = resmgr_dir_unlink;
	DirIoFuncs.read = resmgr_dir_read, DirIoFuncs.close_ocb = resmgr_dir_close, DirIoFuncs.stat = resmgr_dir_stat;
	DirIoFuncs.lock_ocb = NULL, DirIoFuncs.unlock_ocb = NULL;
	OcbFuncs.nfuncs =_IOFUNC_NFUNCS;
	OcbFuncs.ocb_calloc = resmgr_ocb_calloc, OcbFuncs.ocb_free = resmgr_ocb_free;

	memset(&ResmgrAttr, 0, sizeof(ResmgrAttr));
	ResmgrAttr.flags = RESMGR_FLAG_CROSS_ENDIAN;
	ResmgrAttr.nparts_max = 1, ResmgrAttr.msg_max_size = sizeof(struct _io_connect) + _POSIX_PATH_MAX;

	memset(&NamedSemaphores, 0, sizeof(NamedSemaphores));
	iofunc_attr_init(&NamedSemaphores.attr, S_IFDIR | S_ISVTX | S_IPERMS, NULL, NULL);
	NamedSemaphores.attr.mount = &Mount;
	NamedSemaphores.attr.flags |= IOFUNC_ATTR_SYNTHETIC;
	++NamedSemaphores.attr.nlink;
	NamedSemaphores.initial = 0;
	PrefixLength = strlen(NamedSemaphores.name = "/dev/sem") - 1;
	memset(&Mount, 0, sizeof(Mount));
	Mount.conf = IOFUNC_PC_CHOWN_RESTRICTED | IOFUNC_PC_NO_TRUNC;
	Mount.funcs = &OcbFuncs;
	NamedSemaphores.attr.rdev = resmgr_attach(dpp, &ResmgrAttr, NULL, _FTYPE_SEM, _RESMGR_FLAG_DIR | _RESMGR_FLAG_FTYPEONLY, &SemConnectFuncs, &SemIoFuncs, &NamedSemaphores);
	NamedSemaphores.attr.inode = (uintptr_t)&NamedSemaphores;
	DirPathname = resmgr_attach(dpp, &ResmgrAttr, NamedSemaphores.name, _FTYPE_ANY, _RESMGR_FLAG_DIR, &DirConnectFuncs, &DirIoFuncs, &NamedSemaphores);
	rsrcdbmgr_proc_devno(_MAJOR_FSYS, &Mount.dev, -1, 0);

	sysmgr_conf_set(0, _CONF_NUM, _SC_SEMAPHORES, _POSIX_SEMAPHORES, NULL);
	/*
	 * Set _SC_SEM_NSEMS_MAX to -1 (indeterminate value) since the limit
	 * applies to both unnamed and named sempahores.
	 * There is no limit for unnamed semaphores since they are sync objects.
	 */
	MaxSems = max((max_fds - STDERR_FILENO) - 2, _POSIX_SEM_NSEMS_MAX);
	sysmgr_conf_set(0, _CONF_NUM, _SC_SEM_NSEMS_MAX, -1, NULL);
	sysmgr_conf_set(0, _CONF_NUM, _SC_SEM_VALUE_MAX, SEM_VALUE_MAX, NULL);
}

__SRCVERSION("namedsem.c $Rev: 164026 $");
