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

#include "mm_internal.h"

//
// Virtual memory manager data structures
//

struct mm_map_head {
	struct mm_map			*head;
	struct mm_map			*rover;
	volatile unsigned		lock;
	uintptr_t				start;
	uintptr_t				end;
	volatile unsigned		walk_gen;
};

struct map_set {
	struct mm_map_head	*head;
	struct mm_map		*first;
	struct mm_map		*last;
	struct mm_map		*prev;
	int					flags;
};

struct mm_object_ref {
	struct mm_object_ref	*next;
	struct mm_aspace		*adp;
	OBJECT					*obp;
	struct mm_map			*first_ref;
	int						fd;
};


#define EXTRA_FLAG_MADV_MASK	0x000f // bottom 4 bits are POSIX_MADV_*
#define EXTRA_FLAG_LOCK			0x0010
#define EXTRA_FLAG_RDONLY		0x0020
#define EXTRA_FLAG_LOADER		0x0040
#define EXTRA_FLAG_NOINHERIT	0x0080 // for minherit(), if we implement it
#define EXTRA_FLAG_SPECIAL		0x0100
#define EXTRA_FLAG_PRIMARY_STK	0x0200 // Can indicate with MAP_STACK & EXTRA_FLAG_LOADER if we need a bit
#define EXTRA_FLAG_CACHE_CLEANED 0x0400 // Only used with direct mappings
#define EXTRA_FLAG_GBL_VADDR	0x0800
#define EXTRA_FLAG_RLIMIT_DATA   0x1000 // include in rlimit accounting

struct mm_map {
	struct mm_map			*next;
	struct mm_object_ref	*obj_ref;
	off64_t					offset;
	struct {
		struct mm_map		*next;
		struct mm_map		**owner;
	}						ref;
	uintptr_t				start;
	uintptr_t				end;
	int						reloc;
	unsigned				mmap_flags;
	unsigned 				extra_flags;
	unsigned short			last_page_bss;
	uint8_t					spare;
	volatile uint8_t		inuse;
//If things get added, there is code that needs to be modified...
//Grep for "MAPFIELDS:" to find the places.
};


#define MM_ASFLAG_LOCKALL			0x00000001
#define MM_ASFLAG_ISR_LOCK			0x00000002
#define MM_ASFLAG_ISR_INPROGRESS	0x00000004
#define MM_ASFLAG_PRIVATIZING		0x00000008
#define MM_ASFLAG_PADDR64_SAFE		0x00000010

struct mm_aspace {
	struct mm_map_head			map; 
	struct  {
		uintptr_t		vmem;
		uintptr_t		data;
		uintptr_t		stack;
		uintptr_t		memlock;
		uintptr_t		rss;
	}							rlimit;
	OBJECT						*anon;
	struct _memmgr_rwlock		rwlock;
	unsigned					fault_owner;
	unsigned					flags;
	uintptr_t					tmap_base;
	size_t						tmap_size;
	struct cpu_mm_aspace		cpu;
};


#if _PADDR_BITS > 32
// Since we've got 40 bits of paddr, make the 'object' the high 8 bits.
#define PADDR_TO_SYNC_OBJ(addr)	((void *)((uintptr_t)((addr) >> 32) & 0xff))
#else
#define PADDR_TO_SYNC_OBJ(addr)	((void *)_syspage_ptr)
#endif
#define PADDR_TO_SYNC_OFF(addr)	((unsigned)(addr))
	

#ifndef CPU_ZERO_PAGE
    #define CPU_ZERO_PAGE(d, l, mm)	memset((d), 0, (l))
#endif

#ifndef CPU_CACHE_CONTROL
	#define CPU_CACHE_CONTROL(a, d, l, f)	CacheControl((d), (l), (f))
#endif

#ifndef CPU_FAULT_ON_WRITE_WORKS
	#define CPU_FAULT_ON_WRITE_WORKS	1
#endif

#ifndef	CPU_1TO1_FLUSH
	#define	CPU_1TO1_FLUSH(v, l)
#endif

