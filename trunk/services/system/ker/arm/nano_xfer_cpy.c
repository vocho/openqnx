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
 * ARM specific copy routines
 */
#include <unistd.h>
#include <setjmp.h>
#include "externs.h"

extern jmp_buf					*xfer_env;

#ifndef	VARIANT_v6
/*
 * Test whether the area we are about to copy a short message to is read-only.
 *
 * FIXME: this only checks the first byte - we ought to check if the buffer
 *		  crosses a page boundary.
 */
static inline void
xfer_probe(void *dst, void *src, unsigned len)
{
	if (len) {
		unsigned char	new = *(unsigned char*)src;
		__asm__ __volatile__(
			"strbt	%0, [%1]"
			:
			: "r"(new), "r" (dst)
		);
	}
}
#endif

/*
 * This routine is used to do memcpy between a kernel buffer and a user buffer
 * during short msg transfer. It will guranteed to return an error msg when the user
 * buffer can not be accessed.
 *
 * FIXME: this doesn't guarantee a write fault if dst is read-only.
 *		  However, this is currently only used to copy a short reply message
 *		  into the sender's args.msbuff.buff.
 *		  Since this is a kernel struct we should not get a write fault.
 */
int xfer_memcpy(void *dst, const void *src, size_t len) {
	void *old_handler;
	
	if(!(old_handler = (void*) GET_XFER_HANDLER())) {
		jmp_buf					env;
		int						status;

		/* Setup the jump buffer for all exiting of this function */
		xfer_env = &env;
		if((status=xfer_setjmp(*xfer_env))) {
			return status;
		}
		SET_XFER_HANDLER(&xfer_src_handlers);
	}
	memcpy(dst, src, len);
	SET_XFER_HANDLER(old_handler);
	return 0;
}

/*
 * This routine is used to do memcpy from a kernel buffer to a user buffer or iov set
 * during short msg transfer. It will guranteed to return an error msg when the user
 * buffer or iov set can not be accessed.
 *
 */
int xfer_cpy_diov(THREAD* thpd, IOV *dst, uint8_t *saddr, int dparts, unsigned slen) {
	jmp_buf				env;
	int					status;
	char				*daddr;
	unsigned			dlen;
#ifndef	VARIANT_v6
	int					user = (thpd->process->boundry_addr != VM_KERN_SPACE_BOUNDRY);
#endif


	/* Setup the jump buffer for all exiting of this function */
	xfer_env = &env;
	if((status=xfer_setjmp(*xfer_env))) {
		return status;
	}

	if(dparts <= 0) {
		if(dparts == 0) {
			return 0;
		}
		if(!WR_PROBE_PTR(thpd, dst, -dparts)) {
			return XFER_DST_FAULT;
		}
		SET_XFER_HANDLER(&xfer_src_handlers);
#ifndef	VARIANT_v6
		if (user)
			xfer_probe(dst, saddr, min(slen, -dparts));
#endif
		memcpy(dst, saddr, min(slen,-dparts));
		SET_XFER_HANDLER(0);
		return 0;
	}

	SET_XFER_HANDLER(&xfer_dst_handlers);
	daddr = (char *)GETIOVBASE(dst);
	dlen = GETIOVLEN(dst);
	if(!WR_PROBE_PTR(thpd, daddr, dlen)) {
		return XFER_DST_FAULT;
	}

	SET_XFER_HANDLER(&xfer_src_handlers);

	/* Now we move the data. */
	if(slen <= dlen) {
#ifndef	VARIANT_v6
		if (user)
			xfer_probe(daddr, saddr, slen);
#endif
		memcpy(daddr, saddr, slen);
		SET_XFER_HANDLER(0);
		return 0;
	}

	// rare case
	while(1) {	
#ifndef	VARIANT_v6
		if (user)
			xfer_probe(daddr, saddr, dlen);
#endif
		memcpy(daddr, saddr, dlen);
		if(--dparts == 0) {
			break;
		}
		saddr += dlen;
		slen -= dlen;
		++dst;
		SET_XFER_HANDLER(&xfer_dst_handlers);
		daddr = (char *)GETIOVBASE(dst);
		dlen  = GETIOVLEN(dst);
		if(!WR_PROBE_PTR(thpd, daddr, dlen)) {
			return XFER_DST_FAULT;
		}
		SET_XFER_HANDLER(&xfer_src_handlers);
		if(slen <= dlen) {
#ifndef	VARIANT_v6
			if (user)
				xfer_probe(daddr, saddr, slen);
#endif
			memcpy(daddr, saddr, slen);
			break;
		}
	}
	SET_XFER_HANDLER(0);
	return 0;
}

__SRCVERSION("nano_xfer_cpy.c $Rev: 163928 $");
