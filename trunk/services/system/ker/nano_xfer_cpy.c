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
#include <setjmp.h>
#include "externs.h"

extern jmp_buf					*xfer_env;

/*
 * This routine is used to do memcpy between a kernel buffer and a user buffer
 * during short msg transfer. It will guranteed to return an error msg when the user
 * buffer can not be accessed.
 *
 */
int xfer_memcpy(void *dst, const void *src, size_t len) {
	void *old_handler;
	register uint8_t *d;
	register const uint8_t *s;
	
	if(!(old_handler = (void*) GET_XFER_HANDLER())) {
		jmp_buf					env;
		int						status;

		/* Setup the jump buffer for all exiting of this function */
		xfer_env = &env;
		if(status=xfer_setjmp(*xfer_env)) {
			return status;
		}
		SET_XFER_HANDLER(&xfer_src_handlers);
	}
	d = dst;
	s = src;
	while(len--) {
		*d++ = *s++;
	}
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


	/* Setup the jump buffer for all exiting of this function */
	xfer_env = &env;
	if(status=xfer_setjmp(*xfer_env)) {
		return status;
	}

	if(dparts < 0) {
		if(!WR_PROBE_PTR(thpd, dst, -dparts)) {
			return XFER_DST_FAULT;
		}
		SET_XFER_HANDLER(&xfer_src_handlers);
		xfer_memcpy(dst, saddr, min(slen,-dparts));
		SET_XFER_HANDLER(0);
		return 0;
	} else
		if(dparts == 0) return 0;

	SET_XFER_HANDLER(&xfer_dst_handlers);
	daddr = (char *)GETIOVBASE(dst);
	dlen = GETIOVLEN(dst);
	if(!WR_PROBE_PTR(thpd, daddr, dlen)) {
		return XFER_DST_FAULT;
	}

	SET_XFER_HANDLER(&xfer_src_handlers);

	/* Now we move the data. */
	if(slen <= dlen) {
			xfer_memcpy(daddr, saddr, slen);
	} else {
		// rare case
		while(1) {	
			xfer_memcpy(daddr, saddr, dlen);
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
				xfer_memcpy(daddr, saddr, slen);
				break;
			}
		}
	}

	SET_XFER_HANDLER(0);
	return 0;
}

__SRCVERSION("nano_xfer_cpy.c $Rev: 164239 $");
