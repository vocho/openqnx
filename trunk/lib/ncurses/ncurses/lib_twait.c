/****************************************************************************
 * Copyright (c) 1998 Free Software Foundation, Inc.                        *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 ****************************************************************************/

/*
**	lib_twait.c
**
**	The routine _nc_timed_wait().
**
**	(This file was originally written by Eric Raymond; however except for
**	comments, none of the original code remains - T.Dickey).
*/

#include <curses.priv.h>

#if	defined(__QNX__) && !defined(__QNXNTO__)

#include <sys/dev.h>
#include <time.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/proxy.h>
#include <sys/time.h>

#else

#if USE_FUNC_POLL
# include <stropts.h>
# include <poll.h>
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# endif
#elif HAVE_SELECT
# if HAVE_SYS_TIME_H && HAVE_SYS_TIME_SELECT
#  include <sys/time.h>
# endif
# if HAVE_SYS_SELECT_H
#  include <sys/select.h>
# endif
#endif

#endif	/* __QNX__ */

#ifdef __BEOS__
/* BeOS select() only works on sockets.  Use the tty hack instead */
#include <socket.h>
#define select check_select
#endif

MODULE_ID("$Id: lib_twait.c 153052 2008-08-13 01:17:50Z coreos $")

/*
 * We want to define GOOD_SELECT if the last argument of select(2) is
 * modified to indicate time left.  The code will deal gracefully with
 * the other case, this is just an optimization to reduce the number
 * of system calls per input event.
 *
 * In general, expect System-V-like UNIXes to have this behavior and BSD-like
 * ones to not have it.  Check your manual page.  If it doesn't explicitly
 * say the last argument is modified, assume it's not.
 *
 * (We'd really like configure to autodetect this, but writing a proper test
 * turns out to be hard.)
 */

#if HAVE_GETTIMEOFDAY
#if (defined(TRACE) && !HAVE_USLEEP) || ! GOOD_SELECT
static void _nc_gettime(struct timeval *tp)
{
	gettimeofday(tp, (struct timezone *)0);
	T(("time: %ld.%06ld", (long) tp->tv_sec, (long) tp->tv_usec));
}
#endif
#endif

/*
 * Wait a specified number of milliseconds, returning nonzero if the timer
 * didn't expire before there is activity on the specified file descriptors.
 * The file-descriptors are specified by the mode:
 *	0 - none (absolute time)
 *	1 - ncurses' normal input-descriptor
 *	2 - mouse descriptor, if any
 *	3 - either input or mouse.
 * We return a mask that corresponds to the mode (e.g., 2 for mouse activity).
 *
 * If the milliseconds given are -1, the wait blocks until activity on the
 * descriptors.
 */
 
#if	defined(__QNX__) && !defined(__QNXNTO__)
/* 
 * Using QNX proxy and timer instead of select/polling 
 */
