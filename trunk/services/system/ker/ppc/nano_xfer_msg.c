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
 We have many tricks that we can use to speed up message transfer; these
 tricks depend on the CPU type and family we are using. 

 In the case of the 400 and 800, we have a "fast" memcpy routine 
 that we call when both source and dest are aligned. This is usually the
 case. If they are not aligned, we call the normal memcpy.

 In the case of the 600, we always call the fast memcpy routine, even
 if the operands are not aligned. This is because there's usually no penalty
 for a misaligned word access on these CPU's.

 In the case of the 7400+ with Altivec, we make use of the Altivec registers
 to move data when we're doing a large memcpy. Using the Altivec registers
 gives us much more bandwidth; we can easily get over 3GB/sec of raw 
 memcpy speed for larger transfers. We have to be careful, though, as we
 need to save temporarily a few VMX registers; we need to make sure these
 are restored in the case of a fault or preemption.
 *************************************************************************/

#include <unistd.h>
#include "externs.h"

#define XFER_CHUNKSIZE		4096


extern int					_mem_cpy(char *dst, char *src, unsigned nbytes);
extern int					_xfer_cpy(char *dst, char *src, unsigned nbytes);
extern uint32_t 			_xfer_cpy_start[], _xfer_cpy_end[];


#if defined(VARIANT_600) || defined(VARIANT_900)

#define XFER_VMX_THRESHOLD	128
extern int					_xfer_cpy_vmx(char *dst, char *src, unsigned nbytes, PPC_VMX_REGISTERS *regs);
extern void					_xfer_cpy_vmx_restore(PPC_VMX_REGISTERS *regs);
extern uint32_t 			_xfer_cpy_vmx_start[], _xfer_cpy_vmx_end[];

/* for restoring vmx regs in fault handler */
void xfer_fault_restore_vmx(CPU_REGISTERS *regs) {

	// Had we "borrowed" some of the vmx registers?
	if(actives_alt[KERNCPU] && ((uintptr_t *)REGIP(regs) >= _xfer_cpy_vmx_start) && ((uintptr_t *)REGIP(regs) < _xfer_cpy_vmx_end)) {
		_xfer_cpy_vmx_restore(&actives_alt[KERNCPU]->vmx);	
	}
}

unsigned (xferiov_pos)(CPU_REGISTERS *regs) {
	uintptr_t			*ip = (uintptr_t *)REGIP(regs);

	// The _xfer_cpy function saves the base dst in %r6; the current dst
	// is in %r3. The number of bytes xfered in therefore %r3 - %r6
	if(ip >= _xfer_cpy_start && ip < _xfer_cpy_end) {
		return (regs->gpr[3] - regs->gpr[6]);	
	} else if(ip >= _xfer_cpy_vmx_start && ip < _xfer_cpy_vmx_end) {
		// For altivec, base dst is in %r0
		if(actives_alt[KERNCPU]) {
			_xfer_cpy_vmx_restore(&actives_alt[KERNCPU]->vmx);
		}
		return (regs->gpr[3] - regs->gpr[0]);	
	}
	return 0;
}

static inline int xfer_cpy(THREAD *thp, char *dst, char *src, unsigned nbytes) {
	int status;
	
	if((nbytes > XFER_VMX_THRESHOLD) && (__cpu_flags & PPC_CPU_ALTIVEC)) {
//kprintf("copy with vmx: src %x dst %x len %x act %x\n", 
//src, dst, nbytes, &actives_alt[KERNCPU]->vmx);
		status = _xfer_cpy_vmx(dst, src, nbytes, &actives_alt[KERNCPU]->vmx);
	} else { 
		status = _xfer_cpy(dst, src, nbytes);
	}
	thp->args.ms.msglen += nbytes;
	return status;
}

#else

unsigned (xferiov_pos)(CPU_REGISTERS *regs) {
	uintptr_t			*ip = (uintptr_t *)REGIP(regs);

	// The _xfer_cpy function saves the base dst in %r6; the current dst
	// is in %r3. The number of bytes xfered in therefore %r3 - %r6
	if(ip >= _xfer_cpy_start && ip < _xfer_cpy_end) {
		return (regs->gpr[3] - regs->gpr[6]);	
	}
	return 0;
}

static inline int xfer_cpy(THREAD *thp, char *dst, char *src, unsigned nbytes) {
	int status;

	// Check if either buffer is unaligned
	if(((unsigned)dst | (unsigned)src) & 3) {
		unsigned			size;
		do {
			status = _mem_cpy(dst, src, size = min(nbytes, XFER_CHUNKSIZE));
			if(status) break;
			thp->args.ms.msglen += size;
			src += size;
			dst += size;
		} while((nbytes -= size));
	} else {
		status = _xfer_cpy(dst, src, nbytes);
		thp->args.ms.msglen += nbytes;
	}
	return status;
}
#endif

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
