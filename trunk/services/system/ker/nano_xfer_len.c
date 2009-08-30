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

/*************************************************************************
 *************************************************************************
 **
 ** This file may be overridden in a CPU specific directory with code
 ** optimized for the particular machine.
 **
 *************************************************************************
 *************************************************************************/

#include <unistd.h>
#include <setjmp.h>
#include "externs.h"

/* use xfer_fault_handlers_xferlen, in case that xfer_fault is for no setjmp version */
static void xfer_fault(THREAD *thp, CPU_REGISTERS *regs, unsigned flags) {
	xfer_longjmp(*xfer_env, -1, regs);
}

static const struct fault_handlers xfer_fault_handlers_xferlen = {
	xfer_fault, 0
};

int xferlen(THREAD *thp, IOV *iov, int parts) {
	int				len;
	int				status;
	jmp_buf			env;
	
	if(parts < 0) {
		return(-parts);
	}

#ifndef NDEBUG
	/* Make sure iov's address space is accessable */
	if(thp->aspace_prp && thp->aspace_prp != aspaces_prp[KERNCPU]) {
		crash();
	}
#endif

	xfer_env = &env;
	if((status = xfer_setjmp(*xfer_env))) {
		return(status);
	}

	SET_XFER_HANDLER(&xfer_fault_handlers_xferlen);

	len = 0;
	while(parts > 0) {
		len += GETIOVLEN(iov);
		++iov;
		--parts;
	}

	SET_XFER_HANDLER(0);

	return(len);
}


__SRCVERSION("nano_xfer_len.c $Rev: 153052 $");
