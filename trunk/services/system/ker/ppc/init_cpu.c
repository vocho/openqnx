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

extern void tlb_flush_all_tlbia(void);

extern const struct exc_copy_block	enterkernel;
extern const struct exc_copy_block	exitkernel_1;
extern const struct exc_copy_block	exitkernel_2;
extern const struct exc_copy_block	exitkernel_3;
extern const struct exc_copy_block	exitkernel_4;
extern const struct exc_copy_block	exitkernel_5a;
extern const struct exc_copy_block	exitkernel_5b;
extern const struct exc_copy_block	exitkernel_6;

extern const struct exc_copy_block phys_mmu_on;
extern const struct exc_copy_block phys_mmu_off;

#define MK_OPCODE(opcode, r1, r2, immed) \
		(((opcode)<<26)+((r1)<<21)+((r2)<<16)+(immed))
		

static void
flush_icache(uint32_t *exc, unsigned len) {
	while(len != 0) {
		icache_flush((uintptr_t)exc);
		exc += 1;
		len -= sizeof(uint32_t);
	}
}

void
copy_code(uint32_t *dst, const struct exc_copy_block *src) {
	memcpy(dst, src->code, src->size);
	flush_icache(dst, src->size);
}

/*
 * Installs a code sequence for a set of exception table entries.
 */

void
trap_install_set(const struct trap_entry *trap, unsigned num_traps) {
	do {
		trap_install(trap->index, trap->func, trap->exc_code);
		++trap;
	} while(--num_traps != 0);
}

uint32_t *
trap_install(unsigned idx, void (*trap_func)(), const struct exc_copy_block *pref) {
	uint32_t	*start;
	uint32_t	*vector;
	unsigned	size;

	size = pref->size;
	if(trap_func != NULL) size += 4*sizeof(vector[0]);

	start = exc_vector_address(idx, size);
	vector = start;

	copy_code(vector, pref);
	vector = (uint32_t *)((uintptr_t)vector + pref->size);

	//
	// KLUDGE: We need to rework the kernel entry sequences to make
	// the following stuff cleaner.
	//
	// Careful with these instructions sequences. See trap_chain_addr()
	// below for rationale.
	//
	if(trap_func != NULL) {
	
		// lis 	%r3, func >> 16
		// ori 	%r3, func & 0xffff
		// mtlr %r3
		// ba   PPC_KERENTRY_COMMON

		vector[0] = MK_OPCODE(15, 3, 0, (uint32_t)trap_func >> 16);
		vector[1] = MK_OPCODE(24, 3, 3, (uint32_t)trap_func & 0xffff);
		vector[2] = 0x7c6803a6;
		vector[3] = 0x48000002 | PPC_KERENTRY_COMMON;

		flush_icache(vector, 4*sizeof(vector[0]));
	}
	return start;
}

uintptr_t
trap_chain_addr(unsigned idx) {
	unsigned	old;
	uint32_t	*vector;

	vector = exc_vector_address(idx, 0);
	old = *vector;
	if(old != 0x60000000) return 0;

	// Instruction at vector was a NOP - we've installed an exception
	// handler here and any interrupt that the id callout can't identify
	// wants chain to the old handler.

	// We assume that the __common_exc_entry sequence is at
	// the start of the vector (that's the only thing that has a NOP
	// at the start of it). Skip past the entry sequence to find out
	// where it's transfering to. Following the sequence will either
	// be a branch instruction, or two instructions that loads a
	// register with the address (see trap_install() above).

	vector += __common_exc_entry.size / 4;
	old = vector[0];
	if((old & 0xfc000003) == 0x48000002) {
		return(old & ~0xfc000003);
	}
	return(((old & 0xffff) << 16) | (vector[1] & 0xffff));
}

#define BLR_OPCODE 0x4e800020

static uintptr_t
gen_one(uintptr_t addr, const struct exc_copy_block *code) {
	unsigned	size = code->size;

	memcpy((void *)addr, code->code, size);
	return addr + size;
}

