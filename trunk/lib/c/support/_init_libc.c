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




#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/auxv.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include "init.h"
#include "cpucfg.h"
#define EXT
#include "pls.h"
#undef errno		/* Undefined so the main thread uses a global "errno" */

#if defined(__X86__)
  /* X86 uses the fs register for cpupage, so one pointer is not valid */
#elif defined(__MIPS__) \
   || defined(__PPC__) \
   || defined(__ARM__) \
   || defined(__SH__)
  #define WANT_CPUPAGE_PTR
#else
  #error Not configured for CPU
#endif


int								_argc;
char							**_argv;
auxv_t							*_auxv;
int								errno;
char							**environ;
char							*__progname;
struct syspage_entry			*_syspage_ptr;
uint32_t						__cpu_flags;
#if defined(__MIPS__) || defined(__SH__)
volatile unsigned long			*__shadow_imask; //Kludge for MIPS and SH
#endif
#if defined(WANT_CPUPAGE_PTR)
struct cpupage_entry			*_cpupage_ptr;
#endif
#ifdef __WATCOMC__
unsigned char					__8087, __real87;
#endif

// Don't define any new variables in this file - just extern them
// here and put the definitions in their own source files so that
// procnto doesn't keep bringing in the _init_libc() function, which
// it doesn't want. If the variable needs to be set at runtime, inform
// the kernel group so that they can add the necessary gear to the
// kernel initialization code.

extern int						__ealready_value;
extern int						_Multi_threaded;
extern int						__posixly_correct;
extern int						__dir_keep_symlink;

extern void 					__my_thread_exit(void *);


void _init_libc(int argc, char *argv[], char *arge[], auxv_t auxv[], void (*exit_func)(void)) {
	char	*prog;

	if(__progname == NULL) {
		/* Initialize the system page pointer */
		_syspage_ptr = __SysCpupageGet(CPUPAGE_SYSPAGE);
#if defined(__MIPS__)
		__shadow_imask = &_syspage_ptr->un.mips.shadow_imask;
#endif
	
		/* Tell the process manager where our process local storage is */
		(void)__SysCpupageSet(CPUPAGE_PLS, (intptr_t)&_pls);

#if defined(WANT_CPUPAGE_PTR)
		/* Initialize the cpupage pointer if the processor has it at a fixed address */
		_cpupage_ptr = __SysCpupageGet(CPUPAGE_ADDR);
#endif

		/* Initialize the math emulator hook */
		_pls.__mathemulator = _math_emu_stub;

		/* So the kernel always knows what to use for SIGEV_THREAD's */
		LIBC_TLS()->__exitfunc = __my_thread_exit;

		/* extract the cpu flags from syspage so we can get at them faster */
		__cpu_flags = SYSPAGE_ENTRY(cpuinfo)->flags;

#if defined(__SH__)
		if (_syspage_ptr->num_cpu > 1) {
			__shadow_imask = &_cpupage_ptr->un.sh.imask;
		}
		else {
			__shadow_imask = &_syspage_ptr->un.sh.imask;
		}
#endif

#ifdef __WATCOMC__
		/* Watcom uses this for its floating point support routines */
		if(SYSPAGE_ENTRY(cpuinfo)->flags & CPU_FLAG_FPU) {
			__8087 = __real87 = SYSPAGE_ENTRY(cpuinfo)->cpu / 100;
		}
#endif

#if defined(__SH__)
		/* set __cpu_flags for inline ops in case startup didn't set any flags */
		if (_syspage_ptr->num_cpu > 1) {
			__cpu_flags |= SH_CPU_FLAG_SMP;
		}
		if (SH4_PVR_FAM(SYSPAGE_ENTRY(cpuinfo)->cpu) == SH4_PVR_SH4A) {
			__cpu_flags |= SH_CPU_FLAG_MOVLICO;
		}
#endif

		//Get the basename of the program, but don't use basename() - it might
		//modify the argv[0] storage.
		prog = argv[0];
		__progname = strrchr(prog, '/');
		if(__progname == NULL) {
			__progname = prog;
		} else {
			__progname++;
		}

#ifdef __PIC__
		/* This routine will fill _pls with the address of shared objects */
		(void) _dll_list();
#endif

		/* Set up globals for common information */
		_argc = argc;
		_argv = argv;
		environ = arge;
		_auxv = auxv;

		/* check if strict POSIX behaviour is required */
		if (getenv("POSIXLY_CORRECT") != 0) {
			__posixly_correct = 1;
		}
		if (getenv("DIR_KEEP_SYMLINK") != NULL) {
			__dir_keep_symlink = 1;
		}
		if (SYSPAGE_ENTRY(system_private)->private_flags & SYSTEM_PRIVATE_FLAG_EALREADY_NEW) {
			__ealready_value = EALREADY_NEW;
		} else {
			__ealready_value = EALREADY_OLD;
		}
	}

	/* Add exit function for runtime linker if passed */
	if(exit_func) {
		(void)atexit(exit_func);
	}

	/*
	   This is done here so that crt1.S & _init_libc both have the
	   same idea about what the address of the variable 'errno' is.
	*/
	LIBC_TLS()->__errptr = &errno;
	/* Do this last to make sure errno is zero (as per ANSI C 7.1.4) */
	errno = 0;
}

__SRCVERSION("_init_libc.c $Rev: 174656 $");
