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
 *  "libmq"  -  message queues via kernel asynchronous messaging
 *
 *  John Garvey, QNX Software Systems Ltd
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <mqueue.h>
#include <pthread.h>
#include <share.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/asyncmsg.h>
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <sys/stat.h>

/* This signature is intended to separate new mqd_t from old mqd_t/fd */
#define MQ_MAGIC	0x716D0000

static struct {
	pthread_mutex_t		mutex;
	struct mqd			*queues;
	uint16_t			sequence;
} _mqctrl = { PTHREAD_MUTEX_INITIALIZER, NULL, 0 };

struct mqd {
	mqd_t			id;
	int				ioflag;
	int				fd;
	int				chid;
	int				coid;
	struct mq_attr	attr;
	struct mqd		*link;
};

/*
 *  Convenience routine to search the list of open mqueues (used by
 *  mqlookup/insert/delete below, under mutex control).  This linked-list
 *  of mqd_t IDs is used for two reasons: to allow detection of EBADF
 *  values, and to prevent valid values being used as fds to old mq_*().
 */
static struct mqd *mqmatch(mqd_t id, struct mqd ***prev)
{
struct mqd	*mq;

	for (*prev = &_mqctrl.queues; (mq = **prev) != NULL; *prev = &mq->link) {
		if (mq->id == id)
			return(mq);
	}
	return(NULL);
}

/*
 *  Search the linked list of open mqueues for a match (and shuffle any
 *  match to the front of this MRU list); a copy of the mqueue (rather
 *  than a pointer into this list) is returned to avoid needing to hold
 *  the internal mutex around the outer (kernel) call.
 */
static int mqlookup(mqd_t id, struct mqd *copy)
{
struct mqd	**mpp, *mp;

	if ((id & 0xFFFF0000) == MQ_MAGIC) {
		_mutex_lock(&_mqctrl.mutex);
		if ((mp = mqmatch(id, &mpp)) != NULL) {
			if (mpp != &_mqctrl.queues)
				*mpp = mp->link, mp->link = _mqctrl.queues, _mqctrl.queues = mp;
			memcpy(copy, mp, sizeof(struct mqd));
			_mutex_unlock(&_mqctrl.mutex);
			return(!0);
		}
		_mutex_unlock(&_mqctrl.mutex);
	}
	return(0);
}

/*
 *  A two-part lookup/modify operation; unlike mqlookup() a pointer
 *  to the actual in-place mqueue is returned and the mutex remains
 *  locked until an external operation is completed (mq_setattr).
 */
static struct mqd *mqmodify(mqd_t id, int modified)
{
struct mqd	**mpp, *mp;

	if (modified) {
		_mutex_unlock(&_mqctrl.mutex);
	}
	else if ((id & 0xFFFF0000) == MQ_MAGIC) {
		_mutex_lock(&_mqctrl.mutex);
		if ((mp = mqmatch(id, &mpp)) != NULL)
			return(mp);
		_mutex_unlock(&_mqctrl.mutex);
	}
	return(NULL);
}

/*
 *  Insert the new mqueue into the linked list (assigning a unique/cyclic
 *  identifier for use as the mqd_t).
 */
static int mqinsert(struct mqd *created)
{
struct mqd	**mpp;
uint16_t	wrap;

	_mutex_lock(&_mqctrl.mutex);
	wrap = ++_mqctrl.sequence;
	do {
		if (mqmatch(created->id = MQ_MAGIC | _mqctrl.sequence, &mpp) == NULL) {
			created->link = _mqctrl.queues, _mqctrl.queues = created;
			_mutex_unlock(&_mqctrl.mutex);
			return(!0);
		}
	} while (++_mqctrl.sequence != wrap);
	_mutex_unlock(&_mqctrl.mutex);
	return(0);
}

/*
 *  Delete the old mqueue from the linked list.
 */
static struct mqd *mqdelete(mqd_t id)
{
struct mqd	**mpp, *mp;

	_mutex_lock(&_mqctrl.mutex);
	if ((mp = mqmatch(id, &mpp)) != NULL) {
		*mpp = mp->link;
		_mutex_unlock(&_mqctrl.mutex);
		return(mp);
	}
	_mutex_unlock(&_mqctrl.mutex);
	return(NULL);
}

/*
 *  mq_open()  POSIX 1003.1b/15.2.1
 */
