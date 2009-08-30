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
 * init_traps.c
 *	Set up to take over all exception handling
 */
#include "externs.h"

struct exc {
	uint32_t	len;
	uint32_t	code[0x10000]; // variably sized
};

extern struct exc	exc_tlbmiss_generic;
extern struct exc	exc_general;
extern struct exc	exc_cache_generic;
extern struct exc	exc_general_sr7100;
extern struct exc	exc_general_sb1;
extern struct exc	exc_cache_sr7100;
extern struct exc	exc_intr;

extern struct exc	exc_tlbmiss_sb1;
extern struct exc	exc_cache_sb1;
extern struct exc	exc_generic_sb1;

extern struct exc	exc_tlbmiss_r7k;

extern struct exc	exc_tlbmiss_e9k;

extern void 	(*r4k_exception_table[])(void);

extern void		r4k_nocoproc_handler(void);
extern void		r4k_fpu_handler(void);
extern void		sb1_bus_handler(void);

#define INSTALL(d, s)	\
		memcpy((void *)(d), (s)->code, (s)->len);	\
		if(((d)+(s)->len) > last) last = (d)+(s)->len

/*
 * init_traps()
 *	Initialize machine-dependent exception stuff
 */
void
init_traps(void) {
	uintptr_t	last;
	struct exc	*tlbmiss	= &exc_tlbmiss_generic;
	struct exc	*general	= &exc_general;
#if !defined(VARIANT_r3k)	
	struct exc	*cache		= &exc_cache_generic;
	struct exc	*intr		= &exc_intr;
	unsigned	prid;
#endif	

	/*
	 * Install exception handling
	 */
#if !defined(VARIANT_r3k)	
	prid = getcp0_prid();

	if(MIPS_PRID_COMPANY(prid) == MIPS_PRID_COMPANY_BROADCOM) {
		// If we're on a Broadcom chip, install special cache error
		// handler. See kernel.S for rationale.
		general	= &exc_general_sb1;
		cache   = &exc_cache_sb1;
		tlbmiss = &exc_tlbmiss_sb1;
		r4k_exception_table[MIPS_CAUSE_BUS_DATA] = sb1_bus_handler;
	} else if(MIPS_PRID_COMPANY_IMPL(prid) == MIPS_PRID_MAKE_COMPANY_IMPL(MIPS_PRID_COMPANY_SANDCRAFT, MIPS_PRID_IMPL_SR7100)) {
		general	= &exc_general_sr7100;
		cache	= &exc_cache_sr7100;
	} else if(MIPS_PRID_COMPANY_IMPL(prid) == MIPS_PRID_MAKE_COMPANY_IMPL(MIPS_PRID_COMPANY_NONE, MIPS_PRID_IMPL_7000)) {
		tlbmiss = &exc_tlbmiss_r7k;
	} else if(MIPS_PRID_COMPANY_IMPL(prid) == MIPS_PRID_MAKE_COMPANY_IMPL(MIPS_PRID_COMPANY_NONE, MIPS_PRID_IMPL_E9000)) {
		tlbmiss = &exc_tlbmiss_e9k;
	}
#endif	
	last = 0;
	INSTALL(MIPS_EXCV_UTLB_MISS, tlbmiss);
#if defined(VARIANT_r3k)	
	INSTALL(MIPS_EXCV_UTLB_MISS+0x80,general);
#else	
	INSTALL(MIPS_EXCV_GENERAL,   general);
	INSTALL(MIPS_EXCV_XTLB_MISS, tlbmiss);
	INSTALL(MIPS_EXCV_CACHEERR,  cache);
	INSTALL(MIPS_EXCV_INTERRUPT, intr);
#endif	
	
	if(fpuemul) {
		r4k_exception_table[MIPS_CAUSE_CP_UNUSABLE] = r4k_nocoproc_handler;
	} else {
		r4k_exception_table[MIPS_CAUSE_CP_UNUSABLE] = r4k_fpu_handler;
	}

	/*
	 * The memory manager hasn't been initialized yet, so any vaddr=>paddr
	 * translations requests will fail, but we're depending on the fact
	 * that we only really have to get the L1 caches flushed/purged and
	 * they don't require v=>p's. If we don't do this here, we won't
	 * be able to use the kernel debugger until much later.
	 *
	 * The MS_SYNC below is required since we have to be sure to force the
	 * exception code to main memory (MS_INVALIDATE_ICACHE just pushes
	 * to the first unified cache) - the cache error exception executes
	 * from KSEG1.
	 */
	CacheControl((void *)MIPS_R4K_K0BASE, last - MIPS_R4K_K0BASE,
					MS_INVALIDATE_ICACHE|MS_SYNC);

	// Set BEV here, since some startup's prefer not to set it 
	// until the kernel traps are installed
	setcp0_sreg(getcp0_sreg() & ~MIPS_SREG_BEV);

	// We used to do some SREG configuration here, but startup takes
	// care of everything.
}

__SRCVERSION("init_traps.c $Rev: 156872 $");
