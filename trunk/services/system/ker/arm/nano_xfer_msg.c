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
 * ARM specific xferiov()
 */

#include <unistd.h>
#include "externs.h"

extern void	xfer_copy_kern(uint32_t *, char *, char *, unsigned);
extern void	xfer_copy_user(uint32_t *, char *, char *, unsigned);

extern jmp_buf					*xfer_env;

int
(xferiov)(THREAD *sthp, IOV *dst, IOV *src, int dparts, int sparts, int doff, int soff)
{
	char		*daddr,  *saddr;
	unsigned	dlen, slen;
	jmp_buf		env;
	unsigned	status;

	/* Setup the jump buffer for all exiting of this function */
	xfer_env = &env;
	if ((status=xfer_setjmp(*xfer_env))) {
		return status;
	}

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
	for (;;) {
		if (slen < dlen) {
			if ((uintptr_t)daddr < VM_USER_SPACE_BOUNDRY) {
				xfer_copy_user(&sthp->args.ms.msglen, daddr, saddr, slen);
			} else {
				xfer_copy_kern(&sthp->args.ms.msglen, daddr, saddr, slen);
			}
			if (--sparts == 0) {
				break;
			}
			daddr += slen;
			dlen -= slen;
			++src;
			saddr = (char *)GETIOVBASE(src);
			slen  = GETIOVLEN(src);
		} else if (dlen < slen) {
			if ((uintptr_t)daddr < VM_USER_SPACE_BOUNDRY) {
				xfer_copy_user(&sthp->args.ms.msglen, daddr, saddr, dlen);
			} else {
				xfer_copy_kern(&sthp->args.ms.msglen, daddr, saddr, dlen);
			}
			if (--dparts == 0) {
				break;
			}
			saddr += dlen;
			slen -= dlen;
			++dst;
			daddr = (char *)GETIOVBASE(dst);
			dlen  = GETIOVLEN(dst);
		} else {
			if ((uintptr_t)daddr < VM_USER_SPACE_BOUNDRY) {
				xfer_copy_user(&sthp->args.ms.msglen, daddr, saddr, slen);
			} else {
				xfer_copy_kern(&sthp->args.ms.msglen, daddr, saddr, slen);
			}
			if (--dparts == 0) {
				break;
			}
			if (--sparts == 0) {
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
	return(0);
}

__SRCVERSION("nano_xfer_msg.c $Rev: 153052 $");
