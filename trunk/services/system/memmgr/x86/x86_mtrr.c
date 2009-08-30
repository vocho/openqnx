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
#include "vmm.h"
#include <x86/cpu.h>

#define	MTRR_VALID				0x00000100
#define	MTRR_LOCKED				0x00000200
#define	MTRR_BIOS_SET			0x00000400
#define	MTRR_DEFAULT_TYPE_MASK	0x000000ff

struct _mtrr {
	unsigned			refcnt;
	unsigned			flags;
	uint64_t			phys_base;
	uint64_t			phys_mask;
};

static int				nvmtrr;
static struct _mtrr		*mtrr_array;

// Init the MTRR's on X86 if present
void 
x86_init_mtrr() {
	_uint64		cap;

	cap = rdmsr(X86_MSR_MTRRCAP);

	nvmtrr = cap & X86_MTRR_CAP_NVAR_MASK;

	if(nvmtrr) {
		struct _mtrr		*_mtrr;
		int					i;

		if((mtrr_array = _scalloc(nvmtrr * sizeof(struct _mtrr))) == NULL) {
			nvmtrr = 0;
			return;
		}
		_mtrr = mtrr_array;

		for(i = 0; i < nvmtrr; i++, _mtrr++) {
			_mtrr->phys_base = rdmsr(X86_MSR_MTRR_PHYSBASE0 + 2 * i);	
			_mtrr->phys_mask = rdmsr(X86_MSR_MTRR_PHYSMASK0 + 2 * i);	
			if(_mtrr->phys_mask & X86_MTRR_PHYSMASK_VALID) {
				_mtrr->flags = MTRR_VALID | MTRR_BIOS_SET | (_mtrr->phys_base & MTRR_DEFAULT_TYPE_MASK);
				if((_mtrr->phys_base & MTRR_DEFAULT_TYPE_MASK) == X86_MTRR_TYPE_WRITEBACK) {
					_mtrr->flags |= MTRR_LOCKED;
				}
				_mtrr->refcnt = 1;
			} else {
				_mtrr->refcnt = 0;
				_mtrr->flags = 0;
			}
		}
	}
	return;
}

static int 
_find_free_mtrr() {
	int 			i;

	for(i = 0; i < nvmtrr; i++) {
		if(!mtrr_array[i].flags) return i;
	}

	return -1;
}

static int 
_find_overlapping_mtrr(paddr_t start, unsigned size) {
	int 			i;
	uint64_t		base;
	uint64_t		len;

	for(i = 0; i < nvmtrr; i++) {
		if(mtrr_array[i].flags & MTRR_VALID) {
			base = mtrr_array[i].phys_base & X86_MTRR_PHYSBASE_BASE_MASK;
			len = 1 + ((~mtrr_array[i].phys_mask & X86_MTRR_PHYSMASK_RANGE_MASK)|0xfff);
			if((start >= base) && ((start + size) <= (base + len))) {
				return i;
			}
		}
	}

	return -1;
}

//
// Program the MTRR register
// This follows the intel-suggested algorithm, but will NOT work on SMP!
//

static void 
_write_mtrr(int index, struct _mtrr *_mtrr) {
	unsigned		cr0, cr0_val, cr4;
	_uint64			def_type;

	InterruptDisable();
	cr0 = rdcr0();	
	cr4 = rdcr4();	
	ldcr4(cr4 & ~0x80);
	cr0_val = cr0 | 0x40000000;
	flushcache();
	ldcr0(cr0_val);
	flushcache();
	def_type = rdmsr(X86_MSR_MTRR_DEFTYPE);
	wrmsr(X86_MSR_MTRR_DEFTYPE, def_type & ~X86_MTRR_DEFTYPE_E);

	wrmsr(X86_MSR_MTRR_PHYSBASE0 + 2*index, _mtrr->phys_base);
	wrmsr(X86_MSR_MTRR_PHYSMASK0 + 2*index, _mtrr->phys_mask);

	flushcache();
	wrmsr(X86_MSR_MTRR_DEFTYPE, def_type);
	ldcr0(cr0);
	ldcr4(cr4);
	InterruptEnable();

	return;
}

//
// Set special HW attributes for the physical address range.
// flags are special MEMOBJ flags, op is PTE_OP_MAP for adding, PTE_OP_UNMAP 
// for removing
//

int 
x86_set_mtrr(paddr_t start, unsigned size, unsigned flags, unsigned op) {
	unsigned	setting;
	int 		index;
	int 		old;

	CRASHCHECK(!KerextAmInKernel());

	setting = op & PTE_OP_MAP;

	if(!(SYSPAGE_ENTRY(cpuinfo)->flags & X86_CPU_MTRR) || (NUM_PROCESSORS > 1)) {
		return -1;
	}

	if(flags & SHMCTL_LAZYWRITE) {
		// Check that base and size are properly aligned
		if((start & (__PAGESIZE - 1)) || ((start & ~(paddr_t)(size - 1)) != start)) {
			return -1;
		}

		if((old = _find_overlapping_mtrr(start, size)) == -1 && setting && ((index = _find_free_mtrr()) != -1)) {
			struct _mtrr		*_mtrr = &mtrr_array[index];
			// Allocate a new MTRR to this range
			_mtrr->refcnt = 1;
			_mtrr->phys_base = (start & X86_MTRR_PHYSBASE_BASE_MASK) | X86_MTRR_TYPE_WRITECOMBINING;
			_mtrr->phys_mask = (~(paddr_t)(size - 1) & X86_MTRR_PHYSMASK_RANGE_MASK) | X86_MTRR_PHYSMASK_VALID;
			_mtrr->flags = MTRR_VALID;

			_write_mtrr(index, _mtrr);

			return 0;

		} else if(old != -1 && setting) {
			// Up refcnt on MTRR
			mtrr_array[old].refcnt++;
			return 0;

		} else if(old != -1) {
			--mtrr_array[old].refcnt;
			if((mtrr_array[old].refcnt == 1) && mtrr_array[old].flags & MTRR_BIOS_SET) {
				mtrr_array[old].phys_base = (mtrr_array[old].phys_base & X86_MTRR_PHYSBASE_BASE_MASK) | (mtrr_array[old].flags & MTRR_DEFAULT_TYPE_MASK);
			} else if(mtrr_array[old].refcnt == 0) {
				struct _mtrr		*_mtrr = &mtrr_array[old];
				// Allocate a new MTRR to this range
				_mtrr->refcnt = 0;
				_mtrr->phys_base = 0;
				_mtrr->phys_mask = 0;
				_mtrr->flags = 0;

				_write_mtrr(old, _mtrr);

			}
			return 0;
		}
	}
	return -1;
}


__SRCVERSION("x86_mtrr.c $Rev: 153052 $");
