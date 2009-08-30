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
 * nano_xfer_check.c
 * General function to check an iov range to make sure it is inside the process boundary.
 */
/*************************************************************************
 *************************************************************************
 **
 ** This file may be overridden in a CPU specific directory with code
 ** optimized for the particular machine.
 **
 *************************************************************************
 *************************************************************************/

#include "externs.h"
#include <unistd.h>

/*
 * Return from transfer with a error code stating the
 * side that had the fault (src or dest).
 *
 * Note: the fault handlers defined here cover this module, nano_xfer.c,
 * 	nano_xfer_msg.c and nano_xfer_cpy.c.
 *  If those modules are not optimized together in a specific platform,
 *  special care need to be taken on fault handlers.
 */

static void xfer_dst_fault(THREAD *thp, CPU_REGISTERS *regs, unsigned flags) {
	xfer_longjmp(*xfer_env, FAULT_ISWRITE(flags) ? XFER_SRC_FAULT : XFER_DST_FAULT, regs);
}

static void xfer_src_fault(THREAD *thp, CPU_REGISTERS *regs, unsigned flags) {
	xfer_longjmp(*xfer_env, FAULT_ISWRITE(flags) ? XFER_DST_FAULT : XFER_SRC_FAULT, regs);
}

const struct fault_handlers xfer_dst_handlers = {
	xfer_dst_fault, xfer_restart
};

const struct fault_handlers xfer_src_handlers = {
	xfer_src_fault, xfer_restart
};

const struct fault_handlers xfer_async_handlers = {
        xfer_src_fault, xfer_async_restart
};

int	(xfer_memchk)(uintptr_t bound, const IOV *iov, size_t iov_len) {
	unsigned	base, last, len, status;
	jmp_buf		env;

	xfer_env = &env;
	if((status = xfer_setjmp(*xfer_env))) {
		return status;
	}

	while(iov_len) {
		base = (uintptr_t)GETIOVBASE(iov);
		len	 = GETIOVLEN(iov++);
		last = base + len - 1;
		if(base > last || !WITHIN_BOUNDRY(base, last, bound)) {
			if(len != 0) {
				return -1;
			}
		}
		iov_len--;
	}

	return 0;
}

int	(xfer_memprobe)(void *ptr) {
	int			status;
	jmp_buf		env;

	xfer_env = &env;
	if((status = xfer_setjmp(*xfer_env))) {
		return status;
	}

	RD_PROBE_INT(NULL, ptr, 1);

	return 0;
}


__SRCVERSION("nano_xfer_check.c $Rev: 153052 $");
