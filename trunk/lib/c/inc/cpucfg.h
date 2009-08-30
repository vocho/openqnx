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




#if defined(__X86__)

	#include <x86/inline.h>

	#define in_interrupt()		((_cs() & 0x3) == 0)

	#if defined(__WATCOMC__)
		extern struct _thread_local_storage  *LIBC_TLS();
		#pragma aux LIBC_TLS =\
			"mov eax,fs:0"\
			parm [eax] modify nomemory exact [eax] value [eax];
	#elif defined(__GNUC__)
		static __inline__ struct _thread_local_storage * __attribute__((__unused__,__const__)) LIBC_TLS(void) {
			register struct _thread_local_storage		*__p;
			__asm__ (
				"movl %%fs:0,%0"
				: "=r" (__p):);
			return __p;
		}
	#else
		#error Compiler not supported
	#endif

	#define CONDITION_CYCLES(c)		((unsigned long)(c))

	#define L2V_CHEAT

#elif defined(__MIPS__)

	#include <sys/syspage.h>
	#include <mips/cpu.h>
	#include <mips/priv.h>

	#define	in_interrupt()		(_cpupage_ptr->state & 0x3)

	extern struct cpupage_entry *_cpupage_ptr;
	#define LIBC_TLS()	(_cpupage_ptr->tls)

	/*
	 * ClockCycles returns count register in high 32 bits
	 */
	#define CONDITION_CYCLES(c)		((unsigned long)((c) >> 32))

	#define L2V_CHEAT

#elif defined(__PPC__)

	#include <sys/syspage.h>
	#include <ppc/cpu.h>
	#include <ppc/inline.h>

	#define	in_interrupt()		(_cpupage_ptr->state & 0x3)

	extern struct cpupage_entry *_cpupage_ptr;
	#define LIBC_TLS()	(_cpupage_ptr->tls)

	#define CONDITION_CYCLES(c)		((unsigned long)(c))

	#undef L2V_CHEAT

#elif defined(__SH__)

	#include <sys/syspage.h>
	#include <sh/cpu.h>
	#include <sh/inline.h>

	#define	in_interrupt()		(_cpupage_ptr->state & 0x3)

	extern struct cpupage_entry *_cpupage_ptr;
	#define LIBC_TLS()	(_cpupage_ptr->tls)

	#define CONDITION_CYCLES(c)		((unsigned long)((c) >> 32))

	#undef L2V_CHEAT

#elif defined(__ARM__)

	#include <sys/syspage.h>

	#define	in_interrupt()		(_cpupage_ptr->state & 0x3)

	extern struct cpupage_entry *_cpupage_ptr;
	#define LIBC_TLS()	(_cpupage_ptr->tls)

	#define CONDITION_CYCLES(c)		((unsigned long)(c))

	#define L2V_CHEAT

#else

	#error not configured for system

#endif

/* __SRCVERSION("cpucfg.h $Rev: 153052 $"); */
