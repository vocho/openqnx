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

#include "externs.h"
#include "apm.h"

#ifndef EXTERN
	#define EXTERN	extern
#endif

//
// General utility macros 
//
#define VA_INVALID	((uintptr_t)-1)

#define ADDR_OFFSET_BITS	12
#define ADDR_OFFSET_MASK	((1 << ADDR_OFFSET_BITS)-1)
#define ADDR_OFFSET(a)		((uintptr_t)(a) & ADDR_OFFSET_MASK)
#define ADDR_PAGE(a)		((a) & ~ADDR_OFFSET_MASK)

#if (__PAGESIZE-1) != ADDR_OFFSET_MASK
	#error __PAGESIZE and ADDR_OFFSET_BITS do not agree
#endif

#if !defined(_PADDR_BITS)
	#define _PADDR_BITS 32
#endif

#if _PADDR_BITS > 32
	#define HIGH_PADDR(paddr) ((paddr) >> 32)
#else
	#define HIGH_PADDR(paddr) (0)
#endif
#define LOW_PADDR(paddr) ((uint32_t)(paddr))

#define VPS_NONE		0
#define VPS_AUTO		1
#define VPS_HIGHUSAGE	2

#include "cpu_mm_internal.h"

//RUSH3: Go through these and see what macros are still needed after
//RUSH3: done recoding.
#if CPU_SYSTEM_HAVE_COLOURS
	#define COLOUR_VA(va)			(((va) >> ADDR_OFFSET_BITS) & colour_mask)
	#define COLOUR_PA(pa)			COLOUR_VA((unsigned)(pa))

	/* Adjusts the vaddr forward to the appropriate colour */
	#define COLOUR_ADJUST_VA(addr,addr_colour) \
			((addr) + (((addr_colour) - (addr)) & colour_mask_shifted))

	/* Adjusts the paddr backwards to the appropriate colour _if_ it doesn't 
	   match already. (don't bother if we're ignoring colour) */
	#define COLOUR_ADJUST_PA(paddr,paddr_colour) \
			( (((paddr_colour)>> ADDR_OFFSET_BITS) == PAQ_COLOUR_NONE) || ( (COLOUR_PA((paddr)) == (COLOUR_PA(paddr_colour))))) ? \
				(paddr) : ((COLOUR_ADJUST_VA((paddr),(paddr_colour))) - ((colour_mask+1) << ADDR_OFFSET_BITS))

#else
	#define COLOUR_VA(va)			0
	#define COLOUR_PA(va)			0
	#define COLOUR_ADJUST_VA(addr,addr_colour) (addr)
	#define COLOUR_ADJUST_PA(paddr,paddr_colour) (paddr)
#endif

// 
// Flags for memmgr.validate()
//
#define VV_RANGE		0x1
#define VV_MAPPED		0x2


struct _memmgr_rwlock {
	uint32_t	lock;
};

struct vaddr_range {
	uintptr_t			lo;
	uintptr_t			hi;
};

#define MM_FLAG_BACKWARDS_COMPAT 		0x0001
#define MM_FLAG_MULTIPLE_DCACHE_LEVELS	0x0002
#define MM_FLAG_ENFORCE_ALIGNMENT		0x0004
#define MM_FLAG_LOCKALL					0x0008
#define MM_FLAG_SUPERLOCKALL			0x0010
#define MM_FLAG_VPS						0x0020
#define MM_FLAG_PADDR64_SAFE_SYS		0x0040

EXTERN struct pa_restrict	*restrict_proc;
EXTERN struct pa_restrict	*restrict_user;
EXTERN struct pa_restrict	*restrict_user_paddr64;
//RUSH3: Need both colour_mask and colour_mask_shifted?
EXTERN unsigned				colour_mask;
EXTERN unsigned				colour_mask_shifted;
EXTERN paddr_t				last_paddr;
EXTERN uintptr_t			va_rover;
EXTERN unsigned				mm_flags;
EXTERN unsigned				address_cookie;
EXTERN struct vaddr_range	system_heap_vaddr;
EXTERN uintptr_t			pgszlist[sizeof(uintptr_t)*8+1];

#include "pa.h"

void				pa_init(unsigned colours);
void				pa_start_background(void);


/* Physical Allocation flags (input) */
#define PAA_FLAG_CONTIG		0x01
#define PAA_FLAG_NOX64K		0x02
#define PAA_FLAG_HIGHUSAGE	0x04

/* Physical Allocation Status flags (output) */
#define PAA_STATUS_NOT_ZEROED		0x01
#define PAA_STATUS_COLOUR_MISMATCH	0x02
#define PAA_STATUS_ISCONTIG			0x04

