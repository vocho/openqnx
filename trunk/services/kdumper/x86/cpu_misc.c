#include <sys/mman.h>
#include <x86/cpu.h>
#include "kdumper.h"

//Identify 1-to-1 mapping vaddrs & paddrs
#define CPU_SYSTEM_PADDR_START	0
#define CPU_SYSTEM_PADDR_END	((256*1024*1024)-1)
#define CPU_1TO1_VADDR_BIAS		0xe0000000u
#define CPU_1TO1_IS_VADDR(v)	((__cpu_flags & X86_CPU_PSE) && ((v) >= CPU_1TO1_VADDR_BIAS) && ((v) <= (CPU_1TO1_VADDR_BIAS+CPU_SYSTEM_PADDR_END)))
#define CPU_1TO1_IS_PADDR(p)	((__cpu_flags & X86_CPU_PSE) && ((p) <= CPU_SYSTEM_PADDR_END))
#define CPU_SYSTEM_VADDR_START	0xc0000000u

/* This is used when PAE is disabled */
/* To make these work the last entry of the page directory points to itself */

#define PG1_REMAP        0x3ff
#define PG1_PDIRADDR     (PG1_REMAP << 22 | PG1_REMAP << 12)	// Pointer to start of page directory
#define PG1_PTEADDR      (PG1_REMAP << 22)						// Pointer to first page table

#define V1TOPDIRP(v)    (uint32_t *)(PG1_PDIRADDR | (((uint32_t)(v))>>20&~3))		// Pointer to page directory entry
#define V1TOPTEP(v)     (uint32_t *)(PG1_PTEADDR  | (((uint32_t)(v))>>10&~3))		// Pointer to page table entry
#define V1TOPTP(v)      (uint32_t *)(PG1_PTEADDR  | (((uint32_t)(v))>>10&0x3ff000))	// Pointer to start of page table
#define V1TOPADDR(v)	((*V1TOPTEP(v)&~(PAGESIZE-1))|((uint32_t)(v)&(PAGESIZE-1)))	// Physical address


/* This is used when PAE is enabled */
/* To make these work the last 4 entries of the PDPT point to 4 pages within a 16k page directory */

#define PG2_PDIRADDR    0xffffc000		// Pointer to start of page directory
#define PG2_PTEADDR     0xff800000		// Pointer to first page table

#define V2TOPDIRP(v)    (uint64_t *)(PG2_PDIRADDR | (((uint32_t)(v))>>18&~7))		// Pointer to page directory entry
#define V2TOPTEP(v)     (uint64_t *)(PG2_PTEADDR  | (((uint32_t)(v))>>9&~7))		// Pointer to page table entry
#define V2TOPTP(v)      (uint64_t *)(PG2_PTEADDR  | (((uint32_t)(v))>>9&0x7ff000))	// Pointer to start of page table
#define V2TOPADDR(v)    ((*V2TOPTEP(v)&~(__PAGESIZE-1))|((uint32_t)(v)&(__PAGESIZE-1)))	// Physical address


void
cpu_elf_header(void *ehdr) {
	if(dip->big) {
		((Elf64_Ehdr *)ehdr)->e_machine = EM_486;
	} else {
		((Elf32_Ehdr *)ehdr)->e_machine = EM_486;
	}
}


void
cpu_note(struct kdump_note *note) {
	int		pae_enabled = 0;

	if(SYSPAGE_ENTRY(cpuinfo)[0].flags & X86_CPU_PAE) {
		if(rdcr4() & X86_CR4_PAE) {
			pae_enabled = 1;
		}
	}
	note->cpu_info = pae_enabled;
}


static int
check_l2(struct asinfo_entry *as, char *name, void *d) {
	uint64_t	paddr = *(uint64_t *)d;

	if((paddr >= as->start) && (paddr < as->end)) return 0;
	return 1;
}


void
cpu_walk_extra_pmem(void (*func)(paddr_t, paddr_t, int, void *), void *data) {
	uintptr_t			pdep;
	uintptr_t			end;
	uint64_t			pde;
	int					pae_enabled = 0;

	if((rdcr0() & X86_MSW_PG_BIT) == 0) {
		// paging _not_ enabled
		return;
	}
	if(SYSPAGE_ENTRY(cpuinfo)[0].flags & X86_CPU_PAE) {
		if(rdcr4() & X86_CR4_PAE) {
			pae_enabled = 1;
		}
	}

	// Some of the L2 page tables in the system space are allocated by 
	// startup rather than procnto and, as such, they're not in the 
	// physical allocator structures.
	// We look for L2 page tables that aren't in an asinfo sysram
	// region - those are the ones that startup created and have to
	// be written out specially.
	
	if(pae_enabled) {
		pdep = (uintptr_t)V2TOPDIRP(CPU_SYSTEM_VADDR_START);
		end  = (uintptr_t)V2TOPDIRP(0xffffffff) - 8*sizeof(uint64_t);
	} else {
		pdep = (uintptr_t)V1TOPDIRP(CPU_SYSTEM_VADDR_START);
		end  = (uintptr_t)V1TOPDIRP(0xffffffff) - sizeof(uint32_t);
	}
	do {
		if(pae_enabled) {
			pde = *(uint64_t *)pdep;
			pdep += sizeof(uint64_t);
		} else {
			pde = *(uint32_t *)pdep;
			pdep += sizeof(uint32_t);
		}
		if((pde & X86_PDE_PRESENT) && !(pde & X86_PDE_PS)) {
			pde &= ~0x8000000000000fff; // turn into a paddr
			if(walk_asinfo("sysram", check_l2, &pde)) {
				// not in sysram, write it out...
				func(pde, 0x1000, -1, data);
			}
		}
	} while(pdep <= end);
}
