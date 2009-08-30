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

#include <sys/memmsg.h>
#include <sys/trace.h>
#include "mm_internal.h"

/*
 * This flag determines whether we should enfore address alignement
 * restrictions as documented by POSIX 1003.1. This is only done
 * if procnto was run with the -ma flag.
 */
#define VV_ALIGN	0x1000

static int
validate_vaddr(PROCESS *prp, uintptr_t vaddr, size_t len, int flags) {

	if(prp->flags & _NTO_PF_TERMING) {
		// If we're terminating, we're running code in procnto and can
		// trust the values - sometimes we need to pass in things that
		// would normally be illegal.
		return EOK;
	}

	/*
	 * Only enforce alignement is ENFORCE_ALIGNMENT is on.
	 * FUTURE: this is off for the first release of trinity as 
	 * this is a new requirement in POSIX 1003.1 2003 which we
	 * do not believe is valid. It also breaks a lot of code.
	 */
	if((flags & VV_ALIGN) && (mm_flags & MM_FLAG_ENFORCE_ALIGNMENT)) {
		if((vaddr & (memmgr.pagesize-1)) != 0) return EINVAL;
	}
	return memmgr.validate(prp, vaddr, len, flags);
}


int 
memmgr_ctrl(PROCESS *prp, mem_ctrl_t *msg) {
	int			r;
	unsigned	flags;

	/* If the range is too large, quietly truncate it */
	/* All the casting is to do 32-bit arithmetic on 32-bit machines */

	if((size_t)msg->i.len > ~(uintptr_t)0 - (uintptr_t)msg->i.addr) {
		msg->i.len = ~(uintptr_t)0 - (uintptr_t)msg->i.addr;
	}

	switch(msg->i.subtype) {
	case _MEM_CTRL_UNMAP:
		if(msg->i.len == 0) {
			return EINVAL;
		} 
		if(msg->i.flags & ~(UNMAP_INIT_REQUIRED|UNMAP_INIT_OPTIONAL)) {
			return EINVAL;
		}
		r = validate_vaddr(prp, msg->i.addr, msg->i.len, VV_ALIGN|VV_RANGE);
		if(r != EOK) return r;
		r = memmgr.munmap(prp, msg->i.addr, msg->i.len, msg->i.flags, mempart_getid(prp, sys_memclass_id));

#if defined(VARIANT_instr)
		{
			iov_t iov[2];
			SETIOV(iov, &prp->pid, sizeof(prp->pid));
			SETIOV(iov + 1, &msg->i.addr, sizeof(msg->i.addr) + sizeof(msg->i.len));
			(void) KerextAddTraceEventIOV(_TRACE_SYSTEM_C, _NTO_TRACE_SYS_MUNMAP, iov, 2);
		}
#endif

		return r;

	case _MEM_CTRL_PROTECT:
		if(msg->i.flags & ~PROT_MASK) {
			return EINVAL;
		}

		flags =	VV_RANGE|VV_MAPPED;
//START KLUDGE		
		//FUTURE: POSIX says that mprotect() requires the vaddr
		//FUTURE: to be page aligned. Unfortunately, the ldd code
		//FUTURE: in libc has a whack of unaligned mprotect()'s.
		//FUTURE: Obviously, libc has to be fixed. However, we'd
		//FUTURE: like old libc's to continue to work. So we
		//FUTURE: are going to allow non-aligned vaddr's here
		//FUTURE: and do the check in vmm_mprotect, but only if
		//FUTURE: MAP_ELF is off. That will follow POSIX (mostly),
		//FUTURE: since the only code that turns on MAP_ELF is
		//FUTURE: the loader and ldd. In a few years, we can drop
		//FUTURE: support and require the fixed libc's. Don't forget
		//FUTURE: the START/END KLUGE code in vmm_mprotect.c & mm_reference.c
		//FUTURE: when this gets fixed.  bstecher 2005/06/23.
#if 0		
		if(!(mm_flags & MM_FLAG_BACKWARDS_COMPAT)) {
			flags |= VV_ALIGN;
		}
#endif		
//END KLUDGE		
		r = validate_vaddr(prp, msg->i.addr, msg->i.len, flags);
		if(r != EOK) return r;
		return memmgr.mprotect(prp, msg->i.addr, msg->i.len, msg->i.flags);

	case _MEM_CTRL_SYNC:
#define ALL	(MS_INVALIDATE_ICACHE|MS_CACHE_ONLY|MS_SYNC|MS_ASYNC|MS_INVALIDATE)
		if(msg->i.flags & ~ALL) {
			// Illegal bit turned on
			return EINVAL;
		}
		if((msg->i.flags & (MS_SYNC|MS_ASYNC)) == (MS_SYNC|MS_ASYNC)) {
			// Not allowed to turn both on
			return EINVAL;
		}
		flags = VV_RANGE|VV_MAPPED;
		if(!(msg->i.flags & (MS_INVALIDATE_ICACHE|MS_CACHE_ONLY))) {
			// Non-cache operations may need to be page aligned
			flags |= VV_ALIGN;
		}
		r = validate_vaddr(prp, msg->i.addr, msg->i.len, flags);
		if(r != EOK) return r;
		return memmgr.msync(prp, msg->i.addr, msg->i.len, msg->i.flags);

	case _MEM_CTRL_LOCKALL:
		if((msg->i.flags == 0) || (msg->i.flags & ~(MCL_FUTURE|MCL_CURRENT)) != 0) {
			return EINVAL;
		}
		if(prp->cred->info.euid != 0) return EPERM;
		return memmgr.mlock(prp, 0, (size_t)~0, msg->i.flags);

	case _MEM_CTRL_LOCK:
		r = validate_vaddr(prp, msg->i.addr, msg->i.len, VV_RANGE|VV_MAPPED);
		if(r != EOK) return r;
		if(prp->cred->info.euid != 0) return EPERM;
		return memmgr.mlock(prp, msg->i.addr, msg->i.len, MCL_CURRENT);

	case _MEM_CTRL_UNLOCKALL:
		return memmgr.munlock(prp, 0, (size_t)~0, MCL_FUTURE|MCL_CURRENT);

	case _MEM_CTRL_UNLOCK:
		r = validate_vaddr(prp, msg->i.addr, msg->i.len, VV_RANGE|VV_MAPPED);
		if(r != EOK) return r;
		return memmgr.munlock(prp, msg->i.addr, msg->i.len, MCL_CURRENT);

	case _MEM_CTRL_ADVISE:
		if(msg->i.flags > POSIX_MADV_DONTNEED) {
			// Don't need to check for "< 0" because flags is unsigned
			return EINVAL;
		}
		r = validate_vaddr(prp, msg->i.addr, msg->i.len, VV_RANGE|VV_MAPPED);
		if(r != EOK) return r;
		return memmgr.madvise(prp, msg->i.addr, msg->i.len, msg->i.flags);

	default:
		break;
	}

	return ENOSYS;
}

__SRCVERSION("memmgr_ctrl.c $Rev: 160638 $");
