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

#include "externs.h"

/*
 * Save area for cmpxchg and __inline_InterruptEnable/Disable context
 *
 * The emulation code saves user register context before performing any
 * operation that can encounter a TLB exception.
 */
struct __emu_save {
	unsigned	active;
	unsigned	spc;
	unsigned	ssr;
	unsigned	r1;
	unsigned	r0;
};
struct __emu_save	*__ker_emu_save[PROCESSORS_MAX];
struct __emu_save	__emu_save[PROCESSORS_MAX];

extern void __exc_unexpected();
extern void __exc_general_start();
extern void __exc_general_end();

#define MK_MOV_PC_L(disp,rn)	(0xd000 + disp + (rn<<8))
/* used only in four bytes boundary */
void
set_trap(unsigned off, void (*trap_func)(), unsigned extra) {
	uint16_t	*vector;
	uint32_t	*data;
	int32_t		func;

	vector = (uint16_t*) ((unsigned)_syspage_ptr->un.sh.exceptptr  + off) ; /* put in P2 area */
	data = (uint32_t*)(vector + 4);
	func = (uint32_t)trap_func;
	/* mov.l @(1,pc),r0 */
	*vector++ = MK_MOV_PC_L(1, 0);
	/* nop */
	*vector++ = 0x9;
	/* jmp @r0 */
	*vector++ = 0x402b;
	/* nop */
	*vector++ = 0x9;
	/* data */
	*data = func | extra;
	icache_flush((uintptr_t)vector);
	icache_flush((uintptr_t)vector-8);
}

void
copy_code(uint32_t *exc, void *code_start, void *code_end) {
	int	len = (uintptr_t)code_end - (uintptr_t)code_start;

	memcpy(exc, code_start, len);
	while(len > 0) {
		icache_flush((uintptr_t)exc);
		exc += 8; /* SH4 cache line length: 32 bytes */
		len -= 32;
	}
}

void
install_traps() {
	copy_code((void*)((unsigned)_syspage_ptr->un.sh.exceptptr + SH_EXC_GENERAL), (void*)__exc_general_start, (void*)__exc_general_end);
}


void
init_traps(void) {

	/*
	 * Fill the exception area with branch instructions so
	 * if we get something we don't expect, the kernel will hardcrash.
	 */
	set_trap(SH_EXC_GENERAL, __exc_unexpected, 0x00);
	set_trap(SH_EXC_TLBMISS, __exc_unexpected, 0x00);
	set_trap(SH_EXC_INTR, __exc_unexpected, 0x00);

    install_traps();
}



// P4 address for TMU register base, assigned by init_cpu().
paddr_t sh_mmr_tmu_base_address = 0;

void
init_cpu() {
	int		i;

	/*
	 * Set up __ker_emu_save[] pointers for kernel.s emulation code
	 */
	for (i = 0; i < NUM_PROCESSORS; i++) {
		__ker_emu_save[i] = &__emu_save[i];
	}

}

void
halt() {
	//NIY: add power management code later
}

__SRCVERSION("init_cpu.c $Rev: 156873 $");
