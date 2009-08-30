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




#include "kdserver.h"
#define SYSPAGE_TARGET_SH
#include _NTO_HDR_(sys/syspage.h)
#include _NTO_HDR_(sh/cpu.h)
#include _NTO_HDR_(sh/context.h)
#include _NTO_HDR_(sh/ccn.h)


/*
 * Define a structure used for communication to a GDB client.
 * This structure defines the registers in the format expected by
 * gdb.  It is cloned from services/kdebug/gdb/sh/kdbgcpu.h.
 */
typedef struct {
	shint		gr[16];
	shint		pc;
	shint		pr;
	shint		gbr;
	shint		vbr; /* syspage exceptptr */
	shint		mach;
	shint		macl;
	shint		sr;

	//_uint32		fpul;
	//_uint32		fpscr;
	//shfloat		fpr_bank0[16];

	//shint		ssr; /* not needed? */
	//shint		spc; /* not needed? */

	//shint		rb0[8]; /* not needed? */
	//shint		rb1[8]; /* not needed? */

} sh_gdb_regs;




static int
sh_init(int elf_cpu) {
	if(elf_cpu != EM_SH) return 0;
	
	return 1;
}


static int
sh_v2p(uintptr_t vaddr, paddr64_t *paddrp, unsigned *lenp) {

	// FIXME: don't handle vaddrs outside one-to-one area.

	if (SH_IS_P1(vaddr)) {
		*paddrp = SH_P1_TO_PHYS(vaddr);
		*lenp = SH_P1SIZE - (unsigned)(*paddrp);
		return 1;
	}	

	if (SH_IS_P2(vaddr)) {
		*paddrp = SH_P2_TO_PHYS(vaddr);
		*lenp = SH_P2SIZE - (unsigned)(*paddrp);
		return 1;
	}	

	return 0;
}


static void
sh_cvt_regset(const void *in, void *out) {
	const SH_CPU_REGISTERS		*ctx = in;
	sh_gdb_regs					*gdb = out;

	memcpy(&gdb->gr[0], &ctx->gr[0], sizeof(gdb->gr));
	gdb->pc = ctx->pc;
	gdb->pr = ctx->pr;
	gdb->gbr = ctx->gbr;
	gdb->vbr = 0;
	gdb->mach = ctx->mach;
	gdb->macl = ctx->macl;
	gdb->sr = ctx->sr;
	//gdb->fpul = 0;
	//gdb->fpscr = 0;
	//memcpy(&gdb->fpr_bank0, 0, sizeof(gdb->fpr_bank0));
	//gdb->ssr = 0;
	//gdb->spc = 0;
	//memset(&gdb->rb0, 0, sizeof(gdb->rb0));
	//memset(&gdb->rb1, 0, sizeof(gdb->rb1));
}

struct cpuinfo 	cpu_sh = {
	sh_init,
	sh_v2p,
	sh_cvt_regset,
	0x2000,
	sizeof(sh_gdb_regs),
	sizeof(uint32_t),
};


