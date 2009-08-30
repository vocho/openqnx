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


// Routines that are identical between e500 and e500v2
#define flush_vaddr_e500v2 	flush_vaddr_e500
#define mmu_on_e500v2 	mmu_on_e500
#define mmu_off_e500v2 	mmu_off_e500


#define EXTERN_BLOCK(which)											\
	extern void		flush_all_##which(void);						\
	extern void		flush_vaddr_##which(uintptr_t, unsigned);		\
	extern void		flush_asid_##which(unsigned);					\
	extern void		write_entry_##which(int, int, const ppcbke_tlb_t *);\
	extern void		read_entry_##which(int, int, ppcbke_tlb_t *);	\
	extern int		read_raw_##which(int, int, void *);				\
	extern const struct exc_copy_block imiss_##which;				\
	extern const struct exc_copy_block dmiss_##which;				\
	extern const struct exc_copy_block mmu_on_##which;				\
	extern const struct exc_copy_block mmu_off_##which;


EXTERN_BLOCK(e500)
EXTERN_BLOCK(e500v2)
EXTERN_BLOCK(ibm)

struct tlbops				tlbop;
struct ppc_tlbinfo_entry	*tlbinfo;
unsigned					num_tlb;
uint8_t						perm_tlb_mapping[32*4*4]; // entries are 3 bytes on ibm & 4 on e500
unsigned					rand_tlb_num_entries;

#if 0
void
dump_entry(int array, int idx) {
	ppcbke_tlb_t	tlb;

	tlbop.read_entry(array, idx, &tlb);
	kprintf("array/idx:%d/%02d v=%d epn=%x tid=%d ts=%d size=%d\n", array, idx, tlb.v, tlb.epn, tlb.tid, tlb.ts, tlb.size);
	kprintf("       rpn=%08x%08x attr=%x access=%x\n", (unsigned)(tlb.rpn >> 32), (unsigned)tlb.rpn, tlb.attr, tlb.access);
}
#endif


struct tlbselect {
	struct tlbops	op;
	int				(*read_raw)(int tlb, int idx, void *entry);
	const struct exc_copy_block	*imiss;
	const struct exc_copy_block	*dmiss;
	const struct exc_copy_block	*mmu_on;
	const struct exc_copy_block	*mmu_off;
};


