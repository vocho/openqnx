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
#define SYSPAGE_TARGET_PPC
#include _NTO_HDR_(sys/syspage.h)
#include _NTO_HDR_(ppc/cpu.h)
#include _NTO_HDR_(ppc/context.h)

/*
 Define a structure used for communication to a GDB client
 */
typedef struct {
	uint32_t	gpr[32];
	double		fpr[32];
	uint32_t	pc;
	uint32_t	ps;
	uint32_t	cnd;
	uint32_t	lr;
	uint32_t	cnt;
	uint32_t	xer;
	uint32_t	mq;
} ppc_gdb_regs;


static int
ppc_init(int elf_cpu) {
	if(elf_cpu != EM_PPC) return 0;

	switch(note->cpu_info) {
	case PPC_FAMILY_booke:
	case PPC_FAMILY_900:
		cpu->pgtbl_size = 0x2000;
		break;
	}

	return 1;
}


#define ONE_TO_ONE_SIZE	(256*1024*1024)

static int
ppc_v2p(uintptr_t vaddr, paddr64_t *paddrp, unsigned *lenp) {
	uint32_t	l2vaddr;
	paddr64_t	paddr;
	unsigned	offset;
	unsigned	pgsize;
	unsigned	l1shift;
	unsigned	l2mask;
	unsigned	pte_size;
	union {
		struct {
			uint32_t	paddr;
			uint32_t	flags;
		}			_bke;
		uint32_t	_400;
		uint32_t	_600;
		uint32_t	_800;
		uint64_t	_900;
	}			pte;

	if(vaddr < ONE_TO_ONE_SIZE) {
		*paddrp = vaddr;
		*lenp = ONE_TO_ONE_SIZE - *paddrp;
		return 1;
	}

	// consult the page table...

	if(cpu->pgtbl_size == 0x2000) {
		l1shift = 21;
		l2mask  = 0x1ff;
		pte_size = 8;
	} else {
		l1shift = 22;
		l2mask  = 0x3ff;
		pte_size = 4;
	}
	l2vaddr = endian_native32(((uintptr_t *)pgtbl)[vaddr >> l1shift]) & ~0x3ff;
	if(l2vaddr == 0) return 0;
	core_read_vaddr(l2vaddr + ((vaddr >> 12) & l2mask)*pte_size, &pte, pte_size);

	pgsize = 0x1000;
	switch(note->cpu_info) {
	case PPC_FAMILY_booke: 
		pte._bke.paddr = endian_native32(pte._bke.paddr);
		paddr = (pte._bke.paddr & ~0xfff) | ((paddr64_t)(pte._bke.paddr & 0xfff) << 32);
		pte._bke.flags = endian_native32(pte._bke.flags);
		pgsize = 0x400 << (((pte._bke.flags >> 8) & 0xf) * 2);
		break;
	case PPC_FAMILY_400:
	case PPC_FAMILY_800:
		pte._400 = endian_native32(pte._400);
		if(!(pte._400 & 0x3ff)) return 0;
		paddr = pte._400 & ~0x3ff;
		break;
	case PPC_FAMILY_600:
		pte._600 = endian_native32(pte._600);
		if(!(pte._600 & 0x3ff)) return 0;
		paddr = ((paddr64_t)((pte._600) & ~(0x3ff)))
				| ((paddr64_t)(pte._600 & 0x00000e00) << 24)
				| ((paddr64_t)(pte._600 & 0x4) << 30);
		break;
	case PPC_FAMILY_900:
		paddr = endian_native64(pte._900) & ~(paddr64_t)0xfff;
		break;
	default: 
		if(debug_flag > 1) {
			fprintf(stderr, "Unknown PPC family\n");
		}
		return 0;
	}
	offset = vaddr & (pgsize-1);
	*paddrp = paddr | offset;
	*lenp  = 0x1000 - offset;
	return 1;
}


static void
ppc_cvt_regset(const void *in, void *out) {
	const PPC_CPU_REGISTERS	*ctx = in;
	ppc_gdb_regs			*gdb = out;

	if ( protocol == 1 ) {
		/* Protocol 1 context has cpu type as first word - we use 0 "generic PPC" */
		*((uint32_t *)out) = 0;
		gdb = (ppc_gdb_regs *) &((int32_t *)out)[1];
	}
	memcpy(gdb->gpr, ctx->gpr, sizeof(gdb->gpr));
	memset(gdb->fpr, 0, sizeof(gdb->fpr));
	gdb->pc = ctx->iar;
	gdb->ps = ctx->msr;
	gdb->cnd = ctx->cr;
	gdb->lr = ctx->lr;
	gdb->cnt = ctx->ctr;
	gdb->xer = ctx->xer;
	gdb->mq = ctx->u.mq;
}


struct cpuinfo 	cpu_ppc = {
	ppc_init,
	ppc_v2p,
	ppc_cvt_regset,
	0x1000,
	sizeof(ppc_gdb_regs),
	sizeof(uint32_t),
};
