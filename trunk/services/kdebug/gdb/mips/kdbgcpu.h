#ifndef __KDBGCPU_H__
#define __KDBGCPU_H__

#include <mips/cpu.h>
#include <mips/opcode.h>
#include <mips/priv.h>
#include <mips/inline.h>
#include <sys/mman.h>

typedef uint32_t			break_opcode;

#define	KDEBUG_STACK_SIZE		0x1000

#define MIPS_R4K_HI_ADDR_MASK	0xF0000000

#define	REAL_VADDR				0x01000000
#define ROUNDPG(p)				p


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
} gdb_register_context;

#define PADDR_TO_VADDR(x)	(void *)MIPS_PHYS_TO_KSEG0(x)

#endif
