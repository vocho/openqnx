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
 * ARM specific xferpulse() that catches faults to unwritable user pages
 */

#include <unistd.h>
#include <setjmp.h>
#include "externs.h"

int
xferpulse(THREAD *dthp, IOV *dst, int parts, uint32_t code, uint32_t value, int32_t scoid)
{
	struct _pulse	*addr;
	uint32_t		len;
	uintptr_t		last;
	int				status;
	jmp_buf			env;

#ifndef NDEBUG
	/* Make sure iov's address space is accessable */
	if(dthp->aspace_prp && dthp->aspace_prp != aspaces_prp[KERNCPU]) {
		crash();
	}
#endif

	xfer_env = &env;
	if((status = xfer_setjmp(*xfer_env))) {
		return(status);
	}

	SET_XFER_HANDLER(&xfer_fault_handlers);

	/* Special case for 1 part messages */
	if(parts < 0) {
		addr = (struct _pulse *)dst;
		len = -parts;
	} else {
		addr = (struct _pulse *)GETIOVBASE(dst);
		len = GETIOVLEN(dst);
	}

	/* Make dest will hold a pulse */
	if(len < sizeof(*addr)) {
		SET_XFER_HANDLER(0);
		return(XFER_DST_FAULT);
	}

	/* Make sure address is within range for process */
	last = (uintptr_t)addr + len - 1;
	if((uintptr_t)addr > last || !WITHIN_BOUNDRY((uintptr_t)addr, last, dthp->process->boundry_addr)) {
		SET_XFER_HANDLER(0);
		return(XFER_DST_FAULT);
	}

	if (dthp->process->boundry_addr != VM_KERN_SPACE_BOUNDRY) {
		/*
		 * Trigger fault if page is not writable
		 */
		__asm__ __volatile__(
			"strbt	%0, [%1]"
			:
			: "r" (0), "r" (addr)
		);
	}

	/* Write pulse */
	addr->type = _PULSE_TYPE;
	addr->subtype = _PULSE_SUBTYPE;
	addr->code = code;
	addr->value.sival_int = value;
	addr->scoid = scoid;

	SET_XFER_HANDLER(0);
	return(0);
}

__SRCVERSION("nano_xfer_pulse.c $Rev: 153052 $");
