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

#include <fcntl.h>
#include <share.h>
#include "externs.h"
#include "procmgr_internal.h"

int
procmgr_coredump(message_context_t *ctp, int code, unsigned flags, void *handle) {
	union sigval				value = ctp->msg->pulse.value;
	int							dumped = 0;
	int							fd;
	PROCESS						*prp;

  	if (procmgr_scoid != ctp->msg->pulse.scoid) {
		return 0;
  	}

	if(proc_thread_pool_reserve() == 0) {
		/*
		 * Try to cause dumper to write a core file
		 */
		if((fd = _connect(0, "/proc/dumper", 0, O_WRONLY, SH_DENYNO, _IO_CONNECT_OPEN,
				0, _IO_FLAG_RD | _IO_FLAG_WR, _FTYPE_DUMPER, 0, 0, 0, 0, 0, 0)) != -1) {
			char	buff[2*(INT_STRLEN_MAXIMUM(int) + 1)];
			char	*p;

			ultoa(value.sival_int, buff, 10);
			p = &buff[strlen(buff)];
			if((prp = proc_lock_pid(value.sival_int))) {
				if(prp->rlimit_vals_soft[RLIMIT_CORE] != RLIM_INFINITY) {
					// set a maximum core size.
					*p++ = ' ';
					ultoa(prp->rlimit_vals_soft[RLIMIT_CORE], p, 10);
					p += strlen(p);
				}
				proc_unlock(prp);
			}
			if(write(fd, buff, p - buff) != -1) {
				/*
				 * A Dump file are written....
				 */
				dumped = 1;
			}
			close(fd);
		}

		proc_thread_pool_reserve_done();
	}

	/*
	 * Only leave the _NTO_PF_COREDUMP flag on if a dump file was
	 * really created. The termer thread looks at this flag to report
	 * if a coredump file was created.
	 */
	if(!dumped) {
		if((prp = proc_lock_pid(value.sival_int))) {
			prp->flags &= ~_NTO_PF_COREDUMP;
			proc_unlock(prp);
		}
	}

	/*
	 * Let the process terminate normally...
	 */
	(void)ProcessDestroyAll(value.sival_int);

	return 0;
}

__SRCVERSION("procmgr_coredump.c $Rev: 157791 $");
