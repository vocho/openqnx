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
 * kernel/mips.h: kernel consts and data type matchups.
 *

 */
#include <mips/cpu.h>
#include <mips/inline.h>
#include <mips/priv.h>
#include <mips/smpxchg.h>

#define FAULT_ISWRITE(_n)	((_n) & SIGCODE_STORE)

typedef MIPS_CPU_REGISTERS	CPU_REGISTERS;
typedef MIPS_FPU_REGISTERS	FPU_REGISTERS;
typedef MIPS_ALT_REGISTERS	ALT_REGISTERS;
typedef MIPS_PERFREGS		PERF_REGISTERS;
#define CPU_STRINGNAME		MIPS_STRINGNAME

#define STARTUP_STACK_NBYTES		4096
#define IDLE_STACK_NBYTES			512
#define KER_STACK_NBYTES			8192
#define DEF_MAX_THREAD_STACKSIZE	32768

#define DEF_VIRTUAL_THREAD_STACKSIZE		131072
#define DEF_PHYSICAL_THREAD_STACKSIZE		4096
#define DEF_VIRTUAL_FIRST_THREAD_STACKSIZE	524288
#define DEF_PHYSICAL_FIRST_THREAD_STACKSIZE	4096

#define STACK_ALIGNMENT						8
#define STACK_INITIAL_CALL_CONVENTION_USAGE	16

#define VM_USER_SPACE_BOUNDRY		0x7fffffff
#define VM_KERN_SPACE_BOUNDRY		0xffffffff

#define CPU_P2V(p)			MIPS_PHYS_TO_KSEG0(p)
#define CPU_P2V_NOCACHE(p)	MIPS_PHYS_TO_KSEG1(p)

#define CPU_V2P(v)			((paddr_t)((uintptr_t)(v) & ~0xe0000000))

#define CPU_VADDR_IN_RANGE(v) \
	((uintptr_t)(v) >= MIPS_R4K_K0BASE && (uintptr_t)(v) < MIPS_R4K_K3BASE)


/*
 * registers that need to be saved for invoking an interrupt handler
 */
struct cpu_intrsave {
	uint64_t	gp;
};

/* Breakpoint fields that this cpu may need for breakpoints */
#define CPU_DEBUG_BRKPT		\
	paddr_t		paddr;		\
	uint32_t	old;

/* Fields this cpu may need for debugging a process */
#define CPU_DEBUG			\
	unsigned	step_cpu;	\
	BREAKPT		step;		\
	unsigned	num_watchpoints; \
	unsigned	real_step_flags;

/* CPU specific fields in the thread entry structure */
struct cpu_thread_entry {
	MIPS_PERFREGS				*pcr;
#if defined(TX79_SUPPORT)
	struct mips_alt_registers	alt; /* must be last field */
#endif
};

struct cpu_fault_info {
	CPU_REGISTERS	*regs;
	int				unused;
};

struct r4k_tlb {
	_Uint32t pmask;
	_Uint32t hi;
	_Uint32t lo0;
	_Uint32t lo1;
};

struct pte {
	uint32_t		lo;
	uint32_t		pm;
};

/* Extra state (read only) made available by the kernel debugger/dumper */
struct cpu_extra_state {
	struct {
		unsigned		index;
		unsigned		random;
		unsigned		entrylo0;
		unsigned		entrylo1;
		unsigned		context;
		unsigned		pagemask;
		unsigned		wired;
		unsigned		__7;
		unsigned		badvaddr;
		unsigned		count;
		unsigned		entryhi;
		unsigned		compare;
		unsigned		status;
		unsigned		cause;
		unsigned		epc;
		unsigned		prid;
		unsigned		config;
		unsigned		lladdr;
		unsigned		watchlo;
		unsigned		watchhi;
		unsigned		xcontext[2];
		unsigned		__21;
		unsigned		__22;
		unsigned		__23;
		unsigned		__24;
		unsigned		__25;
		unsigned		ecc;
		unsigned		cacheerr;
		unsigned		taglo;
		unsigned		taghi;
		unsigned		err_epc;
		unsigned		ipllo;
		unsigned		iplhi;
		unsigned		intctl;
		unsigned		erraddr0;
		unsigned		erraddr1;
	}				regs;
	struct r4k_tlb	tlb[64];
};


extern void r4k_gettlb(struct r4k_tlb *, uint32_t);
extern void r4k_settlb(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern void r4k_update_tlb(uint32_t hi, uint32_t lo0, uint32_t lo1, uint32_t pgmask);
extern int  r4k_flush_tlb(uint32_t hi);
extern void set_l1pagetable(void *, unsigned);

/* __SRCVERSION("cpu_mips.h $Rev: 169331 $"); */
