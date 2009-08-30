#include "externs.h"

int ppc_can_watch(DEBUG *dep, BREAKPT *bpp)
{
	dep->cpu.max_hw_watchpoints = 2; /* need proper init function? */
	if ( bpp->brk.type & _DEBUG_BREAK_EXEC ) {
		/* NYI - Book E defines 4 instruction address compare regs */
		return 0;
	} else {
		/* Since largest data access is going to be 64bit then the largest
		 * size we can watch is 8 bytes.
		 */
		if ( bpp->brk.size > 8 ) {
			return 0;
		}
		if ( (bpp->brk.type & _DEBUG_BREAK_MODIFY) 
				&& (bpp->brk.size > sizeof(bpp->cpu.data.value)) ) {
			return 0;
		}
		/*
		 * The alignment must match the requested size I, guess?
		 */
		if ( (bpp->brk.addr & ~(bpp->brk.size-1)) != 0 ) {
			return 0;
		}
	}
	return 1;
}

BREAKPT *ppc_identify_watch(DEBUG *dep, THREAD *thp, unsigned *pflags)
{
	unsigned	dbsr, match, hwreg;
	BREAKPT		*d, *b;

	dbsr = thp->cpu.dbsr;

	match = 0;
	hwreg = 0;
	if ( dbsr & PPCBKE_DBSR_DAC1R ) {
#ifdef DEBUG_DEBUG
		kprintf("DAC1R\n");
#endif
		match = _DEBUG_BREAK_RDM;
		*pflags |= _DEBUG_FLAG_TRACE_RD;
	}
	if ( dbsr & PPCBKE_DBSR_DAC1W ) {
#ifdef DEBUG_DEBUG
		kprintf("DAC1W\n");
#endif
		match = _DEBUG_BREAK_WRM;
		*pflags |= _DEBUG_FLAG_TRACE_WR;
	}

	if ( !match ) {
		hwreg = 1;
		if ( dbsr & PPCBKE_DBSR_DAC2R ) {
#ifdef DEBUG_DEBUG
			kprintf("DAC2R\n");
#endif
			match = _DEBUG_BREAK_RDM;
			*pflags |= _DEBUG_FLAG_TRACE_RD;
		}
		if ( dbsr & PPCBKE_DBSR_DAC2W ) {
#ifdef DEBUG_DEBUG
			kprintf("DAC2W\n");
#endif
			match = _DEBUG_BREAK_WRM;
			*pflags |= _DEBUG_FLAG_TRACE_WR;
		}
	}

	if ( match == 0 ) {
		return NULL;
	}

	for(b = NULL, d = dep->brk; d; d = d->next) {
#ifdef DEBUG_DEBUG
		kprintf("breakpoint type %x addr %x reg %d\n", d->brk.type, d->brk.addr, d->cpu.hwreg );
#endif
		if ((d->brk.type & match) && (d->cpu.hwreg == hwreg)) {
			b = d;
			if(d->brk.type & _DEBUG_BREAK_MODIFY) {
				if(memcmp((void *)d->brk.addr, d->cpu.data.value, d->brk.size)) {
					memcpy(d->cpu.data.value, (void *)d->brk.addr, d->brk.size);
					*pflags |= _DEBUG_FLAG_TRACE_MODIFY;
				}
			}
		}
	}

	return b;
}

void ppc_set_watch(DEBUG *dep, BREAKPT *bpp )
{
	unsigned dbcr0 = dep->cpu.dbcr0;

#ifdef DEBUG_DEBUG
	kprintf("set_watch %x ", bpp->brk.addr );
#endif
	if ( bpp->brk.type & _DEBUG_BREAK_RDM ) {
		dbcr0 |= (bpp->cpu.hwreg == 0) ? PPCBKE_DBCR0_DAC1_LOAD:PPCBKE_DBCR0_DAC2_LOAD;
#ifdef DEBUG_DEBUG
		kprintf("RDM DAC%d %x\n", bpp->cpu.hwreg + 1, dbcr0 );
#endif
	}
	if ( bpp->brk.type & _DEBUG_BREAK_WRM ) {
		dbcr0 |= (bpp->cpu.hwreg == 0) ? PPCBKE_DBCR0_DAC1_STORE:PPCBKE_DBCR0_DAC2_STORE;
#ifdef DEBUG_DEBUG
		kprintf("WRM DAC%d %x\n", bpp->cpu.hwreg + 1, dbcr0 );
#endif
	}
	/* Set address in the data address comparison register */
	if ( bpp->cpu.hwreg == 0 ) {
		set_spr( PPCBKE_SPR_DAC1, bpp->brk.addr );
	} else {
		set_spr( PPCBKE_SPR_DAC2, bpp->brk.addr );
	}
	/* Now enable that register */
	dep->cpu.dbcr0 = dbcr0;
}

void ppc_clear_watch(DEBUG *dep, BREAKPT *bpp)
{
	unsigned dbcr0 = dep->cpu.dbcr0;
	
	dbcr0 &= bpp->cpu.hwreg == 0 ? ~(PPCBKE_DBCR0_DAC1_BOTH):~(PPCBKE_DBCR0_DAC2_BOTH);

	dep->cpu.dbcr0 = dbcr0;
}

