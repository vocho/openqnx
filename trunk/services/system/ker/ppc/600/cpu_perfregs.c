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
#include <ppc/intr.h>
#include <ppc/700cpu.h>
#include <string.h>
#include <sys/perfregs.h>



static void rdecl save_perfregs_7450(PPC_PERFREGS *pcr)
{
	pcr->mpc7450.mmc[0] = get_spr( PPC7450_SPR_MMCR0 );

	/* Disable counting so that a PMI isn't generated in
	the kernel code. */
	set_spr( PPC7450_SPR_MMCR0, 0x80000000);

	pcr->mpc7450.mmc[1] = get_spr( PPC7450_SPR_MMCR1 );
	pcr->mpc7450.mmc[2] = get_spr( PPC7450_SPR_MMCR2 );

	pcr->mpc7450.pmc[0] = get_spr( PPC7450_SPR_PMC1 );
	pcr->mpc7450.pmc[1] = get_spr( PPC7450_SPR_PMC2 );
	pcr->mpc7450.pmc[2] = get_spr( PPC7450_SPR_PMC3 );
	pcr->mpc7450.pmc[3] = get_spr( PPC7450_SPR_PMC4 );
	pcr->mpc7450.pmc[4] = get_spr( PPC7450_SPR_PMC5 );
	pcr->mpc7450.pmc[5] = get_spr( PPC7450_SPR_PMC6 );

	pcr->mpc7450.sia = get_spr( PPC7450_SPR_SIA );
}

static void rdecl restore_perfregs_7450(PPC_PERFREGS *pcr)
{
	set_spr( PPC7450_SPR_PMC1, pcr->mpc7450.pmc[0] );
	set_spr( PPC7450_SPR_PMC2, pcr->mpc7450.pmc[1] );
	set_spr( PPC7450_SPR_PMC3, pcr->mpc7450.pmc[2] );
	set_spr( PPC7450_SPR_PMC4, pcr->mpc7450.pmc[3] );
	set_spr( PPC7450_SPR_PMC5, pcr->mpc7450.pmc[4] );
	set_spr( PPC7450_SPR_PMC6, pcr->mpc7450.pmc[5] );

	set_spr( PPC7450_SPR_MMCR1, pcr->mpc7450.mmc[1] );
	set_spr( PPC7450_SPR_MMCR2, pcr->mpc7450.mmc[2] );

	set_spr( PPC7450_SPR_SIA, pcr->mpc7450.sia );

	/* MMC0 contains the FREEZE bits.  This register should be
	restored last so that counting resumes only after all other
	registers have been restored. */
	set_spr( PPC7450_SPR_MMCR0, pcr->mpc7450.mmc[0] );

}




static void rdecl choose_save_perfregs( PPC_PERFREGS *pcr );
static void rdecl choose_restore_perfregs( PPC_PERFREGS *pcr );

static void rdecl (*save_perfregs_p)(PPC_PERFREGS *pcr) = choose_save_perfregs;
static void rdecl (*restore_perfregs_p)(PPC_PERFREGS *pcr) = choose_restore_perfregs;

static void rdecl choose_save_perfregs( PPC_PERFREGS *pcr )
{
	switch(cpu_perfreg_id()) {
		case PERFREGS_MAKEID(PERFREGS_CPUID_PPC,PERFREGS_PARTID_7450):
			save_perfregs_p = save_perfregs_7450;
			break;
		default:
			save_perfregs_p = NULL;
			break;
	}
	if (save_perfregs_p) {
		(save_perfregs_p)(pcr);
	}
}

static void rdecl choose_restore_perfregs( PPC_PERFREGS *pcr )
{
	switch(cpu_perfreg_id()) {
		case PERFREGS_MAKEID(PERFREGS_CPUID_PPC,PERFREGS_PARTID_7450):
			restore_perfregs_p = restore_perfregs_7450;
			break;
		default:
			restore_perfregs_p = NULL;
			break;
	}
	if (restore_perfregs_p) {
		(restore_perfregs_p)(pcr);
	}
}


#ifdef VARIANT_instr
/* Ifdef alloc to remove "unused" warnings from compile. */

static int rdecl cpu_alloc_perfregs( THREAD *thp )
{
	PPC_PERFREGS *pcr;

	if((pcr = _smalloc(sizeof(*pcr))) == NULL) {
		return ENOMEM;
	}

	memset( pcr, 0, sizeof(*pcr) );
	thp->cpu.pcr = pcr;
	return EOK;
}

#endif


void rdecl cpu_save_perfregs(void *vpcr)
{
	PPC_PERFREGS *pcr = vpcr;

	if ( (pcr->id & PERFREGS_ENABLED_FLAG) && 
		!(pcr->id & PERFREGS_SAVED_FLAG) && save_perfregs_p ) {

		/* Once the registers have been saved once, they need to
		be marked so that they aren't "saved over" before a
		restore.  This happens when multiple saves occur before 
		a restore. */
		pcr->id |= PERFREGS_SAVED_FLAG;
		(save_perfregs_p)(pcr);

	}
}

void rdecl cpu_restore_perfregs(void *vpcr)
{
	PPC_PERFREGS *pcr = vpcr;
	

	if ( (pcr->id & PERFREGS_ENABLED_FLAG) && restore_perfregs_p ) {
		(restore_perfregs_p)(pcr);
		/* Mark the registers as unsaved. */
		pcr->id &= ~PERFREGS_SAVED_FLAG;
	}
}

_Uint32t cpu_perfreg_id(void)
{
	static int 					perfreg_id = 0;
	struct cpuinfo_entry		*cpu_p;
	unsigned					type;

	if ( perfreg_id != 0 ) {
		return perfreg_id;
	}

	/* NYI:  Shouldn't be looking for CPU type here.  Should be moved to
	the init_cpu<family>.c file to localize CPU type checks. */
	cpu_p = SYSPAGE_ENTRY(cpuinfo);
	type = PPC_GET_FAM_MEMBER(cpu_p->cpu);
	switch(type) {
		case PPC_7450:
		case PPC_7455:
			perfreg_id = PERFREGS_MAKEID(PERFREGS_CPUID_PPC,PERFREGS_PARTID_7450);
			break;
		default:
			perfreg_id = -1;
			break;
	}

	return perfreg_id;
}

void rdecl cpu_free_perfregs( THREAD *thp )
{
	if ( thp->cpu.pcr == &disabled_perfregs ) {
		return;
	}

	_sfree(thp->cpu.pcr,sizeof(*thp->cpu.pcr));
	thp->cpu.pcr = &disabled_perfregs;
}


int	rdecl cpu_debug_set_perfregs(THREAD *thp, debug_perfreg_t *regs)
{
#ifdef VARIANT_instr
	int status;

	if ( thp->cpu.pcr == &disabled_perfregs ) {
		if ( (status = cpu_alloc_perfregs(thp)) != EOK ) {
			return status;
		}
	}
	memcpy( thp->cpu.pcr, regs, sizeof(*thp->cpu.pcr) );
	return EOK;
#else
	return ENOTSUP;
#endif
}

int	rdecl cpu_debug_get_perfregs(THREAD *thp, debug_perfreg_t *regs)
{
#ifdef VARIANT_instr
	if ( thp->cpu.pcr == &disabled_perfregs ) {
		return ENXIO;
	}
	memcpy( regs, thp->cpu.pcr, sizeof(*thp->cpu.pcr) );

	return EOK;
#else
	return ENOTSUP;
#endif

}


__SRCVERSION("cpu_perfregs.c $Rev: 153052 $");
