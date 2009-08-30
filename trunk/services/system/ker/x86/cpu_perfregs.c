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
#include <x86/context.h>
#include <x86/priv.h>
#include <x86/cpu.h>
#include <x86/intr.h>

extern unsigned long hw_cpuid(int v, void *p, void *featuresp);

static void rdecl save_perfregs_pentium( X86_PERFREGS *pcr )
{
	pcr->pentium.ctr0 = rdmsr( 18 );
	pcr->pentium.ctr1 = rdmsr( 19 );
	pcr->pentium.cesr = rdmsr( 17 );
}



/* Unfortunately, the P6 WRMSR for the performance counters is brain
dead.  Only the bottom 31 bits can be written "correctly".  If bit 31
is set, the upper 8 bits are sign extended so that what's actually
stored in the counter is 0xFFxxxxxxxx). This is apparently so that
negative numbers can be written into the counter (although WHY you'd
want to put a "negative" number in the counter instead of being able to
write the whole counter is beyond me. Looks to me like the HW dudes
wanted to restrict themselves to a single word write and concocted
a "feature" to justify a bad design).
To work around this, the P6 save/restore switches between a "set"
and "sum" mode (as determined by the restore).  If the current value
is in the "good" range, then the counter can be saved / restore directly.
In the "bad" range, the counter is restored to 0 so that what is counted
is actually the numebr of events since the thread was swapped in and this
is then summed to the currently saved value on a swap out.
This covers the value returned to user space, but there is a potential
problem with the interrupt on overflow case.  If the thread restores into
"sum" mode, the counter value does not represent the total event count and
hence will not cause an interrupt at the correct time.  In the uniprocessor
case, the OS tick interrupt will guarantee a save / restore (and appropriate
mode switch from sum to set) every few ms and the potential worst failure
case is if > (2^31 - 1) events occur in between the interrupts (which is
highly unlikely).  However, in the SMP case, if the thread isn't running on
the boot processor then there's no guarantee of a regular context save/
restore so the interrupt on overflow case becomes more possible.  For that 
reason, a limitation will be documented in which it is recommended that the
initial counter value be set to >= 0xFF800000000 for interrupt on overflow
in the SMP case (this guarantees that the counter value can be read / written
directly for the full time leading up to and including the overflow). */


#define P6FAMILY_COUNT_RANGE_START	0x80000000
/* note that the bit shift technique is used instead of ULL for watcom cmpiler */
#define P6FAMILY_COUNT_RANGE_END	(((uint64_t)0xFF << 32) | (uint64_t)0x7FFFFFFF)
#define P6FAMILY_COUNT_RANGE_MAX	(((uint64_t)0xFF << 32) | (uint64_t)0xFFFFFFFF)
static void rdecl save_perfregs_p6family( X86_PERFREGS *pcr )
{
	pcr->p6family.PerfEvtSel0 = rdmsr( 390 );
	pcr->p6family.PerfEvtSel1 = rdmsr( 391 );

	if ((pcr->p6family.PerfCtr0 < P6FAMILY_COUNT_RANGE_START) ||
		(pcr->p6family.PerfCtr0 > P6FAMILY_COUNT_RANGE_END)) {
		/* Restore was in "valid" write range. */
		pcr->p6family.PerfCtr0 = rdmsr( 193 );
	} else {
		/* Restore stored a "0" in counter instead
		of real value.  Sum instead of read. Ensure
		value doesn't cross 40bit  boundary. */
		pcr->p6family.PerfCtr0 += rdmsr( 193 );
		if (pcr->p6family.PerfCtr0 > P6FAMILY_COUNT_RANGE_MAX) {
			pcr->p6family.PerfCtr0 -= P6FAMILY_COUNT_RANGE_MAX;
		}
	}


	if ((pcr->p6family.PerfCtr1 < P6FAMILY_COUNT_RANGE_START) ||
		(pcr->p6family.PerfCtr1 > P6FAMILY_COUNT_RANGE_END)) {
		pcr->p6family.PerfCtr1 = rdmsr( 194 );
	} else {
		pcr->p6family.PerfCtr1 += rdmsr( 194 );
		if (pcr->p6family.PerfCtr1 > P6FAMILY_COUNT_RANGE_MAX) {
			pcr->p6family.PerfCtr1 -= P6FAMILY_COUNT_RANGE_MAX;
		}
	}
}

