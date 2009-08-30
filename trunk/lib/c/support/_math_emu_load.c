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




#include <inttypes.h>
#include <malloc.h>
#include <signal.h>
#include <pthread.h>
#include <dlfcn.h>
#include <sys/neutrino.h>
#include <sys/debug.h>

/*
	Note that this version number needs to match the libc.so version
	number and when it changes, you also need to change the IMAGE_SUFF_DLL
	macro in fpemu's common.mk. Some day I'll figure out how to make it
	happen all automatically.
*/
#define VERSION_NUMBER	"2"

#ifdef __PIC__
extern unsigned (*_emulator_callout)(unsigned sigcode, void **pdata, void *regs);

static void emu_init(void) {
	void						*dll;

	if((dll = dlopen("fpemu.so." VERSION_NUMBER, RTLD_NOW))) {
		unsigned (*func)(unsigned sigcode, void **pdata, void *regs);

		if((func = dlsym(dll, "_math_emulator"))) {
			/*
			 * This must be atomic on all processors or there may be a problem!!!
			 */
			_emulator_callout = func;
		} else {
			dlclose(dll);
		}
	}
}

// We don't support floating point (emulated or hardware) in signal
// handlers. If we did decide to add support, this code would have
// a race condition with a thread doing an FP op and then a signal
// going off while we're trying to load the fpemu.so.2 (e.g. a timer
// expiring). If the signal handler also does an FP op, we would
// come back in here and the pthread_once() would error out with EDEADLK.
// Defining the FPEMU_IN_SIGHANDLER_SUPPORT macro will block the signal
// until we've finished loading and come out of the emu_init() function.
#undef FPEMU_IN_SIGHANDLER_SUPPORT

unsigned _math_emulator(unsigned sigcode, void **pdata, void *regs) {
	static pthread_once_t		emu_once = PTHREAD_ONCE_INIT;
#ifdef FPEMU_IN_SIGHANDLER_SUPPORT	
	sigset_t					oldset, newset;

	sigfillset(&newset);
	SignalProcmask(0, 0, SIG_SETMASK, &newset, &oldset);
#endif
	pthread_once(&emu_once, emu_init);
#ifdef FPEMU_IN_SIGHANDLER_SUPPORT	
	SignalProcmask(0, 0, SIG_SETMASK, &oldset, NULL);
#endif	
	return (_emulator_callout != _math_emulator) ? _emulator_callout(sigcode, pdata, regs) : sigcode;
}
#else
unsigned __attribute__((weak)) _math_emulator(unsigned sigcode, void **pdata, void *regs) {
    return sigcode;
}
#endif

__SRCVERSION("_math_emu_load.c $Rev: 153052 $");