mqd_t mq_open(const char *name, int oflag, ...)
{
struct mqd		*mq;
va_list			args;
struct stat		st;
struct mq_attr	*attr;
mode_t			mode;
int				error;

	if (oflag & O_CREAT) {
		va_start(args, oflag);
		mode = va_arg(args, mode_t) & ~S_IFMT;
		attr = va_arg(args, struct mq_attr *);
		va_end(args);
	}
	else {
		mode = 0;
		attr = NULL;
	}
	if ((mq = malloc(sizeof(struct mqd))) != NULL) {
		if ((mq->fd = _connect(0, name, mode, oflag | O_CLOEXEC, SH_DENYNO, _IO_CONNECT_OPEN, 0, _IO_FLAG_RD | _IO_FLAG_WR, _FTYPE_MQUEUE, _IO_CONNECT_EXTRA_MQUEUE, (attr != NULL) ? offsetof(struct mq_attr, mq_sendwait) : 0, attr, sizeof(struct stat), &st, NULL)) != -1) {
			mq->ioflag = (oflag & ~O_ACCMODE) | ((oflag + 1) & O_ACCMODE);
			memset(&mq->attr, 0, sizeof(struct mq_attr));
			mq->attr.mq_maxmsg = st.st_nblocks;
			mq->attr.mq_msgsize = st.st_blocksize;
			mq->attr.mq_flags = mq->ioflag & O_NONBLOCK;
			mq->chid = st.st_rdev;
			if ((mq->coid = ConnectAttachExt(ND_LOCAL_NODE, 0, mq->chid, 0, _NTO_COF_CLOEXEC, NULL)) != -1) {
				if (mqinsert(mq)) {
					return(mq->id);
				}
				else {
					error = EMFILE;
				}
				ConnectDetach(mq->coid);
			}
			else {
				error = errno;
			}
			close(mq->fd);
		}
		else {
			error = errno;
		}
		free(mq);
	}
	else {
		error = ENOMEM;
	}
	return(errno = error, (mqd_t)-1);
}

/*
 *  mq_close()  POSIX 1003.1b/15.2.2
 */
int mq_close(mqd_t mq)
{
struct mqd	*q;

	if ((q = mqdelete(mq)) == NULL)
		return(errno = EBADF, -1);
	ConnectDetach(q->coid);
	close(q->fd);
	free(q);
	return(0);
}

/*
 *  mq_unlink()  POSIX 1003.1b/15.2.3
 */
int mq_unlink(const char *name)
{
int		fd;

	if ((fd = _connect(_NTO_SIDE_CHANNEL, name, 0, 0, SH_DENYNO, _IO_CONNECT_UNLINK, 0, 0, _FTYPE_MQUEUE, _IO_CONNECT_EXTRA_NONE, 0, NULL, 0, NULL, NULL)) == -1)
		return(-1);
    ConnectDetach(fd);
    return(0);
}


int mq_timedsend_woption(mqd_t mq, const char *msg, size_t len, unsigned priority, const struct timespec *timeout, clockid_t clock_choice)
/* clock_choice is either CLOCK_MONOTONIC or CLOCK_REALTIME */
{
struct mqd	q;
uint64_t	t;

	if (timeout && !TIMESPEC_VALID(timeout)) { 
		errno=EINVAL;
		return -1;
	}

	if (!mqlookup(mq, &q)) {
		errno = EBADF;
	}
	else if (!(q.ioflag & _IO_FLAG_WR)) {
		errno = EBADF;
	}
	else if (len > q.attr.mq_msgsize) {
		errno = EMSGSIZE;
	}
	else if (priority >= MQ_PRIO_MAX) {
		errno = EINVAL;
	}
	else {
		if (q.attr.mq_flags & O_NONBLOCK) {
			TimerTimeout(clock_choice, _NTO_TIMEOUT_SEND, NULL, NULL, NULL);
		}
		else if (timeout != NULL) {
			t = timespec2nsec(timeout);
			TimerTimeout(clock_choice, TIMER_ABSTIME | _NTO_TIMEOUT_SEND, NULL, &t, NULL);
		}
		if (MsgSendAsyncGbl(q.coid, msg, len, priority) != -1) {
			return(0);
		}
		else if (errno == ETIMEDOUT && q.attr.mq_flags & O_NONBLOCK) {
			errno = EAGAIN;
		}
	}
	return(-1);
}





ssize_t mq_timedreceive_woption(mqd_t mq, char *msg, size_t len, unsigned *priority, const struct timespec *timeout, clockid_t clock_choice)
/* clock_choice is CLOCK_REALTIME or CLOCK_MONOTONIC */
{
struct _msg_info	info;
struct mqd			q;
uint64_t			t;

	if (timeout && !TIMESPEC_VALID(timeout)) { 
		errno=EINVAL;
		return -1;
	}

	if (!mqlookup(mq, &q)) {
		errno = EBADF;
	}
	else if (!(q.ioflag & _IO_FLAG_RD)) {
		errno = EBADF;
	}
	else if (len < q.attr.mq_msgsize) {
		errno = EMSGSIZE;
	}
	else {
		if (q.attr.mq_flags & O_NONBLOCK) {
			TimerTimeout(clock_choice, _NTO_TIMEOUT_RECEIVE, NULL, NULL, NULL);
		}
		else if (timeout != NULL) {
			t = timespec2nsec(timeout);
			TimerTimeout(clock_choice, TIMER_ABSTIME | _NTO_TIMEOUT_RECEIVE, NULL, &t, NULL);
		}
		if (MsgReceiveAsyncGbl(q.chid, msg, len, &info, q.coid) != -1) {
			if (priority != NULL)
				*priority = info.priority;
			return(info.msglen);
		}
		else if (errno == ETIMEDOUT && q.attr.mq_flags & O_NONBLOCK) {
			errno = EAGAIN;
		}
	}
	return(-1);
}


