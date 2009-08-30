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

#include "vmm.h"


int						pae_enabled;
unsigned				pd_bits = PD1_BITS;
unsigned				pxe_bits = PXE1_BITS;

struct xfer_slots 		*xfer_slot;

#define PROCTOP	((0xffffffffu - (4 << PD1_BITS)) + 1 - __PAGESIZE)

uintptr_t				pde_map = PROCTOP;

/*
 * What the address space looks like
 *
 * ffc00000 -> L1 pagetable maps to itself (r/o) (~PAE)
 * fec00000 -> 0-1M mapping, mappings for startup allocations (syspage, etc)
 * ff400000 -> with new cpu's, 4k mappings for syspage, gdt, fpu emulation stub etc.
 *
 * f0000000 -> either:
 *				normal kernel mapping
 *				4m pagetable covering only the kernel code and data (Pentium and above)
 * e0000000-efffffff: on 4m cpu's, maps the first 256MB of physical memory
 * d0400000-dfffffff: normal kernel mapping
 * d0000000-d03fffff: temp mappings to zero new L2 pagetables
 * c0000000-cfffffff: message pass temporary mappings
 *
 *  ^^^^^^^^^^^^^^^^
 *  System addresses
 *=================================================================
 *  User addresses
 *  vvvvvvvvvvvvvvvv
 *
 *	 shared libs		0xbfffffff
 *   						|
 *							V
 *						0xb0300000
 *							^
 *							|
 *   Shared objects		0x40100000
 *							^
 *							|			("brk")
 *   Heap					|
 * 							^
 *				BSS			|
 *				Data		|
 *				Text		|
 *   Process image		0x08040000
 *   Stack					|
 *							V
 *	 Thread stacks			|
 *							V
 *	 System				0x00000000
 */

void 
vmm_init_mem(int phase) {
	unsigned						cr0;
	unsigned						pde_mask;
	uintptr_t						base;
	int								i;
	unsigned						slot_size;
	struct xfer_slots				*slot;
	extern void __exc7emul();
	extern void __exc7emul_end();
	extern void *__fpuemu_stub;
	
	if(phase != 0) {
		unsigned		pdir_user_boundry;

		// Remove the the 1-to-1 mapping that we used to get paging enabled.
		if(pae_enabled) {
			pdir_user_boundry = (SYSADDR_BASE >> PD2_BITS) * sizeof(uint64_t);
		} else {
			pdir_user_boundry = (SYSADDR_BASE >> PD1_BITS) * sizeof(uint32_t);
		}
		CPU_ZERO_PAGE((void *)_syspage_ptr->un.x86.pgdir[0], pdir_user_boundry, 0);
		pa_start_background();
		fault_init();
		lock_init();
		return;
	}

	if(__cpu_flags & X86_CPU_PAE) {
		if(rdcr4() & X86_CR4_PAE) {
			pae_enabled = 1;
			pd_bits = PD2_BITS;
			pxe_bits = PXE2_BITS;
		}
	}
	pde_mask = (1 << pd_bits) - 1;


	user_boundry_addr = CPU_USER_VADDR_END;

	pgtbl_init();

	pa_init(1);

	// We may need to remap the fpu emulation code somewhere else
	if((__cpu_flags & X86_CPU_PSE) && (PXE_GET_FLAGS(VTOPDIRP((uintptr_t)&__exc7emul)) & X86_PDE_PS)) {
		unsigned	vaddr;
		paddr_t		paddr;

		vaddr = ((_syspage_ptr->un.x86.pgdir[0] + pde_mask) & ~pde_mask) - __PAGESIZE;
		paddr = (PXE_GET(VTOPDIRP((uintptr_t)&__exc7emul)) & ~(uint64_t)pde_mask) + ((uintptr_t)&__exc7emul & pde_mask);
		paddr &= PTE_PADDR_BITS;
		ldcr0((cr0 = rdcr0()) & ~X86_MSW_WP_BIT);
		PXE_SET(VTOPTEP(vaddr), paddr|(X86_PTE_USER|X86_PTE_PRESENT));
		ldcr0(cr0);
		__fpuemu_stub = (void *)(vaddr + ADDR_OFFSET(((uintptr_t)&__exc7emul)));
	} else {
		ldcr0((cr0 = rdcr0()) & ~X86_MSW_WP_BIT);
		PXE_SET_FLAGS(VTOPTEP((uintptr_t)&__exc7emul), ~0, X86_PTE_USER);
		PXE_SET_FLAGS(VTOPTEP((uintptr_t)&__exc7emul_end), ~0, X86_PTE_USER);
		ldcr0(cr0);
	}
			
	if((__cpu_flags & X86_CPU_PSE) && !(PXE_GET_FLAGS(VTOPDIRP(CPU_1TO1_VADDR_BIAS)) & X86_PTE_PRESENT)) {
		//__cpu_flags &= ~X86_CPU_PSE;
		// Crash for now, make sure people have a new startup
		crash();
	}

	if(__cpu_flags & X86_CPU_PSE) {
		// Adjust pde_map if needed
		pde_map = (uintptr_t) ((_syspage_ptr->un.x86.pgdir[0] + pde_mask) & ~pde_mask) - 2*__PAGESIZE;
		pgszlist[0] = pde_mask + 1;
		pgszlist[1] = __PAGESIZE;
	} else {
		pgszlist[0] = __PAGESIZE;
	}


	if(__cpu_flags & X86_CPU_MTRR) {
		x86_init_mtrr();
	}

	// Allocate the slot lists (Should add check for large number of cpus...
	if(!(xfer_slot = slot = _scalloc(_syspage_ptr->num_cpu * sizeof *xfer_slot))) {
		crash();		
	}
	slot_size = (MAP_SIZE / _syspage_ptr->num_cpu) & ~((1 << (pd_bits+1))-1);
	base = MAP_BASE;
	for(i = 0; i < _syspage_ptr->num_cpu; i++, slot++) {
		slot->addr = slot->first = base;
		base = slot->last = base + slot_size;
	}
}

__SRCVERSION("vmm_init_mem.c $Rev: 164196 $");
