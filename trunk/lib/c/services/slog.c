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




#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/iomsg.h>
#include <sys/sysmsg.h>
#include <sys/neutrino.h>
#include <sys/slog.h>


//
// This is almost magic. Every process has a connection open to
// ProcNto which "just exists". It does not need to be opened.
// It is used for a variety of special purposes. If you send
// a xxx message to Proc it will attempt to open "/dev/slog"
// and send that message on as a simple _IO_WRITE. In effect
// it acts as a relay. Why is this good? It means every server
// which uses slogger (and most will) need not keep a separate fd
// open to the slogger manager. It also allows you to stop and
// start a new slogger manager and all clients will be redirected
// to the new one automatically.
// If an app wants to go direct it can do a
//
//   _slogfd = open("/dev/slog", O_WRONLY);
//
// which will override.
//
// NOTE: Early versions of Proc did not support this so we fall
// back to opening the device ourselves on a failure.
//

extern	pthread_mutex_t	__slog_mux;

//
// This function sends the message. It assumes that iov[0] is free
// for its own use and that nparts starts at iov[1]. It saves copying
// args about.
//
static ssize_t slogsend(iov_t iov[], int nparts, int nbytes)
{
	// These two structures ARE PERFECTLY MATCHED!
	union {
		io_write_t	wr;
		sys_log_t	log;
	} 						msg;
	ssize_t					len;
	int						tried_proc, ret;

	msg.wr.i.combine_len = sizeof msg.wr.i;
	msg.wr.i.xtype = _IO_XTYPE_NONE;
	msg.wr.i.nbytes = nbytes;
	msg.wr.i.zero = 0;
	SETIOV(iov + 0, &msg.wr.i, sizeof msg.wr.i);

	tried_proc = 0;
	if((ret = pthread_mutex_lock(&__slog_mux)) != EOK) {
		errno = ret;
		return -1;
	}
	for( ;; ) { 
		msg.wr.i.type = (_slogfd == SYSMGR_COID) ? _SYS_LOG : _IO_WRITE;
		len = MsgSendv(_slogfd, iov, nparts + 1, 0, 0);
		if(len != -1) break;
		if((errno != ENOSYS) && (errno != EBADF)) break;
		//Request not supported or the server went away	
		if(_slogfd != SYSMGR_COID) {
			close(_slogfd);
			_slogfd = SYSMGR_COID; // Try sending to procnto again
			if(tried_proc) break;
			tried_proc = 1;
		} else {
			_slogfd = open("/dev/slog", O_WRONLY);
			if(_slogfd == -1) break;
		}
	}
	pthread_mutex_unlock(&__slog_mux);

	return(len);
}

ssize_t slogb(int opcode, int severity, void *data, int size)
{
	iov_t		iov[3];
	int			buf[_SLOG_HDRINTS];

	buf[0] = severity;
	buf[1] = opcode;
	SETIOV(iov + 1, &buf[0], sizeof buf);
	SETIOV(iov + 2, data, size);

	return(slogsend(iov, 2, GETIOVLEN(iov + 1) + GETIOVLEN(iov + 2)));
}

ssize_t slogi(int opcode, int severity, int nargs, ...)
{
	iov_t		iov[3];
	int			buf[_SLOG_HDRINTS];
	int			i;
	int			data[32];
	va_list		arglist;

	// Limit to the size of the above buffer 
	if(nargs >  (sizeof(data)/sizeof(*data))) {
		errno = EINVAL;
		return -1;
	}

	// Collect the args
	va_start(arglist, nargs);
	for(i = 0 ; i < nargs ; ++i) {
		data[i] = va_arg(arglist, int);
	}
	va_end(arglist);

	buf[0] = severity;
	buf[1] = opcode;
	SETIOV(iov + 1, &buf[0], sizeof buf);
	SETIOV(iov + 2, data, nargs * sizeof(int));

	return(slogsend(iov, 2, GETIOVLEN(iov + 1) + GETIOVLEN(iov + 2)));
}

ssize_t vslogf(int opcode, int severity, const char *fmt, va_list arglist)
{
	iov_t			iov[2];
	int			*buf;
	int			n, avail, tot;
	char			*start;
	va_list			va_new;


	/*
	 * If we're low on stack, try to get just what we need.
	 * This means an extra vsnprintf.
	 */
	if(__stackavail() < (_SLOG_MAXSIZE + 1024)) {
		va_copy(va_new, arglist);
		avail = vsnprintf(NULL, 0, fmt, va_new);
		va_end(va_new);

		if(avail == -1)
			return -1;

		avail++; /* For '\0' */
		avail = min(avail, _SLOG_MAXSIZE - _SLOG_HDRINTS * sizeof(int));
		tot = avail + _SLOG_HDRINTS * sizeof(int);

	}
	else {
		tot = _SLOG_MAXSIZE;
		avail = tot - _SLOG_HDRINTS * sizeof(int);
	}

	if((buf = alloca(tot)) == NULL) {
		errno = ENOMEM;
		return -1;
	}

	start = (char *)(&buf[_SLOG_HDRINTS]);

	if((n = vsnprintf(start, avail, fmt, arglist)) == -1)
		return -1;

	/* Make sure terminating '\0' is included */
	n = min(n + 1, avail);

	/*
	 * If the user sets _SLOG_TEXTBIT themselves, they are indicating that
	 * they want to output to go to stderr as well.
	 */
	if(severity & _SLOG_TEXTBIT) {
		start[n - 1] = '\n';   /* overwrite '\0' */
		write(STDERR_FILENO, start, n);
		start[n - 1] = '\0';   /* restore */
	}

	buf[0] = severity | _SLOG_TEXTBIT;
	buf[1] = opcode;
	SETIOV(iov + 1, buf, n + _SLOG_HDRINTS * sizeof(int));

	return(slogsend(iov, 1, GETIOVLEN(iov + 1)));
}

ssize_t slogf(int opcode, int severity, const char *fmt, ...)
{
	ssize_t		ret;
	va_list		arglist;

	va_start(arglist, fmt);
	ret = vslogf(opcode, severity, fmt, arglist);
	va_end(arglist);

	return(ret);
}

__SRCVERSION("slog.c $Rev: 170432 $");
