#include "kdebug.h"
#include <ppc/400cpu.h>
#include <ppc/603cpu.h>
#include <ppc/700cpu.h>
#include <ppc/800cpu.h>

unsigned int	spinlock = 0;
unsigned int	msr_bits_off = PPC_MSR_PR|PPC_MSR_EE|PPC_MSR_IR|PPC_MSR_DR;

static void
family_dummy(int type, CPU_REGISTERS *ctx) {
}

void	(*family_stuff)(int type, CPU_REGISTERS *ctx) = family_dummy;

void 	
standard_flusher(uintptr_t mapped_vaddr, uintptr_t real_vaddr, unsigned len) {
	cache_flush(mapped_vaddr, len);
}

void 	(*icache_flusher)(uintptr_t, uintptr_t, unsigned) = standard_flusher;

void
cpu_init() {
}


static void
kd_set_trap(const struct trap_entry *trp) {
	uint32_t	*vector;
	int32_t		func;

	vector = &_syspage_ptr->un.ppc.exceptptr[trp->trap_index];
	func = (uint32_t)trp->func;
	if((func >= -0x03fffffc) && (func <= 0x03fffffc)) {
		/* can branch absolute */
		func |= 0x02;
	} else {
		func -= (uint32_t)vector;
#if 0
		if( func >= -0x03fffffc && func <= 0x3fffffc ) {
			/* branch relative in range */
		} else {
			crash( __FILE__, __LINE__ );
		}
#endif
	}
	*vector = 0x48000000 | func;
	cache_flush((paddr_t)vector, sizeof(uint32_t));
}

void
install_traps(const struct trap_entry *trp, unsigned num) {
	do {
		kd_set_trap(trp++);
	} while(--num != 0);
}

/*
 * init_traps()
 *	Initialize machine-dependent exception stuff
 */
void
init_traps(void) {
	unsigned	pvr;
	int			r;

	pvr = SYSPAGE_CPU_ENTRY(ppc, kerinfo)->pretend_cpu;
	if(pvr == 0) pvr = SYSPAGE_ENTRY(cpuinfo)->cpu;

	switch(SYSPAGE_CPU_ENTRY(ppc, kerinfo)->ppc_family) {
	case PPC_FAMILY_UNKNOWN:
		kprintf("PPC family not set - update your startup!\n");
		r = family_init_400(pvr);
		if(!r) {
			r = family_init_600(pvr);
			if(!r) {
				r = family_init_800(pvr);
			}
		}
		break;
	case PPC_FAMILY_400:
		r = family_init_400(pvr);
		break;
	case PPC_FAMILY_600:
		r = family_init_600(pvr);
		break;
	case PPC_FAMILY_800:
		r = family_init_800(pvr);
		break;
	case PPC_FAMILY_900:
		r = family_init_900(pvr);
		break;
	case PPC_FAMILY_booke:
		r = family_init_booke(pvr);
		break;
	default:
		r = 0;
		break;
	}
	if(!r) {
		kprintf("Can not hook exception vectors for PVR %x\n", pvr);
	}
}
