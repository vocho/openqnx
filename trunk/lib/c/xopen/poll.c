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





#include <sys/poll.h>
#include <sys/select.h>
#include <sys/siginfo.h>
#include <sys/iomsg.h>
#include <errno.h>
#include <atomic.h>
#include <unistd.h>
#include <malloc.h>
#include <alloca.h>

static unsigned serial_number = 0;

static int _poll(struct pollfd *fds, nfds_t nfds, int fd_first, int64_t timo, int extra_mask, int *nebadfp);

static __inline int
checkvalid(struct pollfd *pfd)
{
	int val;

	val = pfd->revents & (POLLRDNORM | POLLWRNORM | POLLRDBAND);

	if ((val & pfd->events) != val) {
		return 0;
	}
	return 1;
}

int
poll(struct pollfd *fds, nfds_t nfds, int timo)
{
	int yet_to_find_one, trailers, fd_first, i;

	yet_to_find_one = 1;
	trailers = 0;
	fd_first = 0;

	for (i = 0; i < nfds; i++) {
		/*
		 * POLLRESERVED is really unavailable because it
		 * can never be returned out of info.si_code
		 * below as it (the code) must be -ve.
		 *
		 * We use it as an indicator to manager(s)
		 * that this fd has yet to be serviced.
		 * If they support the extended io_notify_t
		 * message, they must knock this bit down
		 * as they go.
		 */
		if (fds[i].fd < 0) {
			fds[i].events &= ~POLLRESERVED;
			fd_first += yet_to_find_one;
			trailers++;
		}
		else {
			fds[i].events |= POLLRESERVED;
			yet_to_find_one = 0;
			trailers = 0;
		}
		fds[i].revents = 0;
	}

	nfds -= trailers;

	/* timo is in msec.  Pass in nsec */
	return _poll(fds, nfds, fd_first, timo * (int64_t)1000000, POLLERR | POLLHUP | POLLNVAL, NULL);
}

int
select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *tvptr)
{
	int nfds_real, i, j, was_set, ret, setsize, largeset;
	struct pollfd *pollfds;
	int64_t timo;
	int nebadf = 0;

	if (tvptr == NULL) {
		timo = -1;
	}
	else if (tvptr->tv_usec < 0) {
		errno = EINVAL;
		return -1;
	}
	else {
		timo = tvptr->tv_sec * (int64_t)1000000000 + tvptr->tv_usec * (int64_t)1000;
	}

	largeset = 0;
	i = nfds * sizeof(*pollfds);
	if(__stackavail() < i + 1024 || (pollfds = alloca(i)) == NULL) {
		if ((pollfds = malloc(i)) == NULL)
			return -1;

		largeset = 1;
	}

	for (i = 0, nfds_real = 0; i < nfds; i++) {
		was_set = 0;
		pollfds[nfds_real].events  = 0;
		pollfds[nfds_real].revents = 0;

		if (readfds && FD_ISSET(i, readfds)) {
			pollfds[nfds_real].events |= POLLRDNORM | POLLRESERVED;
			pollfds[nfds_real].fd = i;
			was_set = 1;
		}
		if (writefds && FD_ISSET(i, writefds)) {
			pollfds[nfds_real].events |= POLLWRNORM | POLLRESERVED;
			pollfds[nfds_real].fd = i;
			was_set = 1;
		}
		if (exceptfds && FD_ISSET(i, exceptfds)) {
			pollfds[nfds_real].events |= POLLRDBAND | POLLRESERVED;
			pollfds[nfds_real].fd = i;
			was_set = 1;
		}
		nfds_real += was_set;
	}


	if ((ret = _poll(pollfds, nfds_real, 0, timo, 0, &nebadf)) == -1 || nebadf > 0) {
		if (largeset)
			free(pollfds);

		if (nebadf > 0)
			errno = EBADF;

		return -1;
	}

	/*
	 * Can't use sizeof (fd_set) because client's view of
	 * this can vary depending on how it was compiled.
	 */
	setsize = ((nfds + sizeof(int) * CHAR_BIT - 1) / (sizeof(int) * CHAR_BIT)) * sizeof(int);

	if (readfds)
		memset(readfds, 0x00, setsize);
	if (writefds)
		memset(writefds, 0x00, setsize);
	if (exceptfds)
		memset(exceptfds, 0x00, setsize);

	was_set = 0;
	for (i = 0; ret > 0; i++) {
		/*
		 * First test below guards against a server raising
		 * an event with no flags set, second catches a
		 * server indicating flags that weren't asked for.
		 */
		if (i >= nfds_real || checkvalid(&pollfds[i]) == 0) {
			/*
			 * Server needs to be fixed.
			 *
			 * Erroring here isn't posix as the fd sets aren't
			 * supposed to be modified on error; however we
			 * can't be posix in this case as we aren't running
			 * against a posix server.
			 */
			was_set = -1;
			errno = EBADMSG;
			break;
		}

		j = 0;
		if (pollfds[i].revents & POLLRDNORM) {
			FD_SET(pollfds[i].fd, readfds);
			was_set++;
			j = 1;
		}
		if (pollfds[i].revents & POLLWRNORM) {
			FD_SET(pollfds[i].fd, writefds);
			was_set++;
			j = 1;
		}
		if (pollfds[i].revents & POLLRDBAND) {
			FD_SET(pollfds[i].fd, exceptfds);
			was_set++;
			j = 1;
		}
		ret -= j;
	}

	if (largeset)
		free(pollfds);

	return was_set;
}

