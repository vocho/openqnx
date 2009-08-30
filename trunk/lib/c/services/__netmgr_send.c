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
#include <fcntl.h>
#include <errno.h>
#include <share.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <atomic.h>
#include "netmgr_send.h"

int _netmgr_connect(int base, const char *path, mode_t mode, unsigned oflag, unsigned sflag, unsigned subtype, 
			 int testcancel, unsigned access, unsigned file_type, unsigned extra_type, unsigned extra_len, 
			 const void *extra, unsigned response_len, void *response, int *status);

#define NM_COID_NONE	(-1)

static int				netmgr_coid = NM_COID_NONE;
static volatile unsigned	netmgr_private;

int
__netmgr_send_private(int coid) {

	if(coid == -1) {
		atomic_add(&netmgr_private, 1);
	} else {
		atomic_sub(&netmgr_private, 1);
	}
	return 0;
}

int 
__netmgr_send(void *smsg1, int ssize1, const void *smsg2, int ssize2, 
				void *rmsg, int rsize) {
	int					coid;
	int					status;
	iov_t				iov[2];
	int					parts = 1;
	unsigned			private;

	// Check if there is two parts
	SETIOV(iov + 0, smsg1, ssize1);
	if(smsg2 && ssize2) {
		SETIOV(iov + 1, smsg2, ssize2);
		parts++;
	}

	// If we have never talked to the netmgr, or the netmgr connection is bad,
	// try to find it...
	// The ENOSYS check is in case qnet gets shutdown and the netmgr_coid
	// value is reused by some other open before we get called again.
	status = -1;
	private = netmgr_private;
	coid = netmgr_coid;
	if(  (private != 0) 
	  || ((coid < 0) 
	  || ((status = MsgSendvs(coid, iov, parts, rmsg, rsize)) == -1
	  && ((errno == EBADF) || (errno == ENOSYS))))) {
		// Forget the previous connection to the netmgr
		if(coid >= 0) {
			netmgr_coid = NM_COID_NONE;
			close(coid);
		}

		// This is the same as open(), but is not a cancellation point
		coid = _netmgr_connect(_NTO_SIDE_CHANNEL, "/dev/netmgr", 0, O_RDONLY, 
					SH_DENYNO, _IO_CONNECT_OPEN, 0, _IO_FLAG_RD | _IO_FLAG_WR,
					0, 0, 0, 0, 0, 0, 0);
		if(coid == -1) {
			errno = ENOTSUP;
		} else {
			// Send the message to the newly found netmgr
			status = MsgSendvs(coid, iov, parts, rmsg, rsize);
			if(status == -1) {
				// If there was any error, don't believe it is a netmgr...
				close(coid);
			} else if(private != 0) {
				// We don't want to try and re-use the coid when we're private
				close(coid);
			} else {
				netmgr_coid = coid;
			}
		}
	}
	return status;
}

__SRCVERSION("__netmgr_send.c $Rev: 206031 $");
