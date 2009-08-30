#ifndef __KDBGCPU_H__
#define __KDBGCPU_H__

#include <sh/cpu.h>
#include <sh/inline.h>
#include <sys/mman.h>

typedef uint16_t			break_opcode;

#define	KDEBUG_STACK_SIZE	0x1000

#undef OPCODE_BREAK			
#define OPCODE_BREAK		0xfffd
#define ROUNDPG(p)			p

/*
 Define a structure used for communication to a GDB client
 */
/*
typedef struct {
	shint		gr[16];
	shfloat		fpr_bank0[16];
	shfloat		fpr_bank1[16];
	_uint32		fpul;
	_uint32		fpscr;
	shint		sr;
	shint		pc;
	shint		dbr;
	shint		gbr;
	shint		mach;
	shint		macl;
	shint		pr;
} gdb_register_context;
*/

typedef struct {
	shint		gr[16];

	shint		pc;
	shint		pr;
	shint		gbr;
	shint		vbr; /* syspage exceptptr */
	shint		mach;
	shint		macl;
	shint		sr;

	_uint32		fpul;
	_uint32		fpscr;
	shfloat		fpr_bank0[16];

	shint		ssr; /* not needed? */
	shint		spc; /* not needed? */

	shint		rb0[8]; /* not needed? */
	shint		rb1[8]; /* not needed? */

} gdb_register_context;
extern unsigned sh_family;

#define PADDR_TO_VADDR(x)	((void *)SH_PHYS_TO_P1(x))

#endif