extern int			cpu_vmm_mcreate(PROCESS *);
extern void			cpu_vmm_mdestroy(PROCESS *);
extern int			cpu_vmm_fault(struct fault_info *);
extern unsigned		cpu_vmm_vaddrinfo(PROCESS *, uintptr_t, paddr_t *, size_t *);


#define PTE_OP_MAP			0x0001
#define PTE_OP_PROT			0x0002
#define PTE_OP_UNMAP		0x0004
#define PTE_OP_PREALLOC		0x0008
#define PTE_OP_BAD			0x0010
#define PTE_OP_PREEMPT		0x0080
#define PTE_OP_MERGESTARTED	0x0100
#define PTE_OP_TEMP			0x0200
#define PTE_OP_FORCEMERGE	0x0400

struct mm_pte_manipulate {
	paddr_t				paddr;
	ADDRESS				*adp;
	uintptr_t			start;
	uintptr_t			end;
	uintptr_t			split_end;
	uintptr_t			first;
	unsigned			prot;
	unsigned			op;
	unsigned			special;
	unsigned			shmem_flags;
};

extern int		cpu_pte_split(uintptr_t split, struct mm_pte_manipulate *);
extern int		cpu_pte_manipulate(struct mm_pte_manipulate *);
extern int		cpu_pte_merge(struct mm_pte_manipulate *);

/*
 * definitions for cache colour operations on a page 
 */
#define COLOUR_CLEAN_PURGE		0
#define COLOUR_CLEAN_FLUSH		1
#define COLOUR_CLEAN_ZERO_FLUSH	2

void			cpu_colour_clean(struct pa_quantum *, int);
void			colour_set(uintptr_t, struct pa_quantum *, unsigned);

int				map_init(struct mm_map_head *, uintptr_t start, uintptr_t end);
int				map_fini(struct mm_map_head *);
int 			map_create(struct map_set *, struct map_set *, 
							struct mm_map_head *, uintptr_t va,
							uintptr_t size, uintptr_t mask, unsigned flags);
uintptr_t		map_find_va(struct mm_map_head *, uintptr_t va, uintptr_t size,
							uintptr_t mask, unsigned flags);
int				map_split(struct map_set *, size_t);
int				map_add(struct map_set *);
int				map_remove(struct map_set *);
int				map_coalese(struct map_set *);
int				map_destroy(struct map_set *);
void			map_set_update(struct map_set *, struct mm_map *, struct mm_map *);
int				map_fault_lock(struct mm_map_head *);
void 			map_fault_unlock(struct mm_map_head *);

int	rdecl		rlimit_blown(PROCESS *, int, size_t);

void			sysaddr_map(void *);

#define MI_NONE			0x00
#define MI_SPLIT		0x01
#define MI_NEXT			0x02
#define MI_SKIP_SPECIAL	0x04

int				map_isolate(struct map_set *, struct mm_map_head *, 
							uintptr_t start, size_t size, int flags);

extern int		pte_map(ADDRESS *, uintptr_t,  uintptr_t, int, OBJECT *, paddr_t, unsigned);
extern int		pte_prot(ADDRESS *, uintptr_t, uintptr_t, int, OBJECT *);
extern int		pte_unmap(ADDRESS *, uintptr_t, uintptr_t, OBJECT *);
extern int		pte_bad(ADDRESS *, uintptr_t);
extern int		pte_prealloc(ADDRESS *, uintptr_t,  uintptr_t);
extern int		pte_temp_map(ADDRESS *, uintptr_t, struct pa_quantum *, struct mm_map *, size_t, int (*func)(void *, size_t, void *), void *);

extern int		memref_add(struct mm_map *, OBJECT *, ADDRESS *, int fd);
extern void		memref_del(struct mm_map *);
extern int		memref_walk(OBJECT *, int (*)(struct mm_object_ref *, struct mm_map *, void *), void *);
extern void		memref_walk_restart(struct mm_map_head *);

extern off64_t	anmem_offset(OBJECT *, uintptr_t, size_t);
extern void		anmem_unoffset(OBJECT *, off64_t, size_t);

