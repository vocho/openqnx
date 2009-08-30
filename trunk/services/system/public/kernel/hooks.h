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
 * Memory manager
 *
 */

struct _procfs_map_info;
struct _procfs_debug_info;
struct _mem_debug_info;
struct fault_info;

#define MM_FUNCS(extra) \
	MMF(void,	configure,		(char *cfg), extra) \
	MMF(void,	init_mem,		(int phase), extra) \
	MMF(void,	aspace,			(PROCESS *prp, PROCESS **pactprp), extra)	\
	MMF(int,	fault,			(struct fault_info *), extra) \
	MMF(int,	map_xfer,		(PROCESS *actprp, PROCESS *prp, IOV **piov, int *pparts, int *poff, IOV *niov, int *pnparts, unsigned flags), extra) \
	MMF(int,	mcreate,		(PROCESS *prp), extra) \
	MMF(void,	mdestroy,		(PROCESS *prp), extra) \
	MMF(int,	dup,			(PROCESS *pprp, PROCESS *prp), extra) \
	MMF(int,	mmap,			(PROCESS *prp, uintptr_t addr, size_t len, int prot, int flags, OBJECT *obp, uint64_t off, unsigned align, unsigned prealloc, int fd, void **vaddr, size_t *size, MPART_ID mpid), extra) \
	MMF(int,	munmap,			(PROCESS *prp, uintptr_t addr, size_t len, int flags, MPART_ID mpid), extra) \
	MMF(int,	mprotect,		(PROCESS *prp, uintptr_t addr, size_t len, int prot), extra) \
	MMF(int,	msync,			(PROCESS *prp, uintptr_t addr, size_t len, int flags), extra) \
	MMF(int,	mlock,			(PROCESS *prp, uintptr_t addr, size_t len, int flags), extra) \
	MMF(int,	munlock,		(PROCESS *prp, uintptr_t addr, size_t len, int flags), extra) \
	MMF(unsigned,vaddrinfo,		(PROCESS *prp, uintptr_t vaddr, paddr_t *paddrp, size_t *lenp, unsigned flags), extra) \
	MMF(OBJECT *,vaddr_to_memobj,(PROCESS *prp, void *vaddr, unsigned *offset,int create), extra) \
	MMF(size_t,	mapinfo,		(PROCESS *prp, uintptr_t vaddr, struct _procfs_map_info *info, struct _procfs_debug_info *debug, size_t, OBJECT **, int *fd, size_t *contigp), extra) \
	MMF(int,	debuginfo,		(PROCESS *prp, struct _mem_debug_info *info), extra) \
	MMF(int,	resize,			(OBJECT *obp, size_t size), extra) \
	MMF(int,	memobj_phys,	(OBJECT *obp, paddr_t pa), extra) \
	MMF(int,	swapper,		(PROCESS *prp, unsigned *offset, size_t len, int action), extra) \
	MMF(int,	validate,		(PROCESS *prp, uintptr_t start, size_t len, int flags), extra) \
	MMF(int,	madvise,		(PROCESS *prp, uintptr_t start, size_t len, int flags), extra) \
	MMF(int,	pmem_add,		(paddr_t start, paddr_t len), extra)

struct memmgr_entry {
	unsigned		pagesize;
	unsigned		fault_pulse_code;
	unsigned		sizeof_address_entry;

	#undef MMF
	#define MMF(r, f, p, e)	r (*f)p;	
	MM_FUNCS(x)

	void			*reserved[2];
};

#define MMF_PROTO(r, f, p, e)	extern r e##_##f p;
#define MMF_DEFN(r, f, p, e)	e##_##f,

/*
 * Process manager
 */
struct procmgr_entry {
	void			(*process_threads_destroyed)(PROCESS *prp, struct sigevent *ev);
	int 			(*process_stop_or_cont)(PROCESS *prp, int signo, int sigcode, int sigval, int sender, struct sigevent *ev);
	int				(*process_start)(PROCESS *prp, uintptr_t *start_ip);
	int				(*process_coredump)(PROCESS *prp, struct sigevent *ev);
	int				process_stack_code;
	void			*reserved;
};

/*
 * Definitions for parameters
 */

/* flags for map_addr */
#define	MAPADDR_FLAGS_WRITE		0x1
#define	MAPADDR_FLAGS_SYSPRP	0x2
#define MAPADDR_FLAGS_NOTFIRST	0x4
#define	MAPADDR_FLAGS_IOVKERNEL	0x8000

/* __SRCVERSION("hooks.h $Rev: 156323 $"); */
