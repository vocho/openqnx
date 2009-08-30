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

/* All exceptions are now disabled by default (incidentally
   all exceptions disabled is the default state instilled
   by the fninit call) to maintain compatability across
   hw platforms and emulators.

#if defined(__WATCOMC__)
	extern void init_fpu_state(void);
	#pragma aux init_fpu_state =	\
		".8087"						\
		"fninit"					\
		"sub	esp,4"				\
		"fstcw	[esp]"				\
		"fwait"						\
		"and	word ptr [esp],0xfff2"	\
		"fldcw	[esp]"				\
		"fwait"						\
		"add	esp,4"
#elif defined(__GNUC__)
	#define init_fpu_state()	\
		{						\
			short	cw;			\
								\
			asm volatile(		\
				"fninit		;"	\
				"fstcw	%0	;"	\
				"fwait"			\
				: "=m" (cw));	\
			cw &= 0xfff2;		\
			asm volatile(		\
				"fldcw	%0	;"	\
				"fwait"			\
				:				\
				: "m" (cw));	\
		}
#else
	#error compiler not supported
#endif
*/
#if defined(__WATCOMC__)
	extern void init_fpu_state(void);
	#pragma aux init_fpu_state =	\
		".8087"						\
		"fninit"					\
		"fwait"
#elif defined(__GNUC__)
	#define init_fpu_state()	\
		{						\
			asm volatile(		\
				"fninit; "		\
				"fwait   "		\
				: );			\
		}
#else
	#error compiler not supported
#endif

void
fpusave_alloc() {
	THREAD	*act = actives[KERNCPU];

	if(act->fpudata) crash();

	// If no fpu then abort
	if((rdcr0() & X86_MSW_MP_BIT) == 0) {

		usr_fault(SIGFPE + (FPE_NOFPU*256) + (FLTNOFPU*65536), act, KIP(act));
		return;
	}

	// Save any active state
	InterruptDisable();
	if(actives_fpu[KERNCPU] != NULL) {
		cpu_force_fpu_save(actives_fpu[KERNCPU]);
		actives_fpu[KERNCPU] = NULL;
	}
	InterruptEnable();

	// Clear task switched flag so we don't fault touching the FPU.
	clts();

	//Init FPU state and enable Invalid Op, Zero Div, Overflow exceptions.
	init_fpu_state();

	// Allocate a save area.
	if((act->fpudata = object_alloc(NULL, &fpu_souls)) == NULL) {
		usr_fault(SIGFPE + (FPE_NOMEM*256) + (FLTNOFPU*65536), act, KIP(act));
		return;
	}

	act->fpudata = (void *)((unsigned)act->fpudata | (KERNCPU & FPUDATA_CPUMASK) | FPUDATA_BUSY);
	actives_fpu[KERNCPU] = act;
}


const unsigned char fpuerr2code[6] ={
	FPE_FLTINV, FPE_FLTRES, FPE_FLTDIV,
	FPE_FLTOVF, FPE_FLTUND, FPE_FLTRES
};

__SRCVERSION("nano_fpu.c $Rev: 153052 $");
