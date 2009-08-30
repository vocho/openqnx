#include <sys/types.h>

struct cpu_extra_state;
struct syspage_entry;
struct asinfo_entry;

void		cpu_init_extra(struct cpu_extra_state *);
void		cpu_save_extra(struct cpu_extra_state *);
void		cpu_restore_extra(struct cpu_extra_state *);

void		cpu_init_map(void);
void		*cpu_map(paddr64_t paddr, unsigned size, unsigned prot, int colour, unsigned *mapped_size);
void		cpu_unmap(void *p, unsigned size);
unsigned	cpu_vaddrinfo(void *p, paddr64_t *paddr, unsigned *len);

unsigned	current_cpunum(void);

paddr64_t	alloc_pmem(unsigned size, unsigned align);
int 		walk_asinfo(const char *name, int (*func)(struct asinfo_entry *, char *, void *), void *data);
void		kprintf_setup(void (*oc)(struct syspage_entry *, char), unsigned ps);
void		startnext(uintptr_t next);
uintptr_t	next_bootstrap(void);

void		async_check_init(unsigned channel);
int			async_check_active(void);

extern struct system_private_entry	*private;