static void rdecl save_perfregs_pentium4xeon(X86_PERFREGS *pcr)
{

	pcr->pentium4xeon.bpu_counter[0] = rdmsr(768);
	pcr->pentium4xeon.bpu_counter[1] = rdmsr(769);
	pcr->pentium4xeon.bpu_counter[2] = rdmsr(770);
	pcr->pentium4xeon.bpu_counter[3] = rdmsr(771);
	pcr->pentium4xeon.ms_counter[0] = rdmsr(772);
	pcr->pentium4xeon.ms_counter[1] = rdmsr(773);
	pcr->pentium4xeon.ms_counter[2] = rdmsr(774);
	pcr->pentium4xeon.ms_counter[3] = rdmsr(775);
	pcr->pentium4xeon.flame_counter[0] = rdmsr(776);
	pcr->pentium4xeon.flame_counter[1] = rdmsr(777);
	pcr->pentium4xeon.flame_counter[2] = rdmsr(778);
	pcr->pentium4xeon.flame_counter[3] = rdmsr(779);
	pcr->pentium4xeon.iq_counter[0] = rdmsr(780);
	pcr->pentium4xeon.iq_counter[1] = rdmsr(781);
	pcr->pentium4xeon.iq_counter[2] = rdmsr(782);
	pcr->pentium4xeon.iq_counter[3] = rdmsr(783);
	pcr->pentium4xeon.iq_counter[4] = rdmsr(784);
	pcr->pentium4xeon.iq_counter[5] = rdmsr(785);

	pcr->pentium4xeon.bpu_cccr[0] = rdmsr(864);
	pcr->pentium4xeon.bpu_cccr[1] = rdmsr(865);
	pcr->pentium4xeon.bpu_cccr[2] = rdmsr(866);
	pcr->pentium4xeon.bpu_cccr[3] = rdmsr(867);
	pcr->pentium4xeon.ms_cccr[0] = rdmsr(868);
	pcr->pentium4xeon.ms_cccr[1] = rdmsr(869);
	pcr->pentium4xeon.ms_cccr[2] = rdmsr(870);
	pcr->pentium4xeon.ms_cccr[3] = rdmsr(871);
	pcr->pentium4xeon.flame_cccr[0] = rdmsr(872);
	pcr->pentium4xeon.flame_cccr[1] = rdmsr(873);
	pcr->pentium4xeon.flame_cccr[2] = rdmsr(874);
	pcr->pentium4xeon.flame_cccr[3] = rdmsr(875);
	pcr->pentium4xeon.iq_cccr[0] = rdmsr(876);
	pcr->pentium4xeon.iq_cccr[1] = rdmsr(877);
	pcr->pentium4xeon.iq_cccr[2] = rdmsr(878);
	pcr->pentium4xeon.iq_cccr[3] = rdmsr(879);
	pcr->pentium4xeon.iq_cccr[4] = rdmsr(880);
	pcr->pentium4xeon.iq_cccr[5] = rdmsr(881);

	pcr->pentium4xeon.bsu_escr[0] = rdmsr(928);
	pcr->pentium4xeon.bsu_escr[1] = rdmsr(929);
	pcr->pentium4xeon.fsb_escr[0] = rdmsr(930);
	pcr->pentium4xeon.fsb_escr[1] = rdmsr(931);
	pcr->pentium4xeon.firm_escr[0] = rdmsr(932);
	pcr->pentium4xeon.firm_escr[1] = rdmsr(933);
	pcr->pentium4xeon.flame_escr[0] = rdmsr(933);
	pcr->pentium4xeon.flame_escr[1] = rdmsr(934);
	pcr->pentium4xeon.dac_escr[0] = rdmsr(936);
	pcr->pentium4xeon.dac_escr[1] = rdmsr(937);
	pcr->pentium4xeon.mob_escr[0] = rdmsr(938);
	pcr->pentium4xeon.mob_escr[1] = rdmsr(939);
	pcr->pentium4xeon.pmh_escr[0] = rdmsr(940);
	pcr->pentium4xeon.pmh_escr[1] = rdmsr(941);
	pcr->pentium4xeon.saat_escr[0] = rdmsr(942);
	pcr->pentium4xeon.saat_escr[1] = rdmsr(943);
	pcr->pentium4xeon.u2l_escr[0] = rdmsr(944);
	pcr->pentium4xeon.u2l_escr[1] = rdmsr(945);
	pcr->pentium4xeon.bpu_escr[0] = rdmsr(946);
	pcr->pentium4xeon.bpu_escr[1] = rdmsr(947);
	pcr->pentium4xeon.is_escr[0] = rdmsr(948);
	pcr->pentium4xeon.is_escr[1] = rdmsr(949);
	pcr->pentium4xeon.itlb_escr[0] = rdmsr(950);
	pcr->pentium4xeon.itlb_escr[1] = rdmsr(951);
	pcr->pentium4xeon.cru_escr[0] = rdmsr(952);
	pcr->pentium4xeon.cru_escr[1] = rdmsr(953);
	pcr->pentium4xeon.iq_escr[0] = rdmsr(954);
	pcr->pentium4xeon.iq_escr[1] = rdmsr(955);
	pcr->pentium4xeon.rat_escr[0] = rdmsr(956);
	pcr->pentium4xeon.rat_escr[1] = rdmsr(957);
	pcr->pentium4xeon.ssu_escr0 = rdmsr(958);
	pcr->pentium4xeon.ms_escr[0] = rdmsr(960);
	pcr->pentium4xeon.ms_escr[1] = rdmsr(961);
	pcr->pentium4xeon.tbpu_escr[0] = rdmsr(962);
	pcr->pentium4xeon.tbpu_escr[1] = rdmsr(963);
	pcr->pentium4xeon.tc_escr[0] = rdmsr(964);
	pcr->pentium4xeon.tc_escr[1] = rdmsr(965);
	pcr->pentium4xeon.ix_escr[0] = rdmsr(968);
	pcr->pentium4xeon.ix_escr[1] = rdmsr(969);
	pcr->pentium4xeon.alf_escr[0] = rdmsr(970);
	pcr->pentium4xeon.alf_escr[1] = rdmsr(971);
	pcr->pentium4xeon.cru_escr[2] = rdmsr(972);
	pcr->pentium4xeon.cru_escr[3] = rdmsr(973);
	pcr->pentium4xeon.cru_escr[4] = rdmsr(992);
	pcr->pentium4xeon.cru_escr[5] = rdmsr(993);
}



