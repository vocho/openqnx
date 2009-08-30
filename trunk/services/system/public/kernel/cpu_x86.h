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

#include <x86/v86.h>
#include <x86/cpu.h>
#include <x86/inline.h>
#include <x86/priv.h>
#include <x86/smpxchg.h>

#define FAULT_ISWRITE(_n)	((_n) & X86_FAULT_WRITE)

typedef X86_CPU_REGISTERS	CPU_REGISTERS;
typedef X86_FPU_REGISTERS	FPU_REGISTERS;
typedef X86_ALT_REGISTERS	ALT_REGISTERS;
typedef X86_PERFREGS		PERF_REGISTERS;
#define CPU_STRINGNAME		X86_STRINGNAME

#define STARTUP_STACK_NBYTES		4096 //	increase for -O0 kdeb build 2048
#define KER_STACK_NBYTES			8192
#define IDLE_STACK_NBYTES			256
#define DEF_MAX_THREAD_STACKSIZE	4096

#define DEF_VIRTUAL_THREAD_STACKSIZE		131072
#define DEF_PHYSICAL_THREAD_STACKSIZE		4096
#define DEF_VIRTUAL_FIRST_THREAD_STACKSIZE	524288
#define DEF_PHYSICAL_FIRST_THREAD_STACKSIZE	4096

#define STACK_ALIGNMENT						4
#define STACK_INITIAL_CALL_CONVENTION_USAGE	0

#define CPU_P2V(p)				((uintptr_t)(p))
#define CPU_P2V_NOCACHE(p)		CPU_P2V(p)
#define CPU_V2P(v)				((paddr_t)(uintptr_t)(v))
#define CPU_VADDR_IN_RANGE(v)	1

#define VM_USER_SPACE_BOUNDRY		0xbfffffff
#define VM_KERN_SPACE_BOUNDRY		0xffffffff

/* registers that need to be saved for interrupt routine invocation */
struct cpu_intrsave {
	char	dummy;
};

/* Breakpoint fields that this cpu may need for breakpoints */
#define CPU_DEBUG_BRKPT		\
	int				hwreg;	\
	uint8_t			old[4];

/* Fields this cpu may need for debugging a process */
#undef CPU_DEBUG

/* CPU specific fields in the thread entry structure */
struct cpu_thread_entry {
	X86_PERFREGS	*pcr;
	//nmi_overflow must be last thing in structure
	uint32_t		nmi_overflow[3];
};

/* cpu specific information for memmgr fault handling */
struct cpu_fault_info {
	unsigned	code;
};

/* Extra state (read only) made available by the kernel debugger */
struct cpu_extra_state {
	uint32_t		cr[5];
};


void	v86_mark_running(unsigned on);

/* __SRCVERSION("cpu_x86.h $Rev: 202117 $"); */