static int
_poll(struct pollfd *fds, nfds_t nfds, int fd_first, int64_t timo, int extra_mask, int *nebadfp)
{
	io_notify_t msg;
	iov_t iov[2];
	sigset_t set;
	siginfo_t info;
	int sernum, curridx, action, nebadf;
	int exten_flag;


	msg.i.flags_extra_mask = extra_mask;

	msg.i.event.sigev_notify = SIGEV_SIGNAL_THREAD;
	msg.i.event.sigev_signo = SIGSELECT;  /* This signal is always SIGBLOCKed */
	/*
	 * sigev_value, and possibly sigev_code
	 * will be modified by manager.
	 */
	msg.i.event.sigev_code = SI_NOTIFY;
	msg.i.event.sigev_priority = -1;

	msg.i.mgr[0] = 0;
	msg.i.mgr[1] = 0;

	/*
	 * Pass in the timeout.  This is an optimization for the case where
	 * the manager can unblock the client itself after timeout without
	 * an unblock pulse (eg tcpip stack), and services all fds in the
	 * set.  Otherwise, manger has to let us block in SignalWaitinfo()
	 * below.
	 */
	msg.i.timo       = timo;
	msg.i.nfds       = nfds;
	msg.i.fd_first   = fd_first;
	msg.i.nfds_ready = 0;

	/* Need to keep the serial numbers different across calls. */
	sernum = atomic_add_value(&serial_number, nfds) & _NOTIFY_DATA_MASK;

	nebadf = 0;

	SETIOV(&iov[0], &msg, sizeof(msg));

	if (timo != 0)
		action = _NOTIFY_ACTION_POLLARM;
	else
		action = _NOTIFY_ACTION_POLL;

	exten_flag = _NOTIFY_COND_EXTEN;

	iov[1].iov_base = fds;

	while (msg.i.fd_first < msg.i.nfds) {
		curridx = msg.i.fd_first;

		/* Managers can shrink end of array as they process it */
		iov[1].iov_len = msg.i.nfds * sizeof(*fds);

		/*
		 * Need to reset these every time through the loop as
		 * they gets overwritten by msg.o members of union.
		 */
		msg.i.type = _IO_NOTIFY;
		msg.i.combine_len = sizeof(msg.i);
		msg.i.action = action;

		msg.i.flags_exten = 0;

		msg.i.flags = (unsigned short)fds[curridx].events |
		    ((unsigned short)fds[curridx].events << 28);
		msg.i.flags &= ~POLLRESERVED;
		msg.i.flags |= exten_flag | extra_mask;

		msg.i.event.sigev_value.sival_int = (sernum + curridx) & _NOTIFY_DATA_MASK;

		if (fds[curridx].fd < 0) {
			msg.i.fd_first++;
			continue;
		}

		if (MsgSendv(fds[curridx].fd, iov, 2, iov, 2) == -1) {
			if (errno == ENOSYS) {
#if 1
/*
 * It's been observed that at least one mgr (Photon)
 * actually returns ENOSYS if it sees events it doesn't
 * support.  This seems heavy as most mgrs simply ignore 
 * (don't trigger) these, plus it's slow here as it means
 * a double message pass.  This should hopefully only be
 * for backwards compatibility if we can get concensus
 * not to do that.
 */
				if (exten_flag != 0) {
					exten_flag = 0;
					continue;
				}
#endif
				if ((fds[curridx].revents = fds[curridx].events & (POLLRDNORM | POLLWRNORM)) != 0)
					msg.i.nfds_ready++;
			}
			else if (errno == EBADF) {
				/*
				 * Old style server.  New should set POLLNVAL
				 * themselves on first fd (the one the message
				 * comes in on).
				 */
				fds[curridx].revents = POLLNVAL;
			}
			else {
				return -1;
			}
			fds[curridx].events &= ~POLLRESERVED;
			msg.i.fd_first++;
		}
		else if (fds[curridx].events & POLLRESERVED) {
			/*
			 * Old style manager that doesn't look
			 * at extended io_notify_t.  Move over
			 * the minimum they should have returned.
			 */
			if (msg.o.flags) {
				msg.o.flags |= (msg.o.flags & ~_NOTIFY_COND_EXTEN) >> 28;
				fds[curridx].revents = msg.o.flags;
				msg.i.nfds_ready++;
			}
			fds[curridx].events &= ~POLLRESERVED;
			msg.i.fd_first++;
		}
		else {
			/*
			 * New style server which should:
			 *  - knock down POLLRESERVED in events member of appropriate fds.
			 *  - decrement msg.o.nfds to new value.
			 *  - increment msg.o.fd_first to new value.
			 *  - increment msg.o.nfds_ready
			 *  - set / clear appropriate bits in fds[].events / fds[].revents
			 */
		}

		if (fds[curridx].revents & POLLNVAL) {
			msg.i.nfds_ready++;
			nebadf++;
		}
		/*
		 * If they weren't all handled by first server, we can't
		 * allow subsequent ones to block us in the MsgSend().
		 */
		msg.i.timo = 0;

		if (msg.i.nfds_ready > 0)
			action = _NOTIFY_ACTION_POLL;

		exten_flag = _NOTIFY_COND_EXTEN;
	}

	if (action == _NOTIFY_ACTION_POLL) {
		if (nebadfp != NULL)
			*nebadfp = nebadf;
		return msg.i.nfds_ready;
	}

	sigemptyset(&set);
	sigaddset(&set, SIGSELECT);


	for (;;) {
		/*
		 * Posix says block indefinitely for timeout == -1.
		 * Here we apply for all timeouts < 0 as 0 case itself
		 * handled above.
		 */
		if (timo > 0 && TimerTimeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_SIGWAITINFO, 0, (uint64_t *)&timo, 0) == -1)
			return -1;
			

		if (SignalWaitinfo(&set, &info) == -1) {
			if (errno != ETIMEDOUT)
				return -1;
			return 0;
		}

		curridx = ((info.si_value.sival_int & _NOTIFY_DATA_MASK) - sernum) & _NOTIFY_DATA_MASK;
		if (curridx >= 0 && curridx < nfds) {
			if (info.si_value.sival_int & _NOTIFY_COND_EXTEN) {
				fds[curridx].revents = -info.si_code;
			}
			else {
				fds[curridx].revents = (info.si_value.sival_int & 0xf0000000) >> 28;
			}
			return 1;
		}
		/* Old event.  Ignore.  Timeout gets reset. */
	}
}

__SRCVERSION("poll.c $Rev: 200715 $");
