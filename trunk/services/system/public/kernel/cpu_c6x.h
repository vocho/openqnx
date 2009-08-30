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

#include <c6x/cpu.h>
#include <c6x/priv.h>
#include <c6x/smpxchg.h>

#define FAULT_ISWRITE(_n)	(_n)

typedef C6X_CPU_REGISTERS	CPU_REGISTERS;
typedef C6X_FPU_REGISTERS	FPU_REGISTERS;
#define CPU_STRINGNAME		C6X_STRINGNAME

#define STARTUP_STACK_NBYTES		4096
#define KER_STACK_NBYTES			4096
#define IDLE_STACK_NBYTES			512
#define DEF_MAX_THREAD_STACKSIZE	32768

#define DEF_VIRTUAL_THREAD_STACKSIZE		DEF_MAX_THREAD_STACKSIZE
#define DEF_PHYSICAL_THREAD_STACKSIZE		DEF_MAX_THREAD_STACKSIZE
#define DEF_VIRTUAL_FIRST_THREAD_STACKSIZE	DEF_MAX_THREAD_STACKSIZE
#define DEF_PHYSICAL_FIRST_THREAD_STACKSIZE DEF_MAX_THREAD_STACKSIZE

#define STACK_ALIGNMENT						4
#define STACK_INITIAL_CALL_CONVENTION_USAGE	0

#define VM_USER_SPACE_BOUNDRY		0xefffffff
#define VM_KERN_SPACE_BOUNDRY		0xffffffff

/* registers that need to be saved for interrupt routine invocation */
struct cpu_intrsave {
	uint32_t		CSR;
};

/* Breakpoint fields that this cpu may need for breakpoints */
#undef CPU_DEBUG_BRKPT

/* Fields this cpu may need for debugging a process */
#undef CPU_DEBUG

/* Extra state (read only) made available by the kernel debugger */
struct cpu_extra_state {
	char	dummy;
};

/* __SRCVERSION("cpu_c6x.h $Rev: 153052 $"); */