struct pa_quantum	*pa_alloc(paddr_t size, paddr_t align, unsigned colour,
						unsigned flags, unsigned *status,
						struct pa_restrict *restriction, paddr_t resv_size);
int					pa_alloc_given(struct pa_quantum *, unsigned, unsigned *);
void				pa_free(struct pa_quantum *paq, unsigned num, paddr_t resv_size);
void				pa_free_list(struct pa_quantum *paq, paddr_t resv_size);
#if 1
/* see pa.c:pa_free_paddr() for an explanation for this change */
void				pa_free_paddr(paddr_t p, paddr_t s, paddr_t resv_size);
#else	/* pa_free_paddr() bug */
#define 			pa_free_paddr(p, s, r)	\
						pa_free(pa_paddr_to_quantum(p), LEN_TO_NQUANTUM(s), r)
#endif	/* pa_free_paddr() bug */
struct pa_quantum	*pa_alloc_fake(paddr_t paddr, paddr_t size);
void				pa_free_fake(struct pa_quantum *paq);
void				pa_free_info(struct pa_restrict *restriction, paddr_t *total, paddr_t *contig);
int					pa_sysram_overlap(struct pa_restrict *restriction);
int					pa_pmem_add(paddr_t start, paddr_t len);

paddr_t				pa_quantum_to_paddr(struct pa_quantum *pq);
struct pa_quantum	*pa_paddr_to_quantum(paddr_t paddr);

memsize_t				pa_free_size(void);
memsize_t				pa_used_size(void);
memsize_t				pa_reserved_size(void);

extern uintptr_t	cpu_sysvaddr_find(uintptr_t, size_t);
extern void 		*cpu_early_paddr_to_vaddr(paddr_t paddr, unsigned size, paddr_t *l2mem);
extern unsigned		cpu_whitewash(struct pa_quantum *paq);

extern int 			shmem_create(OBJECT *, void *);
extern int 			shmem_done(OBJECT *);

extern struct pa_quantum	*tymem_rdb_alloc(OBJECT *obp, size_t size, unsigned flags);
extern struct pa_quantum	*tymem_rdb_alloc_given(OBJECT *obp, paddr_t paddr, size_t size, int *r);
extern void					tymem_rdb_free(OBJECT *obp, paddr_t paddr, size_t size);
extern void					tymem_free_info(OBJECT *obp, paddr_t *total, paddr_t *contig);

extern void		rdecl memobj_lock(OBJECT *);
extern int		rdecl memobj_cond_lock(OBJECT *);
extern void		rdecl memobj_unlock(OBJECT *);


// The definition for struct mm_object is in "pathmgr/pathmgr_object.h", since
// I couldn't figure a clean way of separating the fields required
// by the pathmgr and those required by the memmgr. Sigh.

#include "pathmgr/pathmgr_object.h"

/*
 * Flags for memory objects
 */
//Global flags
#define MM_MEM_HAS_SYNC				0x80000000
//#define unused					0x40000000
#define MM_MEM_SKIPLOCKCHECK		0x20000000
//#define unused					0x10000000
#define MM_MEM_RDB					0x08000000
#define MM_MEM_HAS_BAD_PAGE			0x04000000
#define MM_MEM_COLOUR_FLUSH			0x02000000
	/* MM_MEM_COLOUR_FLUSH indicates the memory object is referenced
	 * through two or more virtual addresses with different colours.
	 * The code must be careful to flush the cache when appropriate
	 * during loading and mapping.
	 */

//Shared memory flags
//reuse SHMCTL_FLAG_* bits
#define MM_SHMEM_SPECIAL			(SHMCTL_FLAG_MASK+1)
#define MM_SHMEM_IFS				(MM_SHMEM_SPECIAL << 1)

//mmap'd file flags
#define MM_FDMEM_STICKY				0x00000001
//#define unused					0x00000002
//#define unused					0x00000004
#define MM_FDMEM_NEEDSSYNC			0x00000008

//anonymous, typed memory flags
//reuse POSIX_TYPED_MEM_* bits
#define MM_ANMEM_TYPED_MASK			0x0000000f 
#define MM_ANMEM_MULTI_REFS			0x00000010 

#define VERIFY_OBJ_LOCK(o)	CRASHCHECK(!((o)->mem.mm.flags & MM_MEM_SKIPLOCKCHECK) && !proc_mux_haslock(&(o)->mem.mm.mux, 0))


/* __SRCVERSION("mm_internal.h $Rev: 207313 $"); */