extern unsigned	memobj_colour(OBJECT *, off64_t);
extern int		memobj_pmem_del_len(OBJECT *, off64_t, uint64_t);
extern int		memobj_pmem_del_end(OBJECT *, off64_t, off64_t);
extern int		memobj_pmem_add(OBJECT *, off64_t, size_t, int);
extern int		memobj_pmem_add_pq(OBJECT *, off64_t, struct pa_quantum *);
extern int		memobj_pmem_split(OBJECT *obp, struct pa_quantum *pq, unsigned);
extern int		memobj_offset_check(OBJECT *, off64_t, uint64_t);

#define MPW_HOLES	0x01
#define MPW_PHYS	0x02
#define MPW_SYSRAM	0x04

extern int 		memobj_pmem_walk(int, OBJECT *, off64_t, off64_t, int (*func)(OBJECT *, off64_t, struct pa_quantum *, unsigned, void *), void *);
extern int 		memobj_pmem_walk_mm(int, struct mm_map *, int (*func)(OBJECT *, off64_t, struct pa_quantum *, unsigned, void *), void *);

// These defines have to be different from all the user visible unmap flags
#define UNMAP_PRIVATIZE	0x80000000
#define UNMAP_NORLIMIT	0x40000000

extern int		ms_unmap(ADDRESS *, struct map_set *, int);
extern int		ms_lock(ADDRESS *, struct map_set *);
extern int		mm_sync(OBJECT *, struct mm_map *, int);

#define MR_NONE		0x0000
#define MR_WRITE	0x0001
#define MR_NOINIT	0x0002
//define unused		0x0004
#define MR_TRUNC	0x0008

extern int		memory_reference(struct mm_map **, uintptr_t, uintptr_t, unsigned, struct map_set *);

extern void 	fault_init(void);
extern void 	lock_init(void);


#if defined(CPU_GBL_VADDR_START)
	extern void		gbl_vaddr_init(void);
	extern int		gbl_vaddr_unmap(uintptr_t, size_t);

	#define GBL_VADDR_INIT()		gbl_vaddr_init()
	#define	GBL_VADDR_UNMAP(v, s)	gbl_vaddr_unmap((v), (s))

	//RUSH3: ARMism
	extern int	cpu_gbl_mmap(ADDRESS *, uintptr_t, uintptr_t, unsigned);
	extern void	cpu_gbl_unmap(ADDRESS *, uintptr_t, uintptr_t, unsigned);
#else
	#define GBL_VADDR_INIT()
	#define	GBL_VADDR_UNMAP(v, s)	ENOTSUP
#endif

#undef MMF
#define MMF(r, f, p, e)		MMF_PROTO(r, f, p, e)
MM_FUNCS(vmm)

extern int			munmap_flags_default;
extern intrspin_t	map_spin;
extern struct vaddr_range	system_code_vaddr;
extern struct vaddr_range	system_data_vaddr;
extern uintptr_t			(*vaddr_search_adjust_hook)(uintptr_t vaddr, 
							ADDRESS *adp, unsigned flags, uintptr_t size);

#define PMEM_STATS_FREE     0x0
#define PMEM_STATS_ALLOCATE 0x1

extern void (*pmem_stats_hook)(OBJECT *obp, size_t size, unsigned type);


#define SHM_LAZY(o)	(((o)->hdr.type == OBJECT_MEM_SHARED) \
						&& ((o)->mem.mm.flags & SHMCTL_LAZY))

// We can save some time by not locking the anonymous object sometimes
// if there aren't multiple address spaces referencing it
// ("/proc/<pid>/as" mappings). However, there's a gotcha. With the
// debug version of procnto, the VERIFY_OBJ_LOCK() macros will start
// complaining that the object isn't locked. So, if we're building
// a debug version, we pretend that the anon object is always has
// multiple references, so it always gets locked and VERIFY_OBJ_LOCK()
// is happy
#ifdef NDEBUG
	#define ANMEM_MULTI_REFS(obp)	((obp)->mem.mm.flags & MM_ANMEM_MULTI_REFS)
#else
	#define ANMEM_MULTI_REFS(obp)	1
#endif

/* __SRCVERSION("vmm.h,v $Rev: 211761 $"); */