static void rdecl restore_perfregs_pentium( X86_PERFREGS *pcr )
{
	wrmsr( 18, pcr->pentium.ctr0 );
	wrmsr( 19, pcr->pentium.ctr1 );
	wrmsr( 17, pcr->pentium.cesr );
}

static void rdecl restore_perfregs_p6family( X86_PERFREGS *pcr )
{
	if ((pcr->p6family.PerfCtr0 < P6FAMILY_COUNT_RANGE_START) ||
		(pcr->p6family.PerfCtr0 > P6FAMILY_COUNT_RANGE_END)) {
		wrmsr(193, pcr->p6family.PerfCtr0);
	} else {
		wrmsr(193,0);
	}

	if ((pcr->p6family.PerfCtr1 < P6FAMILY_COUNT_RANGE_START) ||
		(pcr->p6family.PerfCtr1 > P6FAMILY_COUNT_RANGE_END)) {
		wrmsr(194, pcr->p6family.PerfCtr1);
	} else {
		wrmsr(194, 0);
	}

	wrmsr( 390, pcr->p6family.PerfEvtSel0 );
	wrmsr( 391, pcr->p6family.PerfEvtSel1 );
}




static void rdecl restore_perfregs_pentium4xeon(X86_PERFREGS *pcr)
{
	wrmsr(768 ,pcr->pentium4xeon.bpu_counter[0]);
	wrmsr(769 ,pcr->pentium4xeon.bpu_counter[1]);
	wrmsr(770 ,pcr->pentium4xeon.bpu_counter[2]);
	wrmsr(771 ,pcr->pentium4xeon.bpu_counter[3]);
	wrmsr(772 ,pcr->pentium4xeon.ms_counter[0]);
	wrmsr(773 ,pcr->pentium4xeon.ms_counter[1]);
	wrmsr(774 ,pcr->pentium4xeon.ms_counter[2]);
	wrmsr(775 ,pcr->pentium4xeon.ms_counter[3]);
	wrmsr(776 ,pcr->pentium4xeon.flame_counter[0]);
	wrmsr(777 ,pcr->pentium4xeon.flame_counter[1]);
	wrmsr(778 ,pcr->pentium4xeon.flame_counter[2]);
	wrmsr(779 ,pcr->pentium4xeon.flame_counter[3]);
	wrmsr(780 ,pcr->pentium4xeon.iq_counter[0]);
	wrmsr(781 ,pcr->pentium4xeon.iq_counter[1]);
	wrmsr(782 ,pcr->pentium4xeon.iq_counter[2]);
	wrmsr(783 ,pcr->pentium4xeon.iq_counter[3]);
	wrmsr(784 ,pcr->pentium4xeon.iq_counter[4]);
	wrmsr(785 ,pcr->pentium4xeon.iq_counter[5]);


	wrmsr(928 ,pcr->pentium4xeon.bsu_escr[0]);
	wrmsr(929 ,pcr->pentium4xeon.bsu_escr[1]);
	wrmsr(930 ,pcr->pentium4xeon.fsb_escr[0]);
	wrmsr(931 ,pcr->pentium4xeon.fsb_escr[1]);
	wrmsr(932 ,pcr->pentium4xeon.firm_escr[0]);
	wrmsr(933 ,pcr->pentium4xeon.firm_escr[1]);
	wrmsr(934 ,pcr->pentium4xeon.flame_escr[0]);
	wrmsr(935 ,pcr->pentium4xeon.flame_escr[1]);
	wrmsr(936 ,pcr->pentium4xeon.dac_escr[0]);
	wrmsr(937 ,pcr->pentium4xeon.dac_escr[1]);
	wrmsr(938 ,pcr->pentium4xeon.mob_escr[0]);
	wrmsr(939 ,pcr->pentium4xeon.mob_escr[1]);
	wrmsr(940 ,pcr->pentium4xeon.pmh_escr[0]);
	wrmsr(941 ,pcr->pentium4xeon.pmh_escr[1]);
	wrmsr(942 ,pcr->pentium4xeon.saat_escr[0]);
	wrmsr(943 ,pcr->pentium4xeon.saat_escr[1]);
	wrmsr(944 ,pcr->pentium4xeon.u2l_escr[0]);
	wrmsr(945 ,pcr->pentium4xeon.u2l_escr[1]);
	wrmsr(946 ,pcr->pentium4xeon.bpu_escr[0]);
	wrmsr(947 ,pcr->pentium4xeon.bpu_escr[1]);
	wrmsr(948 ,pcr->pentium4xeon.is_escr[0]);
	wrmsr(949 ,pcr->pentium4xeon.is_escr[1]);
	wrmsr(950 ,pcr->pentium4xeon.itlb_escr[0]);
	wrmsr(951 ,pcr->pentium4xeon.itlb_escr[1]);
	wrmsr(952 ,pcr->pentium4xeon.cru_escr[0]);
	wrmsr(953 ,pcr->pentium4xeon.cru_escr[1]);
	wrmsr(954 ,pcr->pentium4xeon.iq_escr[0]);
	wrmsr(955 ,pcr->pentium4xeon.iq_escr[1]);
	wrmsr(956 ,pcr->pentium4xeon.rat_escr[0]);
	wrmsr(957 ,pcr->pentium4xeon.rat_escr[1]);
	wrmsr(958 ,pcr->pentium4xeon.ssu_escr0);
	wrmsr(960 ,pcr->pentium4xeon.ms_escr[0]);
	wrmsr(961 ,pcr->pentium4xeon.ms_escr[1]);
	wrmsr(962 ,pcr->pentium4xeon.tbpu_escr[0]);
	wrmsr(963 ,pcr->pentium4xeon.tbpu_escr[1]);
	wrmsr(964 ,pcr->pentium4xeon.tc_escr[0]);
	wrmsr(965 ,pcr->pentium4xeon.tc_escr[1]);
	wrmsr(968 ,pcr->pentium4xeon.ix_escr[0]);
	wrmsr(969 ,pcr->pentium4xeon.ix_escr[1]);
	wrmsr(970 ,pcr->pentium4xeon.alf_escr[0]);
	wrmsr(971 ,pcr->pentium4xeon.alf_escr[1]);
	wrmsr(972 ,pcr->pentium4xeon.cru_escr[2]);
	wrmsr(973 ,pcr->pentium4xeon.cru_escr[3]);
	wrmsr(992 ,pcr->pentium4xeon.cru_escr[4]);
	wrmsr(993 ,pcr->pentium4xeon.cru_escr[5]);
	
	/* Restore Control registers last so that 
	 * config changes don't take effect until
	 * other registers have been set up. */
	wrmsr(864 ,pcr->pentium4xeon.bpu_cccr[0]);
	wrmsr(865 ,pcr->pentium4xeon.bpu_cccr[1]);
	wrmsr(866 ,pcr->pentium4xeon.bpu_cccr[2]);
	wrmsr(867 ,pcr->pentium4xeon.bpu_cccr[3]);
	wrmsr(868 ,pcr->pentium4xeon.ms_cccr[0]);
	wrmsr(869 ,pcr->pentium4xeon.ms_cccr[1]);
	wrmsr(870 ,pcr->pentium4xeon.ms_cccr[2]);
	wrmsr(871 ,pcr->pentium4xeon.ms_cccr[3]);
	wrmsr(872 ,pcr->pentium4xeon.flame_cccr[0]);
	wrmsr(873 ,pcr->pentium4xeon.flame_cccr[1]);
	wrmsr(874 ,pcr->pentium4xeon.flame_cccr[2]);
	wrmsr(875 ,pcr->pentium4xeon.flame_cccr[3]);
	wrmsr(876 ,pcr->pentium4xeon.iq_cccr[0]);
	wrmsr(877 ,pcr->pentium4xeon.iq_cccr[1]);
	wrmsr(878 ,pcr->pentium4xeon.iq_cccr[2]);
	wrmsr(879 ,pcr->pentium4xeon.iq_cccr[3]);
	wrmsr(880 ,pcr->pentium4xeon.iq_cccr[4]);
	wrmsr(881 ,pcr->pentium4xeon.iq_cccr[5]);

}