/*
 *  mq_timedreceive()  POSIX 1003.1d/15.2.5
 */
ssize_t mq_timedreceive(mqd_t mq, char *msg, size_t len, unsigned *priority, const struct timespec *timeout)
{
	return mq_timedreceive_woption(mq, msg, len, priority, timeout, CLOCK_REALTIME);
}


/*
 *  a version of mq_timedreceive() which is not affected by TOD changes. Not POSIX compliant.
 */
ssize_t mq_timedreceive_monotonic(mqd_t mq, char *msg, size_t len, unsigned *priority, const struct timespec *abs_mono_timeout)
{
	return mq_timedreceive_woption(mq, msg, len, priority, abs_mono_timeout, CLOCK_MONOTONIC);
}


/*
 *  mq_receive()  POSIX 1003.1b/15.2.5
 */
ssize_t mq_receive(mqd_t mq, char *msg, size_t len, unsigned *priority)
{
	return(mq_timedreceive_woption(mq, msg, len, priority, NULL, CLOCK_REALTIME));
}


/*
 *  mq_timedsend()  POSIX 1003.1d/15.2.4
 */
int mq_timedsend(mqd_t mq, const char *msg, size_t len, unsigned priority, const struct timespec *timeout) { 
	return mq_timedsend_woption(  mq, msg, len, priority, timeout, CLOCK_REALTIME);
}


/*
 *  A version of mq_timedsend()  which is unaffeted by TOD changes. Not POSIX compliant. 
 */
int mq_timedsend_monotonic(mqd_t mq, const char *msg, size_t len, unsigned priority, const struct timespec *abs_mono_timeout) { 
	return mq_timedsend_woption( mq, msg, len, priority, abs_mono_timeout, CLOCK_MONOTONIC);
}


/*
 *  mq_send()  POSIX 1003.1b/15.2.4
 */
int mq_send(mqd_t mq, const char *msg, size_t len, unsigned priority)
{
	return(mq_timedsend_woption(mq, msg, len, priority, NULL, CLOCK_REALTIME));
}

/*
 *  mq_notify()  POSIX 1003.1b/15.2.6
 */
int mq_notify(mqd_t mq, const struct sigevent *event)
{
struct mqd					q;
union _channel_connect_attr	qattr;

	if (!mqlookup(mq, &q))
		return(errno = EBADF, -1);
	if (event == NULL)
		SIGEV_NONE_INIT(&qattr.ev.event);
	else if (SIGEV_GET_TYPE(event) == SIGEV_SIGNAL)
		SIGEV_SIGNAL_CODE_INIT(&qattr.ev.event, event->sigev_signo, event->sigev_value.sival_ptr, SI_MESGQ);
	else
		memcpy(&qattr.ev.event, event, sizeof(struct sigevent));
	qattr.ev.coid = q.coid;
	return(ChannelConnectAttr(q.chid, NULL, &qattr, _NTO_CHANCON_ATTR_EVENT));
}

/*
 *  mq_getattr()  POSIX 1003.1b/15.2.8
 */
int mq_getattr(mqd_t mq, struct mq_attr *mqattr)
{
struct mqd					q;
union _channel_connect_attr	qattr;

	if (!mqlookup(mq, &q))
		return(errno = EBADF, -1);
	if (ChannelConnectAttr(q.chid, &qattr, NULL, _NTO_CHANCON_ATTR_CURMSGS) == -1)
		return(-1);
	memcpy(mqattr, &q.attr, sizeof(struct mq_attr));
	mqattr->mq_flags &= O_NONBLOCK;
	mqattr->mq_curmsgs = qattr.num_curmsgs;
	return(0);
}

/*
 *  mq_setattr()  POSIX 1003.1b/15.2.7
 */
int mq_setattr(mqd_t mq, const struct mq_attr *mqattr, struct mq_attr *oldattr)
{
struct mqd		*q;
struct mq_attr	saved;

	if (oldattr != NULL && mq_getattr(mq, (oldattr == mqattr) ? &saved : oldattr) == -1)
		return(-1);
	if ((q = mqmodify(mq, 0)) == NULL)
		return(errno = EBADF, -1);
	q->attr.mq_flags = (q->attr.mq_flags & ~O_NONBLOCK) | (mqattr->mq_flags & O_NONBLOCK);
	mqmodify(mq, !0);
	if (oldattr != NULL && oldattr == mqattr)
		memcpy(oldattr, &saved, sizeof(struct mq_attr));
	return(0);
}

__SRCVERSION("mq.c $Rev: 200517 $");