static void
gen_entry(const struct exc_copy_block *mmu_on, const struct exc_copy_block **extra) {
	uintptr_t					low_code;
	const struct exc_copy_block	*curr;

	low_code = gen_one(PPC_KERENTRY_COMMON, &enterkernel);
	for( ;; ) {
		curr = *extra;
		if(curr == NULL) break;
		low_code = gen_one(low_code, curr);
		++extra;
	}
// NOTE: The ker/ppc/booke/vm_e500.s has a workaround for the CPU29
// errata in the mmu_on code that assumes it is being immediately followed
// by a BLR instruction. Don't change this sequence without modifying the
// workaround.
	low_code = gen_one(low_code, mmu_on);
	*(uint32_t *)low_code = BLR_OPCODE;
	low_code += sizeof(uint32_t);
	flush_icache((void *)PPC_KERENTRY_COMMON, low_code - PPC_KERENTRY_COMMON);
}

static void
gen_exit(const struct exc_copy_block *mmu_off, const struct exc_copy_block **extra) {
	uintptr_t					low_code;
	uintptr_t					usr_b_addr;
	uintptr_t					kcall_b_addr;
	const struct exc_copy_block	*curr;

	low_code = PPC_KEREXIT_COMMON;
	for( ;; ) {
		curr = *extra;
		if(curr == NULL) break;
		low_code = gen_one(low_code, curr);
		++extra;
	}
	low_code = gen_one(low_code, &exitkernel_1);
	usr_b_addr = low_code - sizeof(uint32_t);	// remember usr branch ins
	low_code = gen_one(low_code, mmu_off);
	low_code = gen_one(low_code, &exitkernel_2);
	kcall_b_addr = low_code - sizeof(uint32_t);	//remember kcall branch ins

	// Make user branch go to correct address
	*(uint32_t *)usr_b_addr += (low_code - usr_b_addr);

	low_code = gen_one(low_code, &exitkernel_3);

	if(exitkernel_2.size > 0) {
		// Make kcall branch go to correct address
		*(uint32_t *)kcall_b_addr += (low_code - kcall_b_addr);
	}

	low_code = gen_one(low_code, &exitkernel_4);

	// Work around for CPU errata 25 on certain Freescale chips
	if(__cpu_flags & PPC_CPU_STWCX_BUG) {
		low_code = gen_one(low_code, &exitkernel_5b);
	} else {
		low_code = gen_one(low_code, &exitkernel_5a);
	}
	low_code = gen_one(low_code, &exitkernel_6);
	flush_icache((void *)PPC_KEREXIT_COMMON, low_code - PPC_KEREXIT_COMMON);
}