#ifdef VARIANT_instr

/* IFDEF to remove "unused" warning for non-instrumented kernel. */

#define P4XEON_OVF		(1U<<31)
/* Mask off both OVF_PMI0 and OVF_PMI1 in case HyperThreading being used*/
#define P4XEON_OVF_PMI	(3U<<26) 


static const struct sigevent *
perfregs_handler_pentium4xeon(void *dummy, int id) {
	THREAD *thp = actives_pcr[RUNCPU];
	
	if ( thp != NULL ) {
		X86_PERFREGS *pcr = (X86_PERFREGS *)thp->cpu.pcr;


		/* If a counter has overflowed, ensure that the
			interrupt is masked off to prevent an interrupt
			being generated again on restore. */

		if (pcr->pentium4xeon.bpu_cccr[0] & P4XEON_OVF) {
			pcr->pentium4xeon.bpu_cccr[0] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.bpu_cccr[1] & P4XEON_OVF) {
			pcr->pentium4xeon.bpu_cccr[1] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.bpu_cccr[2] & P4XEON_OVF) {
			pcr->pentium4xeon.bpu_cccr[2] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.bpu_cccr[3] & P4XEON_OVF) {
			pcr->pentium4xeon.bpu_cccr[3] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.bpu_cccr[0] & P4XEON_OVF) {
			pcr->pentium4xeon.bpu_cccr[0] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.ms_cccr[0] & P4XEON_OVF) {
			pcr->pentium4xeon.ms_cccr[0] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.ms_cccr[1] & P4XEON_OVF) {
			pcr->pentium4xeon.ms_cccr[1] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.ms_cccr[2] & P4XEON_OVF) {
			pcr->pentium4xeon.ms_cccr[2] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.ms_cccr[3] & P4XEON_OVF) {
			pcr->pentium4xeon.ms_cccr[3] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.flame_cccr[0] & P4XEON_OVF) {
			pcr->pentium4xeon.flame_cccr[0] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.flame_cccr[1] & P4XEON_OVF) {
			pcr->pentium4xeon.flame_cccr[1] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.flame_cccr[2] & P4XEON_OVF) {
			pcr->pentium4xeon.flame_cccr[2] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.flame_cccr[3] & P4XEON_OVF) {
			pcr->pentium4xeon.flame_cccr[3] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.iq_cccr[0] & P4XEON_OVF) {
			pcr->pentium4xeon.iq_cccr[0] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.iq_cccr[1] & P4XEON_OVF) {
			pcr->pentium4xeon.iq_cccr[1] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.iq_cccr[2] & P4XEON_OVF) {
			pcr->pentium4xeon.iq_cccr[2] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.iq_cccr[3] & P4XEON_OVF) {
			pcr->pentium4xeon.iq_cccr[3] &= ~P4XEON_OVF_PMI;
		}
		if (pcr->pentium4xeon.iq_cccr[4] & P4XEON_OVF) {
			pcr->pentium4xeon.iq_cccr[4] &= ~P4XEON_OVF_PMI;
		}
	}

	return NULL;
}


