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
 * Definitions that are common to ARMv4 and ARMv6 implementations
 */

// Low and high ranges of memory that the system (kernel/proc) can quickly
// get at.
#define CPU_SYSTEM_PADDR_START	0

// Whether the system can _only_ access paddrs in the above range.
#define CPU_SYSTEM_PADDR_MUST	0

// Whether the system supports multiple page sizes
#define CPU_SYSTEM_HAVE_MULTIPLE_PAGESIZES	VPS_AUTO

/*
 * The locations of physical memory is board specific so we
 * have to calculate these values at runtime
 */
#define CPU_SYSTEM_PADDR_END	cpu_system_paddr_end
#define	CPU_1TO1_VADDR_BIAS		cpu_1to1_vaddr_bias
#define	CPU_1TO1_IS_VADDR(v)	(cpu_1to1_present && ((unsigned)(v)) >= ARM_1TO1_BASE && ((unsigned)(v)) < (ARM_1TO1_BASE+ARM_1TO1_SIZE))
#define	CPU_1TO1_IS_PADDR(v)	(cpu_1to1_present && ((unsigned)(v)) >= cpu_system_paddr_start && ((unsigned)(v)) <= cpu_system_paddr_end)

extern paddr_t		cpu_system_paddr_start;
extern paddr_t		cpu_system_paddr_end;
extern unsigned		cpu_1to1_vaddr_bias;
extern int			cpu_1to1_present;

#include <arm/mmu.h>

typedef ptp_t			cpu_page_table;

extern ptp_t				*L1_table;
extern struct arm_cpu_entry	*arm_cpu;

#define	PDE_SPAN	(1 << 22)		// 4MB spanned by "page directory page"

#define	ARM_L1IDX(v)	((v) >> 20)

extern void			vx_capability_check(void);

/*
 * Pull in architecture specific definitions
 */
#include "vx_mm_internal.h"
