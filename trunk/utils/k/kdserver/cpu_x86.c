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




#include "kdserver.h"
#include _NTO_HDR_(x86/cpu.h)
#include _NTO_HDR_(x86/context.h)

/*
 Define a structure used for communication to a GDB client
 */
typedef struct {
	int	eax, ecx, edx, ebx, uesp, ebp, esi, edi, eip, efl;
	int cs, ss, ds, es, fs, gs;
} x86_gdb_regs;


static int
x86_init(int elf_cpu) {
	switch(elf_cpu) {
	case EM_386:	
	case EM_486:
		break;
	default:
		return 0;
	}

	if(note->cpu_info) {
		// PAE enabled
		cpu->pgtbl_size = 0x4000;
	}

	return 1;
}


static int
x86_v2p(uintptr_t vaddr, paddr64_t *paddrp, unsigned *lenp) {
	uint64_t	pde;
	uint64_t	pte;
	paddr64_t	paddr;
	unsigned	offset;
	unsigned	page_size;

	// consult the page table...
	if(note->cpu_info) {
		// PAE enabled
		pde = endian_native64(((uint64_t *)pgtbl)[vaddr >> 21]);
		page_size = 2*1024*1024;
	} else {
		pde = endian_native32(((uint32_t *)pgtbl)[vaddr >> 22]);
		page_size = 4*1024*1024;
	}
	if(!(pde & X86_PDE_PRESENT)) {
		return 0;
	}
	if(!(pde & X86_PDE_PS)) {
		// 4K page
		page_size = 0x1000;
		paddr = pde & ~(X86_PDE_NX | (page_size-1));
		if(note->cpu_info) {
			//PAE enabled
			paddr += ((vaddr >> 12) & 0x1ff)*sizeof(uint64_t);
			if(core_read_paddr(paddr, &pte, sizeof(pte)) != sizeof(pte)) {
				fprintf( stderr, "PAE core_readmem of %llx failed\n", paddr );
				return 0;
			}
			pte = endian_native64(pte);
		} else {
			uint32_t	pte32;

			paddr += ((vaddr >> 12) & 0x3ff)*sizeof(uint32_t);
			if(core_read_paddr(paddr, &pte32, sizeof(pte32)) != sizeof(pte32)) {
				fprintf( stderr, "core_readmem of %llx failed\n", paddr );
				return 0;
			}
			pte = endian_native32(pte32);
		}
		paddr = pte & ~(X86_PDE_NX | (page_size-1));
		if(!(pte & X86_PTE_PRESENT)) { 
			return 0;
		}
	} else {
		paddr = pde & ~(X86_PDE_NX | (page_size-1));
	}
	offset = vaddr & (page_size - 1);
	*paddrp = paddr | offset;
	*lenp  = page_size - offset;
	return 1;
}


static void
x86_cvt_regset(const void *in, void *out) {
	const X86_CPU_REGISTERS		*ctx = in;
	x86_gdb_regs				*gdb = out;

	gdb->eax = ctx->eax;
	gdb->ebx = ctx->ebx;
	gdb->ecx = ctx->ecx;
	gdb->edx = ctx->edx;
	gdb->esi = ctx->esi;
	gdb->edi = ctx->edi;
	gdb->ebp = ctx->ebp;
	//
	// On the X86, the ctx->esp value is not valid if we didn't
	// perform a privity switch. Check to see if we're coming from ring 0
	// and, if so, use the exx value in the context for the stack pointer
	// (that's the ESP from before the PUSHA). The "+ 3*4" is to skip
	// over the EIP/CS/EFLAGS pushed by the exception. Ugh.
	//
	if((ctx->cs & 0x3) == 0) {
		gdb->uesp = ctx->exx + 3*4;
		gdb->ss = 0;
	} else {
		gdb->uesp = ctx->esp;
		gdb->ss = ctx->ss;
	}
	gdb->efl = ctx->efl;
	gdb->eip = ctx->eip;
	gdb->cs = ctx->cs;
	gdb->ds = 0;
	gdb->es = 0;
	gdb->fs = 0;
	gdb->gs = 0;
}


struct cpuinfo 	cpu_x86 = {
	x86_init,
	x86_v2p,
	x86_cvt_regset,
	0x1000,
	sizeof(x86_gdb_regs),
	sizeof(uint32_t)
};