#define P6FAMILY_OVF_INT		(1<<20)

static const struct sigevent *
perfregs_handler_p6family(void *dummy, int id) {
	THREAD *thp = actives_pcr[RUNCPU];
	

	if ( thp != NULL ) {
		X86_PERFREGS *pcr = (X86_PERFREGS *)thp->cpu.pcr;

		pcr->p6family.PerfEvtSel0 &= ~P6FAMILY_OVF_INT;
		pcr->p6family.PerfEvtSel1 &= ~P6FAMILY_OVF_INT;

	}

	return NULL;
}




static void attach_irq(void)
{
	int level;

	switch(cpu_perfreg_id()) {
		case PERFREGS_MAKEID(PERFREGS_CPUID_X86,PERFREGS_PARTID_PENTIUM):

			break;
		case PERFREGS_MAKEID(PERFREGS_CPUID_X86,PERFREGS_PARTID_P6FAMILY):
			lock_kernel();
			level = get_interrupt_level( NULL, X86_INTR_APIC_PERFORMANCE);
			if ( level >= 0 ) {
				(void) interrupt_attach( level, perfregs_handler_p6family, 0, 0 );
			}
			break;
		case PERFREGS_MAKEID(PERFREGS_CPUID_X86,PERFREGS_PARTID_PENTIUM4XEON):
			lock_kernel();
			level = get_interrupt_level( NULL, X86_INTR_APIC_PERFORMANCE);
			if ( level >= 0 ) {
				(void) interrupt_attach( level, perfregs_handler_pentium4xeon, 0, 0 );
			}
			break;
		default:
			break;
	}
}