#define DEFN_BLOCK(which)	\
	{	{flush_all_##which,	 flush_vaddr_##which, flush_asid_##which, \
			write_entry_##which, read_entry_##which},		\
		read_raw_##which,	\
		&imiss_##which,		\
		&dmiss_##which,		\
		&mmu_on_##which,	\
		&mmu_off_##which	\
	}
	
// NOTE: the order of elements in this array initializers is dependent on 
// the values of the PPCBKE_TLB_SELECT_* enums in ppc/kercpu.h...
static const struct tlbselect	tlbrtn_array[] = {
	DEFN_BLOCK(e500),
	DEFN_BLOCK(e500v2),
	DEFN_BLOCK(ibm)
};


static void
set_miss_hdlr(unsigned miss_spr, unsigned fault_vector, const struct exc_copy_block *miss) {
	uint32_t				*vector;
	uint32_t				*ins;

	// need to force exc_vector_address() to give us a new location
	set_spr_indirect(miss_spr, 0);

	vector = exc_vector_address(miss_spr, miss->size);
	copy_code(vector, miss);

	// Make the "ba" instruction at the end go to the fault handler
	ins = (void *)((uintptr_t)vector + miss->size - sizeof(uint32_t));
	*ins |= fault_vector;
	icache_flush((uintptr_t)ins);
}

/*
 * Initialise TLB structures:
 * - covering TLB used for TS=1
 * - permanent mapping entries (for cpu0 only)
 * - random reload array
 */
void
ppcbke_tlb_init(uint8_t *perm)
{
	const struct tlbselect		*tlbsel = &tlbrtn_array[ppcbke_tlb_select];
	unsigned					i;
	unsigned					j;
	unsigned					num;
	unsigned					covering_tlb = 0;
	ppcbke_tlb_t				tlb;
	uint8_t						*p;

	tlbinfo = SYSPAGE_CPU_ENTRY(ppc, tlbinfo);
	num = _syspage_ptr->un.ppc.tlbinfo.entry_size / sizeof(*tlbinfo);
	num_tlb = num;
	for(i = 0; i < num; ++i) {
		for(j = 0; j < tlbinfo[i].num_entries; ++j) {
			tlbop.read_entry(i, j, &tlb);
			if(tlb.v) {
				if(tlb.epn == 0) {
					covering_tlb = i;
				} else if (perm) {
					// Save the TLB entries in their native format so
					// that it's easier to load them up when we need them.
					perm += tlbsel->read_raw(i, j, perm);
				}
			}
		}
	}

	// Create a translation mapping for the first 256M of memory with
	// TS==1. This is only made valid while code in procnto is running.
	tlbop.read_entry(covering_tlb, 0, &tlb);
	tlb.ts = 1;
	tlbop.write_entry(covering_tlb, 1, &tlb);

	// Set up random reload array 
	rand_tlb_num_entries = tlbinfo[covering_tlb].num_entries;
	p = (uint8_t *)(PPCBKE_RANDOM_BASE + (RUNCPU << PPCBKE_RANDOM_SHIFT));
	j = 0;
	for(i = 0; i < PPCBKE_RANDOM_SIZE; ++i) {
		*p++ = j;
		if(++j >= rand_tlb_num_entries) j = 0;
	}
	tlbop_set_wired(2);
}

void
get_mmu_code(const struct exc_copy_block **on, const struct exc_copy_block **off) {
	const struct tlbselect		*tlbsel = &tlbrtn_array[ppcbke_tlb_select];

	*on  = tlbsel->mmu_on;
	*off = tlbsel->mmu_off;

	// Do all the TLB initialzation code here rather than in copy_vm_code()
	// like we used to. This way the mmu_on/mmu_off code fragments will be
	// invokable sooner. Otherwise, there might be a huge gap of kernel
	// initialization that we won't be able to debug.

	tlbop = tlbsel->op;
	tlb_flush_all = tlbop.flush_all;

	ppcbke_tlb_init(&perm_tlb_mapping[0]);

	set_miss_hdlr(PPCBKE_SPR_IVOR13, get_spr(PPCBKE_SPR_IVOR2), tlbsel->dmiss);
	set_miss_hdlr(PPCBKE_SPR_IVOR14, get_spr(PPCBKE_SPR_IVOR3), tlbsel->imiss);
}


void
copy_vm_code() {
	// This routine is just used by the mem_virtual.c file to cause this
	// file to be linked into procnto and get the get_mmu_code() function
	// defined.
}


void
set_l1pagetable(void *tbl, unsigned asid) {
	set_spr(PPCBKE_SPR_L1PAGETABLE_WR, (uintptr_t)tbl);
	set_spr(PPCBKE_SPR_PID, asid);
	ppc_isync();
}


void *
get_l1pagetable(void) {
	return ((uint32_t **)get_spr(PPCBKE_SPR_L1PAGETABLE_RD));
}


void
tlbop_set_wired(unsigned n) {
	uint8_t		*p;
	unsigned	old;
	unsigned	i;
	unsigned	j;

	p = (uint8_t *)(PPCBKE_RANDOM_BASE + (RUNCPU << PPCBKE_RANDOM_SHIFT));
	old = *p;
	if(old != n) {
		if(old > n) {
			// clear out entries
			j = n;
			for(i = n; i < PPCBKE_RANDOM_SIZE; ++i) {
				p[i] = j;
				++j;
				if(j >= old) {
					j = n;
					i += rand_tlb_num_entries - old;
				}
			}
		}
		j = 0;
		for(i = 0; i < PPCBKE_RANDOM_SIZE; ++i) {
			p[i] = j + n;
			++j;
			if(j >= n) {
				j = 0;
				i += rand_tlb_num_entries - n;
			}
		}
	}
}


unsigned
tlbop_get_wired(void) {
	return *(uint8_t *)(PPCBKE_RANDOM_BASE + (RUNCPU << PPCBKE_RANDOM_SHIFT));
}

__SRCVERSION("init_vm.c $Rev: 199396 $");
