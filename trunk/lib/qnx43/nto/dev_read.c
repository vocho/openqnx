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



#include <unistd.h>
#include <signal.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/neutrino.h>

extern int __trigger( pid_t __pid );
extern void *_scalloc(unsigned size);
extern void _sfree(void *data, unsigned size); 

struct __drparms {
	int		fd;
	void	*buf;
	int		min;
	int		time;
	int		timeout;
	int		proxy;
	int		tid;
	} ;


void __dev_read(struct __drparms *drp) {

	readcond(drp->fd, drp->buf, 0, drp->min, drp->time, drp->timeout);
	if(drp->proxy & (~0u ^ (~0u >> 1)))	// Top bit set?
		SignalKill(0, 0, drp->tid, ~drp->proxy, SI_USER, 0);
	else
		__trigger(drp->proxy);

	_sfree(drp, sizeof(*drp));
	ThreadDestroy(0, 0, 0);
	}


int dev_read(int fd, void *buf, unsigned n, unsigned min, unsigned time,
				unsigned timeout, pid_t proxy, int *armed)
	{
	int status;
	struct __drparms *drp;

	// Handle simple synchronous case.
	if(proxy == 0)
		return(readcond(fd, buf, n, min, time, timeout));

	// Handle asynchronous case.
	if((n = readcond(fd, buf, n, min, time, -1)) > 0) {
		if(armed) *armed = 0;
		return(n);
		}

	// Data is not available now. Create a thread to wait for it.
	if(armed) *armed = 1;
	drp = (struct __drparms *) _scalloc(sizeof(*drp));
	drp->fd = fd;
	drp->buf = buf;
	drp->min = min;
	drp->time = time;
	drp->timeout = timeout;
	drp->proxy = proxy;
	drp->tid = gettid();
	status = ThreadCreate(0, (void *)__dev_read, drp, 0);

	return(status == -1 ? -1 : 0);
	}