//RUSH3: Shim code to be removed later
unsigned
vmm_fault_shim(PROCESS *prp, void *vaddr, unsigned flags) {
	struct fault_info	info;
	paddr_t				paddr;
	unsigned			sigcode;
	extern unsigned		cpu_vmm_vaddrinfo(PROCESS *, uintptr_t, paddr_t *, size_t *);
	unsigned  r = cpu_vmm_vaddrinfo(prp, (uintptr_t)vaddr, &paddr, NULL);

	info.cpu.code = flags & ~0xffff;
	info.cpu.asid = flags &  0xffff;
	info.prp = prp;
	info.vaddr = (uintptr_t)vaddr;
	
	if (flags & VM_FAULT_WIMG_ERR) {
		sigcode = MAKE_SIGCODE(SIGSEGV, SEGV_ACCERR, FLTACCESS);
	} else if ((r != PROT_NONE) &&	// required since PROT_NONE == 0
		(((flags & VM_FAULT_WRITE) && ((r & PROT_WRITE) == 0)) ||
		 ((flags & VM_FAULT_INSTR) && ((r & PROT_EXEC) == 0)))) {
		/*
		 * a real fault - note that if VM_FAULT_WRITE or VM_FAULT_INSTR bits
		 * are not set, a read fault (VM_FAULT_READ which does not exist) is implied
		*/
		sigcode = MAKE_SIGCODE(SIGSEGV, SEGV_ACCERR, FLTPAGE);
	} else {
		/* will get here for every other condition including PROT_NONE */
		sigcode = MAKE_SIGCODE(SIGSEGV, SEGV_MAPERR, FLTPAGE);
	}
	if(flags & VM_FAULT_WRITE) {
		sigcode |= SIGCODE_STORE;
	}
	if(flags & VM_FAULT_INKERNEL) {
		sigcode |= SIGCODE_KERNEL;
		if(flags & VM_FAULT_KEREXIT) {
			sigcode |= SIGCODE_KEREXIT;
		}
		if(GET_XFER_HANDLER() != NULL) {
			sigcode |= SIGCODE_INXFER;
		}
	}
	info.sigcode = sigcode;

	switch(memmgr.fault(&info)) {
	case -1:	// illegal access, drop a signal on the process
		// temp hack to turn off the SIGCODE_KERNEL/KEREXIT/INXFER bits
		// since those can confuse the kernel debugger.
		return info.sigcode & ~(SIGCODE_KERNEL|SIGCODE_KEREXIT|SIGCODE_INXFER);
	case 0:		// defer to process time
		//At this point in time, we need to make sure to
		//acquire the kernel, if we haven't already.
#if defined(VARIANT_smp)
// The code in kernel.s isn't properly detecting bad references in a locked
// kernel with SMP. I've put a comment in there to make sure it's fixed when
// we get rid of the shim, but adding this here for right now...
if((sigcode & SIGCODE_KERNEL) && (get_inkernel() & INKERNEL_LOCK)) crash();
#endif
		if(PageWait(info.vaddr, info.sigcode, info.prp->pid, memmgr.fault_pulse_code) != EOK) {
			/*
			 * We're in serious trouble here. We either got called from an
			 * interrupt handler or we've run out of memory. Return a
			 * somewhat strange code so that people know what's going on.
			 */
			return MAKE_SIGCODE(SIGILL, ILL_BADSTK, FLTSTACK);
		}
		break;
	case 1:		// access OK and corrected, retry instruction
		break;
	default: break;
	}
	return 0;
}

void
init_traps(void) {
	struct ppc_kerinfo_entry	*ker;
	unsigned					pvr;
	const struct exc_copy_block	*mmu_on;
	const struct exc_copy_block	*mmu_off;
	const struct exc_copy_block	*extra_entry[10];
	const struct exc_copy_block	*extra_exit[10];

	ker = SYSPAGE_CPU_ENTRY(ppc,kerinfo);
	pvr = ker->pretend_cpu;
	if(pvr == 0) {
		pvr = SYSPAGE_ENTRY(cpuinfo)->cpu;
	}
	if(ker->ppc_family == PPC_FAMILY_UNKNOWN) {
		ker->ppc_family = determine_family(pvr);
	}
	if(ker->ppc_family != PPC_FAMILY_MINE) {
		kprintf("Processor family/kernel mismatch (%d/%d)\n", (int)ker->ppc_family, PPC_FAMILY_MINE);
		crash();
	}

	exc_vector_init();

	tlb_flush_all = tlb_flush_all_tlbia;

	config_cpu(pvr, extra_entry, extra_exit);

	// install second half of general kerentry sequence

	mmu_on  = &phys_mmu_on;
	mmu_off = &phys_mmu_off;
	if(__cpu_flags & CPU_FLAG_MMU) {
		//
		// We make this a weak symbol so it doesn't get searched for.
		// If the family specific init_vm*.c gets hauled in, it'll be
		// defined and we'll get the virtual mmu on/off routines.
		// Done to avoid bringing in VM support code when linking the
		// standalone kernel.
		//
		extern void	get_mmu_code() __attribute__((weak));

		if(&get_mmu_code != 0) {
			get_mmu_code(&mmu_on, &mmu_off);
		}
	}

	gen_entry(mmu_on, extra_entry);
	gen_exit(mmu_off, extra_exit);

	SYSPAGE_CPU_ENTRY(ppc,kerinfo)->init_msr
		|= ppc_ienable_bits
		| (get_msr() & (PPC_MSR_ILE|PPC_MSR_LE))
		| PPC_MSR_ME | PPC_MSR_RI;
}

void
init_cpu() {
}

__SRCVERSION("init_cpu.c $Rev: 167800 $");
