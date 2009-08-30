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
#define SYSPAGE_TARGET_MIPS
#include _NTO_HDR_(sys/syspage.h)
#include _NTO_HDR_(mips/cpu.h)
#include _NTO_HDR_(mips/vm.h)
#include _NTO_HDR_(mips/context.h)


/*
 Define a structure used for communication to a GDB client
 */
typedef struct {
    int regs[32];                       /* CPU registers */
    int sr;                             /* status register */
    int lo;                             /* LO */
    int hi;                             /* HI */
    int bad;                            /* BadVaddr */
    int cause;                          /* Cause */
    int pc;                             /* EPC */
    int fp;                             /* Psuedo frame pointer */
} mips_gdb_regs;

typedef struct {
	uint32_t	lo;
	uint32_t	pm;
} pte_t;

static unsigned		pfn_topshift;


static int
mips_init(int elf_cpu) {
	if(elf_cpu != EM_MIPS) return 0;

	pfn_topshift = note->cpu_info & MIPS_CPU_FLAG_PFNTOPSHIFT_MASK;
	
	return 1;
}


static int
mips_v2p(uintptr_t vaddr, paddr64_t *paddrp, unsigned *lenp) {
	uint32_t	l2vaddr;
	pte_t		pte;
	paddr64_t	paddr;
	unsigned	offset;
	unsigned	pgsize;
	pte_t		**page_table = pgtbl;

	if(MIPS_IS_KSEG0(vaddr)) {
		*paddrp = MIPS_KSEG0_TO_PHYS(vaddr);
		*lenp = MIPS_R4K_K0SIZE - *paddrp;
		return 1;
	}
	// consult the page table...
	l2vaddr = endian_native32((uintptr_t)page_table[L1IDX(vaddr)]);
	if(l2vaddr == 0) return 0;

	core_read_paddr(MIPS_KSEG0_TO_PHYS(l2vaddr) + L2IDX(vaddr)*sizeof(pte_t), &pte, sizeof(pte));
	pte.lo = endian_native32(pte.lo);
	pte.pm = endian_native32(pte.pm);
	if(note->cpu_info & MIPS_CPU_FLAG_NO_WIRED) {
		// was an R3K
		if(!(pte.lo & MIPS3K_TLB_VALID)) return 0;
		paddr = pte.lo & MIPS3K_TLB_LO_PFNMASK;
		pgsize = 0x1000;
	} else {
		if(!(pte.lo & MIPS_TLB_VALID)) return 0;
		paddr = ((paddr64_t)(pte.lo & MIPS_TLB_LO_PFNMASK)) << pfn_topshift;
		pgsize = ((pte.pm | 0x1fff) + 1) >> 1;
	}
	offset = vaddr & (pgsize-1);
	*paddrp = paddr | offset;
	*lenp  = pgsize - offset;
	return 1;
}


static void
mips_cvt_regset(const void *in, void *out) {
	const MIPS_CPU_REGISTERS	*ctx = in;
	mips_gdb_regs				*gdb = out;
    unsigned 					i;
	int							big_endian;

#if defined(__BIGENDIAN__)
	#define NATIVE_BIG	1
#elif defined(__LITTLEENDIAN__)
	#define NATIVE_BIG	0
#else
	#error ENDIAN not defined
#endif	

	if(endian_native32(1) != 1) {
		// cross endian
		big_endian = !NATIVE_BIG;
	} else {
		big_endian = NATIVE_BIG;
	}

	// Playing games. The MIPS_CREG macro assumes that we're running
	// on the actual system as far as endianness goes. We're going
	// to undef MIPS_REGS_LOW_WORD (which is the dependent piece)
	// and redefine it so that it works properly for the endianness
	// of the target system, not the host that kdserver is running on.
	#undef MIPS_REGS_LOW_WORD
	#define MIPS_REGS_LOW_WORD	big_endian	

    for(i=0; i < 32; ++i) {
		gdb->regs[i] = ctx->regs[MIPS_CREG(i)];
	}

    gdb->sr = ctx->regs[MIPS_CREG(MIPS_REG_SREG)];
    gdb->lo = ctx->regs[MIPS_CREG(MIPS_REG_LO)];
    gdb->hi = ctx->regs[MIPS_CREG(MIPS_REG_HI)];
    gdb->bad = ctx->regs[MIPS_CREG(MIPS_REG_BADVADDR)];
    gdb->cause = 0;
    gdb->pc = ctx->regs[MIPS_CREG(MIPS_REG_EPC)];
    gdb->fp = ctx->regs[MIPS_CREG(MIPS_REG_SP)];
}

struct cpuinfo 	cpu_mips = {
	mips_init,
	mips_v2p,
	mips_cvt_regset,
	0x2000,
	sizeof(mips_gdb_regs),
	sizeof(uint32_t),
};
