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
#include "externs.h"

#define XFER_CHUNKSIZE			4096

unsigned (xferiov_pos)(CPU_REGISTERS *regs) {
//	uintptr_t			*ip = (uintptr_t *)REGIP(regs);

	return 0;
}

static int xfer_cpy(THREAD *thp, char *dst, char *src, unsigned nbytes) {
	unsigned	size, status;
	jmp_buf		env;

	xfer_env = &env;
	if((status = xfer_setjmp(*xfer_env))) {
		return status;
	}

	do {
		xfer_memcpy(dst, src, size = min(nbytes, XFER_CHUNKSIZE));
		thp->args.ms.msglen += size;
		src += size;
		dst += size;
	} while(nbytes -= size);

	return 0;
}

int (xferiov)(THREAD *sthp, IOV *dst, IOV *src, int dparts, int sparts, int doff, int soff) {
	char		*daddr,  *saddr;
	unsigned	dlen, slen, ret;
	
#ifndef NDEBUG
if(doff > GETIOVLEN(dst)) crash();
#endif
	daddr = (char *)GETIOVBASE(dst) + doff;
	dlen = GETIOVLEN(dst) - doff;

#ifndef NDEBUG
if(soff > GETIOVLEN(src)) crash();
#endif
	saddr = (char *)GETIOVBASE(src) + soff;
	slen = GETIOVLEN(src) - soff;

	/* Now we move the data. */
	for(;;) {
		if(slen < dlen) {
			ret = xfer_cpy(sthp, daddr, saddr, slen);
			if((--sparts == 0) || ret) {
				break;
			}
			daddr += slen;
			dlen -= slen;
			++src;
			saddr = (char *)GETIOVBASE(src);
			slen  = GETIOVLEN(src);
		} else if(dlen < slen) {
			ret = xfer_cpy(sthp, daddr, saddr, dlen);
			if((--dparts == 0) || ret) {
				break;
			}
			saddr += dlen;
			slen -= dlen;
			++dst;
			daddr = (char *)GETIOVBASE(dst);
			dlen  = GETIOVLEN(dst);
		} else {
			ret = xfer_cpy(sthp, daddr, saddr, slen);
			if((--dparts == 0) || (--sparts == 0) || ret) {
				break;
			}
			++src;
			saddr = (char *)GETIOVBASE(src);
			slen  = GETIOVLEN(src);
			++dst;
			daddr = (char *)GETIOVBASE(dst);
			dlen  = GETIOVLEN(dst);
		}
	}
	return(ret);
}


__SRCVERSION("nano_xfer_msg.c $Rev: 153052 $");