int _nc_timed_wait(int mode, int milliseconds, int *timeleft)
{
int		count = 0;
long	whole_secs = milliseconds / 1000;
long	micro_secs = (milliseconds % 1000) * 1000;

int		result = 0;
struct 	timeval ntimeout;
struct	itimerspec value, ovalue;
pid_t	pid;

#ifdef __GNUC__
timer_t	timer_id;
#endif

	if (milliseconds >= 0) {
		ntimeout.tv_sec  = whole_secs;
		ntimeout.tv_usec = micro_secs;
	} else {
		ntimeout.tv_sec  = 0;
		ntimeout.tv_usec = 0;
	}

	T(("start twait: %lu.%06lu secs, mode: %d", (long) ntimeout.tv_sec, (long) ntimeout.tv_usec, mode));

#ifdef HIDE_EINTR
	/*
	 * The do loop tries to make it look like we have restarting signals,
	 * even if we don't.
	 */
	do {
#endif /* HIDE_EINTR */
		count = 0;
		
		/* QNX Mouse is not selectable, we have to use dev_arm to
		   instead of select */

		/* Take care of mouse first */
		if ((mode & 2) && SP->_qnx_mouse_ctrl && !SP->_qnx_mouse_armed)
			result |= 2;
			
		if (result != 0) {
			if (timeleft)
				*timeleft = (ntimeout.tv_sec * 1000) + (ntimeout.tv_usec / 1000);
			return result;
		}


		/* Arm keyboard if necessary */
		if ((mode & 1)) {
			if  (SP->_qnx_kbd_proxy == -1 &&
				(SP->_qnx_kbd_proxy = qnx_proxy_attach(0, 0, 0, -1)) == -1)
				return -1;

			if (SP->_qnx_kbd_vproxy == -1) 
			{
				struct _dev_info_entry die;

				if (dev_info(SP->_ifd, &die) == -1)
					return -1;
				SP->_qnx_kbd_vproxy =  qnx_proxy_rem_attach(die.nid, SP->_qnx_kbd_proxy);
				if (SP->_qnx_kbd_vproxy == -1)
					return -1;
			}
			dev_arm(SP->_ifd, SP->_qnx_kbd_vproxy, _DEV_EVENT_INPUT);
		}			

		/* Start the timer if we have timeout */
		if (milliseconds > 0) {
			if (SP->_qnx_timer_proxy == -1 && 
				(SP->_qnx_timer_proxy = qnx_proxy_attach(0, 0, 0, -1)) == -1)
				return -1;

			if (SP->_qnx_timer == -1)
			{		
				struct sigevent	ev;

				ev.sigev_signo = -1 * SP->_qnx_timer_proxy;
				#ifdef __GNUC__
				if ((SP->_qnx_timer = timer_create(CLOCK_REALTIME, &ev, &timer_id)) == -1)
				#else
				if ((SP->_qnx_timer = timer_create(CLOCK_REALTIME, &ev)) == -1)
				#endif
				{
					qnx_proxy_detach(SP->_qnx_timer_proxy);
					return -1;
				}
			}
			while (Creceive(SP->_qnx_timer_proxy, 0, 0) != -1);
			value.it_value.tv_sec = ntimeout.tv_sec;
			value.it_value.tv_nsec = ntimeout.tv_usec * 1000;
			value.it_interval.tv_sec = 0 ;
			value.it_interval.tv_nsec = 0 ;
			timer_settime(SP->_qnx_timer, 0, &value, NULL);
		}
		result = 0;
		if (milliseconds == 0) {
			/* always disarm input so there won't be a window which one key press
			 * cause 2 proxy trigger 
			 */
			dev_arm(SP->_ifd, _DEV_DISARM, _DEV_EVENT_INPUT);
			pid = Creceive(0, 0, 0);
		} else {
			pid = Receive(0, 0, 0);
			dev_arm(SP->_ifd, _DEV_DISARM, _DEV_EVENT_INPUT);
		}
		if (pid == SP->_qnx_timer_proxy) {
			ntimeout.tv_sec = ntimeout.tv_usec = 0;
			result = 0;
		} else {
			if (milliseconds > 0) {
				timer_gettime(SP->_qnx_timer, &ovalue);
				ntimeout.tv_sec = ovalue.it_value.tv_sec;
				ntimeout.tv_usec = ovalue.it_value.tv_nsec / 1000;
			}
			if (pid != -1)	/* might be an EINTR for window size change */
			{ 
			 	if (pid == SP->_qnx_kbd_proxy)
				{
					if (mode & 1)
						result |= 1;
				} else if (pid == SP->_qnx_mouse_proxy) {
					SP->_qnx_mouse_armed = 0;
					if (mode & 2)
						result |= 2;
				} else if (SP->_qnx_user_callback)
					SP->_qnx_user_callback(pid);
			}
		}
#ifdef HIDE_EINTR
	} while (pid == -1 && errno == EINTR);
#endif

	/* return approximate time left on the ntimeout, in milliseconds */
	if (timeleft)
		*timeleft = (ntimeout.tv_sec * 1000) + (ntimeout.tv_usec / 1000);

	T(("end twait: returned %d (%d), remaining time %lu.%06lu secs (%d msec)",
		result, errno,
		(long) ntimeout.tv_sec, (long) (ntimeout.tv_usec / 1000),
		timeleft ? *timeleft : -1));

	return (result);
}

/* This QNX-Extended function on/off the user_callback function */
void set_qnx_callback(void (* user_callback)(pid_t pid), bool on)
{
	if (on) {
		if (user_callback)
			SP->_qnx_user_callback = user_callback;
	} else {
		SP->_qnx_user_callback = NULL;
	}
}


#else	/* __QNX__ */ 
 
