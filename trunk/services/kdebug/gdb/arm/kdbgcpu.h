#ifndef	__KDBGCPU_H__
#define	__KDBGCPU_H__

#include <sys/mman.h>
#include <arm/cpu.h>
#include <arm/mmu.h>

typedef uint32_t	break_opcode;

/*
 * WARNING: this must match kernel and gdb definitions
 */
#define	OPCODE_BREAK		0xe7ffdefe

#define	KDEBUG_STACK_SIZE	0x1000

//#define	DEBUG_GDB

/*
 * This corresponds to the register format in gdb-4.17
 *
 * FIXME: we don't support the FPU stuff yet.
 *		  It corresponds to the FPA registers, which are ARMv3
 *		  only. We'll need to revisit this once we decide on
 *		  how to provide floating point support.
 */
typedef struct {
	unsigned		gpr[16];		// general purpose registers
	char			fpu[12*8];		// FPU registers
	char			fps[4];			// FPU status
	unsigned		ps;				// processor status register
} gdb_register_context;

extern ptp_t	*L1_table;

// We'd need to establish new mappings for this to actually work
#define PADDR_TO_VADDR(x)	(void *)0

#endif