#endif

static void rdecl choose_save_perfregs( X86_PERFREGS *pcr );
static void rdecl choose_restore_perfregs( X86_PERFREGS *pcr );

static void rdecl (*save_perfregs_p)(X86_PERFREGS *pcr) = choose_save_perfregs;
static void rdecl (*restore_perfregs_p)(X86_PERFREGS *pcr) = choose_restore_perfregs;

static void rdecl choose_save_perfregs( X86_PERFREGS *pcr )
{
	switch(cpu_perfreg_id()) {
		case PERFREGS_MAKEID(PERFREGS_CPUID_X86,PERFREGS_PARTID_PENTIUM):
			save_perfregs_p = save_perfregs_pentium;
			break;
		case PERFREGS_MAKEID(PERFREGS_CPUID_X86,PERFREGS_PARTID_P6FAMILY):
			save_perfregs_p = save_perfregs_p6family;
			break;
		case PERFREGS_MAKEID(PERFREGS_CPUID_X86,PERFREGS_PARTID_PENTIUM4XEON):
			save_perfregs_p = save_perfregs_pentium4xeon;
			break;
		default:
			save_perfregs_p = NULL;
			break;
	}
	if (save_perfregs_p) {
		(save_perfregs_p)(pcr);
	}
}

static void rdecl choose_restore_perfregs( X86_PERFREGS *pcr )
{
	switch(cpu_perfreg_id()) {
		case PERFREGS_MAKEID(PERFREGS_CPUID_X86,PERFREGS_PARTID_PENTIUM):
			restore_perfregs_p = restore_perfregs_pentium;
			break;
		case PERFREGS_MAKEID(PERFREGS_CPUID_X86,PERFREGS_PARTID_P6FAMILY):
			restore_perfregs_p = restore_perfregs_p6family;
			break;
		case PERFREGS_MAKEID(PERFREGS_CPUID_X86,PERFREGS_PARTID_PENTIUM4XEON):
			restore_perfregs_p = restore_perfregs_pentium4xeon;
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
/* Ifdef alloc to remove warning "not used" when compiling with
non-instrumented kernel. */

static int rdecl cpu_alloc_perfregs( THREAD *thp )
{
	X86_PERFREGS *pcr;
	
	if((pcr = _smalloc(sizeof(*pcr))) == NULL) {
		return ENOMEM;
	}
	memset( pcr, 0, sizeof(*pcr) );
	thp->cpu.pcr = pcr;
	return EOK;
}
#endif

/* global functions */
_Uint32t cpu_perfreg_id(void)
{
	_Uint32t	c, family;
	char		id[13];
	static unsigned	perfreg_id = 0, features = 0;

	if ( perfreg_id != 0 ) {
		return perfreg_id;
	}

	if ( hw_cpuid(0,id,0) < 1 ) {
		return 0;
	}
	
	id[sizeof id - 1] = 0;

	c = hw_cpuid(1,0,&features);

	if ( !(features & X86_FEATURE_MSR) ) { /* must have rdmsr/wrmsr */
		return 0;
	}
	
	family = (c >> 8) & 0xf;

	c = 0;
	if ( strncmp( id, "GenuineIntel", 12 ) == 0 ) {
		switch(family) {
			case 5: /* Pentium */
				c = PERFREGS_MAKEID(PERFREGS_CPUID_X86,PERFREGS_PARTID_PENTIUM);
				break;
			case 6: /* Pentium6 */
				c = PERFREGS_MAKEID(PERFREGS_CPUID_X86,PERFREGS_PARTID_P6FAMILY);
				break;
			case 15: /* Xeon */
				c = PERFREGS_MAKEID(PERFREGS_CPUID_X86,PERFREGS_PARTID_PENTIUM4XEON);
				break;
			default:
				break;
		}
	}
	perfreg_id = c;
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


void rdecl cpu_save_perfregs(void *vpcr)
{
	X86_PERFREGS *pcr = vpcr;

	if ((pcr->id & PERFREGS_ENABLED_FLAG) && 
		!(pcr->id & PERFREGS_SAVED_FLAG) && save_perfregs_p ) {

		pcr->id |= PERFREGS_SAVED_FLAG;
		(save_perfregs_p)(pcr);
		(restore_perfregs_p)(&disabled_perfregs);

	}
}

void rdecl cpu_restore_perfregs(void *vpcr)
{
	X86_PERFREGS *pcr = vpcr;
	
	if ((pcr->id & PERFREGS_ENABLED_FLAG) && restore_perfregs_p ) {
		(restore_perfregs_p)(pcr);
		pcr->id &= ~PERFREGS_SAVED_FLAG;
	}
}

int	rdecl cpu_debug_set_perfregs(THREAD *thp, debug_perfreg_t *regs)
{

#ifdef VARIANT_instr
	int status;
	static int attached = 0;
	if ( thp->cpu.pcr == &disabled_perfregs ) {
		if ( (status = cpu_alloc_perfregs(thp)) != EOK ){
			return status;
		}
	}
	memcpy( thp->cpu.pcr, regs, sizeof(*thp->cpu.pcr) );
	if ( !attached ) {
		attach_irq();
		attached = 1;
	}	

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

__SRCVERSION("cpu_perfregs.c $Rev: 198534 $");
