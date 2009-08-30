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




#include <errno.h>
#include <sys/iomsg.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <atomic.h>
#include <alloca.h>
#include <string.h>

static unsigned serial_number = 0;

int _select_event(int nfds, fd_set *readfds, fd_set *writefds,
			fd_set *exceptfds, const struct timespec *ts,
			struct sigevent *event, int (*eventwait)(const struct timespec *ts,
			union sigval *value, void *arg), void *arg) {
	struct _io_notify			msgi;
	int							sernum;
	int							fd, status, pollsuccess;
	union sigval				value;
	struct timespec				zero;
	/*
	 * Can't use sizeof (fd_set) because client's view of
	 * this can vary depending on how it was compiled.
	 */
	int setsize = ((nfds + sizeof(int) * CHAR_BIT - 1) / (sizeof(int) * CHAR_BIT)) * sizeof(int);
	int *saved_rd = 0, *saved_wr = 0, *saved_ex = 0;
	unsigned char *saved;

	if(!(saved = alloca(3 * setsize))) {
		errno = ENOMEM;
		return -1;
	}

	if(readfds) {
		saved_rd = (int *)(saved + (0 * setsize));
		memcpy(saved_rd, readfds, setsize);
	}

	if(writefds) {
		saved_wr = (int *)(saved + (1 * setsize));
		memcpy(saved_wr, writefds, setsize);
	}

	if(exceptfds) {
		saved_ex = (int *)(saved + (2 * setsize));
		memcpy(saved_ex, exceptfds, setsize);
	}

	// Figure out the real number of fds.
	for(fd = nfds - 1, nfds = 0; fd >= 0; fd--) {
		if(		(readfds && FD_ISSET(fd, readfds)) ||
				(writefds && FD_ISSET(fd, writefds)) ||
				(exceptfds && FD_ISSET(fd, exceptfds))) {
			nfds = fd + 1;
			break;
		}
	}

	/*
	 * We want to keep the serial numbers different across threads to
	 * reduce confusion.  atomic_add_value() returns the old value.
	 */

	sernum = atomic_add_value(&serial_number, nfds) & _NOTIFY_DATA_MASK;

	/* msg.i.flags gets set in the loop below */
	msgi.type = _IO_NOTIFY;
	msgi.combine_len = sizeof msgi;
	msgi.action = _NOTIFY_ACTION_POLLARM;
	msgi.event = *event;

	pollsuccess = 0;
	for(fd = 0; fd < nfds; fd++) {
		msgi.event.sigev_value.sival_int = (sernum + fd) & _NOTIFY_DATA_MASK;
		msgi.flags = 0;

		/* we clear as we go because the poll could succeed and we need to record it */
		if(readfds && FD_ISSET(fd, readfds)) {
			msgi.flags |= _NOTIFY_COND_INPUT;
		}

		if(writefds && FD_ISSET(fd, writefds)) {
			msgi.flags |= _NOTIFY_COND_OUTPUT;
		}
		
		if(exceptfds && FD_ISSET(fd, exceptfds)) {
			msgi.flags |= _NOTIFY_COND_OBAND;
			FD_CLR(fd, exceptfds);
		}

		/*
		 * Don't clear the bits until all the tests are done
		 * in case the fds point to the same fd set.
		 */
		if(msgi.flags & _NOTIFY_COND_INPUT) {
			FD_CLR(fd, readfds);
		}

		if(msgi.flags & _NOTIFY_COND_OUTPUT) {
			FD_CLR(fd, writefds);
		}

		if(msgi.flags) {
			struct _io_notify_reply		msgo;

			if(MsgSend(fd, &msgi, sizeof msgi, &msgo, sizeof msgo) == -1) {
				if(errno != ENOSYS) {
					if(readfds) {
						memcpy(readfds, saved_rd, setsize);
					}
					if(writefds) {
						memcpy(writefds, saved_wr, setsize);
					}
					if(exceptfds) {
						memcpy(exceptfds, saved_ex, setsize);
					}
					return -1;
				}
				msgo.flags = msgi.flags & (_NOTIFY_COND_INPUT | _NOTIFY_COND_OUTPUT);
			}

			if(msgo.flags & _NOTIFY_COND_MASK) {
				if(msgo.flags & _NOTIFY_COND_INPUT) {
					FD_SET(fd, readfds);
					pollsuccess++;
				}
				if(msgo.flags & _NOTIFY_COND_OUTPUT) {
					FD_SET(fd, writefds);
					pollsuccess++;
				}
				if(msgo.flags & _NOTIFY_COND_OBAND) {
					FD_SET(fd, exceptfds);
					pollsuccess++;
				}

				/* Don't arm the other fds */
				msgi.action = _NOTIFY_ACTION_POLL;
			}
		}
	}
	
	/* we got 1 or more poll success' */
	if(pollsuccess) {
		return pollsuccess;
	}
	
	/*
	 * the timeout is handled in a very hokey way.  Its should be applied
	 * to the whole routine, but its only applied to the blocking receive
	 * below.  If the send's above took a measureable amount of
	 * time, then the timeout could be violated by a significant amount.
	 * receiving signals with old serial nums also causes a short timeout 
	 */
	zero.tv_sec = zero.tv_nsec = 0;
	while((status = eventwait(ts, &value, arg)) != -1) {
		fd = ((value.sival_int & _NOTIFY_DATA_MASK) - sernum) & _NOTIFY_DATA_MASK;
		if(fd >= 0 && fd < nfds) {
			if(value.sival_int & _NOTIFY_COND_INPUT) {
				FD_SET(fd, readfds);
				pollsuccess++;
			}
			
			if(value.sival_int & _NOTIFY_COND_OUTPUT) {
				FD_SET(fd, writefds);
				pollsuccess++;
			}
			
			if(value.sival_int & _NOTIFY_COND_OBAND) {
				FD_SET(fd, exceptfds);
				pollsuccess++;
			}
		} else {
			continue; /* absorb old signals -- timeout gets reset :( */
		}
		ts = &zero; /* spin hard absorbing up any queued signals */
	}

	if(status == -1 && errno != ETIMEDOUT && errno != EAGAIN) {
		if(readfds) {
			memcpy(readfds, saved_rd, setsize);
		}
		if(writefds) {
			memcpy(writefds, saved_wr, setsize);
		}
		if(exceptfds) {
			memcpy(exceptfds, saved_ex, setsize);
		}
		return -1;
	}

	return pollsuccess;
}

__SRCVERSION("select_event.c $Rev: 153052 $");
