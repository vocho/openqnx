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

#include "mm_internal.h"
#include <x86/v86.h>
#include <x86/cpumsg.h>


static void 
v86_map(void *on) {
	pxe_t				*pdep_zero;
	pxe_t				*ptep;
	unsigned			addr;
	unsigned			cr0 = rdcr0();

	KerextLock();
	ldcr0(cr0 & ~X86_MSW_WP_BIT);

	// get old page directory
	pdep_zero = VTOPDIRP((uintptr_t)0x00000000);

	// map real_addr to virtual address 0x0000000 for v86 mode
	if(pae_enabled) {
		pdep_zero->pxe64 = *V2TOPDIRP(_syspage_ptr->un.x86.real_addr);
	} else {
		pdep_zero->pxe32 = *V1TOPDIRP(_syspage_ptr->un.x86.real_addr);
	}

	// scan for 1M plus 64k flipping user bit
	for(addr = 0x00000000; addr < (1 << 20) + (64 << 10); addr += __PAGESIZE) {
		ptep = VTOPTEP(addr);
		if((PXE_GET_FLAGS(ptep) & (X86_PTE_PRESENT | X86_PTE_WRITE)) == (X86_PTE_PRESENT | X86_PTE_WRITE)) {
			if(on) {
				PXE_SET_FLAGS(ptep, ~0, X86_PTE_USER);
			} else {
				PXE_SET_FLAGS(ptep, ~X86_PTE_USER, 0);
			}
		}
	}

	// If asked to disable, remove mapping from address 0x00000000
	if(!on) {
		PXE_SET(pdep_zero, 0);
	}

	ldcr0(cr0);
}


int 
v86_handle(resmgr_context_t *ctp, x86_cpu_v86_t *msg) {
	static pthread_mutex_t		mutex = PTHREAD_MUTEX_INITIALIZER;
	int							err;

	if(!proc_isaccess(0, &ctp->info)) {
		MsgError(ctp->rcvid, EPERM);
		return _RESMGR_NOREPLY;
	}

	if(pthread_mutex_lock(&mutex) != EOK) {
		MsgError(ctp->rcvid, EBUSY);
		return _RESMGR_NOREPLY;
	}

	ProcessBind(SYSMGR_PID);

	__Ring0(v86_map, (void *)1);

	// read user registers and data to first address zero
	SETIOV(ctp->iov + 1, (uint8_t *)_syspage_ptr->un.x86.real_addr + offsetof(struct _v86_memory, reg),
		offsetof(struct _v86_memory, memory) - offsetof(struct _v86_memory, reg));
	(void) MsgReadv(ctp->rcvid, ctp->iov + 1, 1, offsetof(struct _x86_cpu_v86, regs));

	(void) ThreadCtl(_NTO_TCTL_RUNMASK, (void *)0x01);
	err = EOK;

	v86_mark_running(1);

	if (msg->i.swi & _V86_OPTION_CALLDIRECT) {
		long 					patch_save;
		long 					*patch;
		// it is a direct call in v86 mode 
		patch = (long *)((uint8_t *)_syspage_ptr->un.x86.real_addr + offsetof(struct _v86_memory, rvecs[(unsigned char)(msg->i.swi & 0xFF)]));
		// save/patch int vector at int number in least significant 8 bits 
		// so that interrupt vectors to userdata region
		patch_save = *patch;	
		*patch = (long)offsetof(struct _v86_memory, userdata);

		// Do the V86 emulation via direct call
		if(V86Enter(msg->i.swi) == -1) {
			err = errno;
		}

		// restore the patched vector
		*patch = patch_save;
	} else {
		// Do the V86 emulation via interrupt
		if(V86Enter(msg->i.swi) == -1) {
			err = errno;
		}
	}

	v86_mark_running(0);

	(void) ThreadCtl(_NTO_TCTL_RUNMASK, (void *)~0x00);

	if(err != EOK) {
		MsgError(ctp->rcvid, err);
	} else {
		// Return the resulting regs and data
		SETIOV(ctp->iov + 0, msg, offsetof(struct _x86_cpu_v86_reply, regs));
		MsgReplyv(ctp->rcvid, 0, ctp->iov, 2);
	}

	__Ring0(v86_map, (void *)0);

	ProcessBind(0);

	pthread_mutex_unlock(&mutex);

	return _RESMGR_NOREPLY;
}

__SRCVERSION("v86.c $Rev: 202117 $");
