#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/kdebug.h>
#include <sys/syspage.h>
#include <sys/kdump.h>
#include <sys/mman.h>
#include <sys/elf.h>
#include <kernel/nto.h>
#include <kernel/kdutil.h>

#define TRUNC(_x,_a)	((unsigned)(_x)&((_a)-1))
#define ROUND(_x,_a)	(((unsigned)(_x)+((_a)-1)) & ~((_a)-1))
#define ALIGN(_x) 		ROUND(_x, sizeof(unsigned))

#define WRITE_CMD_ADDR	((void *)-(uintptr_t)1)
#define WRITE_INIT		0
#define WRITE_FINI		1
struct writer {
	char		*name;
	void		(*func)(void *, unsigned);
};

	
/*
	Prototypes
*/

void		kprintf_init(void);

void		fault_init(void);

void		dump_system(unsigned sigcode, void *ctx, void (*write_data)(void *, unsigned));

void		compress_init(void);
void		compress_start(void (*)(void *, unsigned));
void		compress_write(void *, unsigned);

void		cpu_elf_header(void *);
void		cpu_note(struct kdump_note *);
void		cpu_walk_extra_pmem(void (*func)(paddr_t, paddr_t, int, void *), void *data);


/*
	Globals
*/

extern int 							debug_flag;
extern int 							interesting_faults;
extern struct system_private_entry	*private;
extern struct cpupage_entry			*_cpupage_ptr;
extern struct kdump					*dip;
extern struct writer				*writer;
extern struct writer				available_writers[];
extern unsigned						__cpu_flags;
