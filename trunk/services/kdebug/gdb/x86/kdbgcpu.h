#include <x86/cpu.h>
#include <x86/priv.h>
#include <x86/inline.h>

typedef uint8_t				break_opcode;

typedef struct x86_cpu		wat_cpu;
typedef struct x86_fpu		wat_fpu;

/*
 Define a structure used for communication to a GDB client
 the register order must line up with tm-i386v.h
  EAX, ECX, EDX, EBX, UESP, EBP, ESI, EDI, EIP, EFL, CS, SS, DS, ES, FS, GS,
*/
typedef struct {
	int	eax, ecx, edx, ebx, uesp, ebp, esi, edi, eip, efl;
	int cs, ss, ds, es, fs, gs;
} gdb_register_context;

extern short unsigned	ker_cs;
extern short unsigned	ker_ds;
extern short unsigned	__ds, __es, __fs, __gs;

#define		KDEBUG_STACK_SIZE	4000

#define PARASIZE		16
#define PAGESIZE		4096

#define DBREG_EX	0
#define DBREG_WR	1
#define DBREG_IO	2	// Pentium only
#define DBREG_RW	3

#define OPCODE_BREAK		0xcc

#define ROUNDPG(_x)		ROUND((_x), PAGESIZE)
#define ROUNDPA(_x)		ROUND((_x), PARASIZE)
#define TRUNCPG(_x)		TRUNC((_x), PAGESIZE)

// We'd need to establish new mappings for this to actually work
#define PADDR_TO_VADDR(x)	(void *)0
