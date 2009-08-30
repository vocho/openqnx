#ifndef __KDBGCPU_H__
#define __KDBGCPU_H__

#define NUM_ELTS( array )	(sizeof(array) / sizeof( array[0] ))

#include <ppc/cpu.h>
#include <ppc/inline.h>
#include <sys/mman.h>

typedef uint32_t			break_opcode;

#define	KDEBUG_STACK_SIZE	0x1000

#undef OPCODE_BREAK			// @@@ NEED TO FIX mips/cpu.h
#define OPCODE_BREAK		0x7fe00008
#define ROUNDPG(p)			p

/*
 Define a structure used for communication to a GDB client
 */
typedef struct {
	uint32_t	gpr[32];
	double		fpr[32];
	uint32_t	pc;
	uint32_t	ps;
	uint32_t	cnd;
	uint32_t	lr;
	uint32_t	cnt;
	uint32_t	xer;
	uint32_t	mq;
} gdb_register_context;

typedef struct {
	uint32_t	cpu_type;
	uint32_t	gpr[32];
	uint32_t	pc;
	uint32_t	ps;
	uint32_t	cnd;
	uint32_t	lr;
	uint32_t	cnt;
	uint32_t	xer;
	uint32_t	mq;
} protocol1_gdb_register_context;

struct trap_entry {
	unsigned		trap_index;
	void			(*func)();
};

void install_traps(const struct trap_entry *trp, unsigned num);

extern void exc_alignment();

enum {
	FAM_DBG_ENTRY,
	FAM_DBG_EXIT_CONTINUE,
	FAM_DBG_EXIT_STEP
};
	

extern int	family_init_400(unsigned pvr);
extern int	family_init_600(unsigned pvr);
extern int	family_init_800(unsigned pvr);
extern int	family_init_booke(unsigned pvr);
extern int	family_init_900(unsigned pvr);
extern void		(*family_stuff)(int type, CPU_REGISTERS *ctx);

extern unsigned	msr_bits_off;

#define PADDR_TO_VADDR(x)	((void *)(x))

extern void (*icache_flusher)(uintptr_t, uintptr_t, unsigned);

#define CPU_CACHE_FLUSH(m, v, l)	icache_flusher((uintptr_t)(m), (uintptr_t)(v), (l))
#endif