int _nc_timed_wait(int mode, int milliseconds, int *timeleft)
{
int	fd;
int	count = 0;
long	whole_secs = milliseconds / 1000;
long	micro_secs = (milliseconds % 1000) * 1000;

int result = 0;
struct timeval ntimeout;

#if USE_FUNC_POLL
struct pollfd fds[2];
#elif HAVE_SELECT
static fd_set set;
#endif

#if !GOOD_SELECT && HAVE_GETTIMEOFDAY
struct timeval starttime, returntime;
long delta;

	_nc_gettime(&starttime);
#endif

	if (milliseconds >= 0) {
		ntimeout.tv_sec  = whole_secs;
		ntimeout.tv_usec = micro_secs;
	} else {
		ntimeout.tv_sec  = 0;
		ntimeout.tv_usec = 0;
	}

	T(("start twait: %lu.%06lu secs, mode: %d", (long) ntimeout.tv_sec, (long) ntimeout.tv_usec, mode));

#ifdef HIDE_EINTR
	/*
	 * The do loop tries to make it look like we have restarting signals,
	 * even if we don't.
	 */
	do {
#endif /* HIDE_EINTR */
#if !GOOD_SELECT && HAVE_GETTIMEOFDAY
	retry:
#endif
		count = 0;
#if USE_FUNC_POLL

		if (mode & 1) {
			fds[count].fd     = SP->_ifd;
			fds[count].events = POLLIN;
			count++;
		}
		if ((mode & 2)
		 && (fd = SP->_mouse_fd) >= 0) {
			fds[count].fd     = fd;
			fds[count].events = POLLIN;
			count++;
		}
		result = poll(fds, count, milliseconds);
#elif HAVE_SELECT
		/*
		 * Some systems modify the fd_set arguments; do this in the
		 * loop.
		 */
		FD_ZERO(&set);

		if (mode & 1) {
			FD_SET(SP->_ifd, &set);
			count = SP->_ifd + 1;
		}
		if ((mode & 2)
		 && (fd = SP->_mouse_fd) >= 0) {
			FD_SET(fd, &set);
			count = max(fd, count) + 1;
		}

		errno = 0;
		result = select(count, &set, NULL, NULL, milliseconds >= 0 ? &ntimeout : 0);
#endif

#if !GOOD_SELECT && HAVE_GETTIMEOFDAY
		_nc_gettime(&returntime);

		/* The contents of ntimeout aren't guaranteed after return from
		 * 'select()', so we disregard its contents.  Also, note that
		 * on some systems, tv_sec and tv_usec are unsigned.
		 */
		ntimeout.tv_sec  = whole_secs;
		ntimeout.tv_usec = micro_secs;

#define DELTA(f) (long)ntimeout.f - (long)returntime.f + (long)starttime.f

		delta = DELTA(tv_sec);
		if (delta < 0)
			delta = 0;
		ntimeout.tv_sec = delta;

		delta = DELTA(tv_usec);
		while (delta < 0 && ntimeout.tv_sec != 0) {
			ntimeout.tv_sec--;
			delta += 1000000;
		}
		ntimeout.tv_usec = delta;
		if (delta < 0)
			ntimeout.tv_sec = ntimeout.tv_usec = 0;

		/*
		 * If the timeout hasn't expired, and we've gotten no data,
		 * this is probably a system where 'select()' needs to be left
		 * alone so that it can complete.  Make this process sleep,
		 * then come back for more.
		 */
		if (result == 0
		 && (ntimeout.tv_sec != 0 || ntimeout.tv_usec > 100000)) {
			napms(100);
			goto retry;
		}
#endif
#ifdef HIDE_EINTR
	} while (result == -1 && errno == EINTR);
#endif

	/* return approximate time left on the ntimeout, in milliseconds */
	if (timeleft)
		*timeleft = (ntimeout.tv_sec * 1000) + (ntimeout.tv_usec / 1000);

	T(("end twait: returned %d (%d), remaining time %lu.%06lu secs (%d msec)",
		result, errno,
		(long) ntimeout.tv_sec, (long) (ntimeout.tv_usec / 1000),
		timeleft ? *timeleft : -1));

	/*
	 * Both 'poll()' and 'select()' return the number of file descriptors
	 * that are active.  Translate this back to the mask that denotes which
	 * file-descriptors, so that we don't need all of this system-specific
	 * code everywhere.
	 */
	if (result != 0) {
		if (result > 0) {
			result = 0;
#if USE_FUNC_POLL
			for (count = 0; count < 2; count++) {
				if ((mode & (1 << count))
				 && (fds[count].revents & POLLIN)) {
					result |= (1 << count);
					count++;
				}
			}
#elif HAVE_SELECT
			if ((mode & 2)
			 && (fd = SP->_mouse_fd) >= 0
			 && FD_ISSET(fd, &set))
				result |= 2;
			if ((mode & 1)
			 && FD_ISSET(SP->_ifd, &set))
				result |= 1;
#endif
		}
		else
			result = 0;
	}

	return (result);
}

#endif

