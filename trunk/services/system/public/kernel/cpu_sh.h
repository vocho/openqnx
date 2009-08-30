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



#ifndef  _KERNEL_CPU_SH_H
#define  _KERNEL_CPU_SH_H
/*
*	kernel/cpu_sh.h


*/
#include <sh/cpu.h>
#include <sh/inline.h>
#include <sh/priv.h>
#include <sh/smpxchg.h>

#define FAULT_ISWRITE(_n)	((_n) & SIGCODE_STORE)

typedef SH_CPU_REGISTERS	CPU_REGISTERS;
typedef SH_FPU_REGISTERS	FPU_REGISTERS;
typedef SH_ALT_REGISTERS	ALT_REGISTERS;
typedef SH_PERFREGS			PERF_REGISTERS;
#define CPU_STRINGNAME		SH_STRINGNAME

#define STARTUP_STACK_NBYTES		4096
#define KER_STACK_NBYTES			8192
#define IDLE_STACK_NBYTES			512
#define DEF_MAX_THREAD_STACKSIZE	32768

#define DEF_VIRTUAL_THREAD_STACKSIZE		131072
#define DEF_PHYSICAL_THREAD_STACKSIZE		4096
#define DEF_VIRTUAL_FIRST_THREAD_STACKSIZE	524288
#define DEF_PHYSICAL_FIRST_THREAD_STACKSIZE	4096

#define STACK_ALIGNMENT						16
#define STACK_INITIAL_CALL_CONVENTION_USAGE	0

#define CPU_P2V(p)			SH_PHYS_TO_P1(p)
#define CPU_P2V_NOCACHE(p)	SH_PHYS_TO_P2(p)

#define CPU_V2P(v)			((paddr_t)((uintptr_t)(v) & ~0xe0000000))

#define CPU_VADDR_IN_RANGE(v) \
	((uintptr_t)(v) >= SH_P1BASE && (uintptr_t)(v) < SH_P3BASE)

/* These two address are hints, real v_addr will match the cache color */
#define VM_CPUPAGE_ADDR		0x7bff0000	/* U0 */
#define VM_SYSPAGE_ADDR		0x7bff4000	/* U0 */

/* v_addr for read system timer */
#define VM_TIMERPAGE_ADDR	0x7bff8000	/* U0 */
#define VM_TIMERPAGE_ADDR_OLD	0x7fff8000	/* U0 */
/* sh_mmr_tmu_base_address: P4 address for TMU register base, assigned by init_cpu().
 * In SH-4 architectures, we need to convert this to an A7 address when we put it
 * in the page table for user-mode accesses.  In SH-4a architectures, where we run
 * in 32-bit mode, the P4 address and the physical address are the same, so we can
 * use this value directly. 
 */
extern paddr_t sh_mmr_tmu_base_address;

#define VM_USER_SPACE_BOUNDRY	0x7bff0000 	/* U0 */
#define VM_KERN_SPACE_BOUNDRY	0xcfffffff	/* P1 + P2 + half P3 */
#define VM_USER_SPACE_SIZE 		0x7bff0000
#define VM_KERN_SPACE_SIZE		0x4fffffff

// might in use for msg buffer in non-continuous or colored v addr space
#define VM_MSG_MAX_SIZE		0x10000000
#define VM_MSG_XFER_END		0xe0000000	/* P3 */	
#define VM_MSG_XFER_START	(VM_MSG_XFER_END-VM_MSG_MAX_SIZE)

#define VM_FAULT_INKERNEL	0x80000000
#define VM_FAULT_WRITE		0x40000000

#define	VM_ASID_BOUNDARY	0xff
#define	VM_ASID_MASK		0xff

// NOTE: SYSP_ADDCOLOR is only valid after colour_mask_shifted has been
// assigned.  On most CPUS this happens in vmm_init_mem() phase 0 when
// pa_init is called, on SH we also define it in cpu_syspage_init since
// we need colour support to set up the user-space syspage and cpupage
// pointers.
extern unsigned colour_mask_shifted;  // defined in memmgr/mm_internal.h, but used in ker
#define SYSP_COLOR_MASK colour_mask_shifted
#define SYSP_SIZECOLOR  (SYSP_COLOR_MASK+__PAGESIZE)
#define SYSP_GETCOLOR(x)        ((uint32_t)x&SYSP_COLOR_MASK)
#define SYSP_ADDCOLOR(addr,color) ((SYSP_GETCOLOR(addr)>color)?((addr+SYSP_SIZECOLOR)&~SYSP_COLOR_MASK)|color :(addr&~SYSP_COLOR_MASK)|color)

/* registers that need to be specially saved during signal handling */
struct cpu_sigsave {
	shint	gr0;
	shint	gr4;
	shint	sr;
};

/* registers that need to be saved for interrupt routine invocation */
struct cpu_intrsave {
	int dummy;
};

/* Breakpoint fields that this cpu may need for breakpoints */
#define CPU_DEBUG_BRKPT		\
	paddr_t		paddr;		\
	union {					\
		uint16_t	ins;	\
		uint8_t		value[4];\
	}			data;

/* Fields this cpu may need for debugging a process */
#define CPU_DEBUG	\
	BREAKPT		step;

/* CPU specific fields in the thread entry structure */
struct cpu_thread_entry {
	SH_PERFREGS		*pcr;
};

/* CPU specific fields for handling a page fault */
struct cpu_fault_info {
	unsigned		flags;
};

/* Extra state (read only) made available by the kernel debugger */
struct cpu_extra_state {
};

extern void copy_vm_code(void);


struct mm_aspace;
void	sh4_update_tlb(uintptr_t vaddr, _Uint32t ptel);
void	tlb_flush_va(struct mm_aspace *adp, uintptr_t start, uintptr_t end);


#endif /* _KERNEL_CPU_SH_H */

/* __SRCVERSION("cpu_sh.h $Rev: 198837 $"); */
