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
#include <string.h>
#include <sys/perfregs.h>
#include <mips/context.h>
#include <mips/priv.h>
#include <mips/cpu.h>
#include <mips/sb1intr.h>

extern void rdecl save_perfregs_sb1( MIPS_PERFREGS *pcr );
extern void rdecl restore_perfregs_sb1( MIPS_PERFREGS *pcr );
extern void rdecl disable_counters_sb1(void);

static void rdecl (*save_perfregs_p)(MIPS_PERFREGS *pcr) = save_perfregs_sb1;
static void rdecl (*restore_perfregs_p)(MIPS_PERFREGS *pcr) = restore_perfregs_sb1;
static void rdecl (*disable_counters)(void) = disable_counters_sb1;



#define SB1_OVF_BIT (1LL<<40)
#define SB1_IE_BIT (1<<4)
static void rdecl mask_irq_sb1(MIPS_PERFREGS *pcr)
{
	/* Change the saved registers to mask the interrupt
		on any counters which have overflowed. */
	if (pcr->sb1.EventCounter[0] & SB1_OVF_BIT) {
		pcr->sb1.EventControl[0] &= ~SB1_IE_BIT;
	}
	if (pcr->sb1.EventCounter[1] & SB1_OVF_BIT) {
		pcr->sb1.EventControl[1] &= ~SB1_IE_BIT;
	}
	if (pcr->sb1.EventCounter[2] & SB1_OVF_BIT) {
		pcr->sb1.EventControl[2] &= ~SB1_IE_BIT;
	}
	if (pcr->sb1.EventCounter[3] & SB1_OVF_BIT) {
		pcr->sb1.EventControl[3] &= ~SB1_IE_BIT;
	}
}

static void rdecl (*mask_irq)(MIPS_PERFREGS *pcr) = mask_irq_sb1;



#ifdef VARIANT_instr
/* ifdef out alloc so no unused warnings on compile. */

static int rdecl 
cpu_alloc_perfregs( THREAD *thp ) 
{
	MIPS_PERFREGS *pcr;

	if((pcr = _smalloc(sizeof(*pcr))) == NULL) {
		return ENOMEM;
	}
	memset( pcr, 0, sizeof(*pcr) );
	thp->cpu.pcr = pcr;
	return EOK;
}

#endif

/* global functions */
_Uint32t 
cpu_perfreg_id(void) 
{
	static unsigned	perfreg_id = 0;
	unsigned cpu;

	/* NYI:  All checks / initializations for CPU should be moved to 
	cpu_init.c  */
	if ( perfreg_id == 0 ) {
		cpu = getcp0_prid();
		switch(MIPS_PRID_COMPANY_IMPL_CANONICAL(cpu)) {
		case MIPS_PRID_MAKE_COMPANY_IMPL(MIPS_PRID_COMPANY_BROADCOM, MIPS_PRID_IMPL_SB1):
			perfreg_id = PERFREGS_MAKEID(PERFREGS_CPUID_MIPS,PERFREGS_PARTID_SB1);
			break;
		default:
			perfreg_id = ~0;
			break;
		}
	}

	return perfreg_id;
}

void rdecl 
cpu_free_perfregs( THREAD *thp ) 
{
	if ( thp->cpu.pcr == &disabled_perfregs ) {
		return;
	}
	_sfree(thp->cpu.pcr,sizeof(*thp->cpu.pcr));
	thp->cpu.pcr = &disabled_perfregs;
}

void rdecl 
cpu_save_perfregs(void *vpcr) 
{
	MIPS_PERFREGS *pcr = vpcr;

	if ( (pcr->id & PERFREGS_ENABLED_FLAG) && 
		!(pcr->id & PERFREGS_SAVED_FLAG) && save_perfregs_p ) {

		pcr->id |= PERFREGS_SAVED_FLAG;
		(save_perfregs_p)(pcr);

		/* Note that the IE bit and OVF bits MUST be left intact in order 
		for the interrupt handler to determine the source of the interrupt. 
		The IE bits are all cleared in the interrupt callout and won't be
		re-enabled until the restore routine.  The OVF bits are cleared in
		the restore if necessary. */
		disable_counters();
	}
}

void rdecl 
cpu_restore_perfregs(void *vpcr) 
{
	MIPS_PERFREGS *pcr = vpcr;

	if ( (pcr->id & PERFREGS_ENABLED_FLAG) && restore_perfregs_p ) {
		(restore_perfregs_p)(pcr);
		pcr->id &= ~PERFREGS_SAVED_FLAG;
	} 
}



const struct sigevent *
perfregs_handler(void *dummy, int id) {
	THREAD *thp = actives_pcr[RUNCPU];

	if ( thp != NULL ) {
		mask_irq(thp->cpu.pcr);
	}

	return NULL;
}

int	rdecl 
cpu_debug_set_perfregs(THREAD *thp, debug_perfreg_t *regs) 
{

#ifdef VARIANT_instr
	int status, level;
	static int attached = 0;

	if ( thp->cpu.pcr == &disabled_perfregs ) {
		if ( (status = cpu_alloc_perfregs(thp)) != EOK ) {
			return status;
		}
	}
	memcpy( thp->cpu.pcr, regs, sizeof(*thp->cpu.pcr) );

	if ( !attached ) {
		lock_kernel();
		level = get_interrupt_level( NULL, SB1_INTR_PERF );
		if ( level >= 0 ) {
			(void) interrupt_attach( level, perfregs_handler, 0, 0 );
		}
		attached = 1;
	}
	return EOK;
#else
	return ENOTSUP;
#endif
}

int	rdecl 
cpu_debug_get_perfregs(THREAD *thp, debug_perfreg_t *regs) 
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

void rdecl 
cpu_debug_init_perfregs(void) 
{
	disabled_perfregs.id = PERFREGS_ENABLED_FLAG | cpu_perfreg_id();
}

__SRCVERSION("cpu_perfregs.c $Rev: 167933 $");
