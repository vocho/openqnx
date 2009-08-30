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
 *  smpxchg.h     code sequences for SMP atomic exchanging of mutex stuff.
 *

 */

#ifndef __SMPXCHG_H_INCLUDED
#define __SMPXCHG_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

#if defined(__WATCOMC__)

int _smp_cmpxchg(volatile unsigned *__dst, unsigned __cmp, unsigned __new);
#pragma aux _smp_cmpxchg = \
			".486" \
			"lock cmpxchg 0[edx],ecx" \
			parm [edx] [eax] [ecx] \
			modify exact [eax] value [eax];

int _smp_xchg(volatile unsigned *__dst, unsigned __src);
#pragma aux _smp_xchg = "xchg 0[edx],eax" parm [edx] [eax] modify exact [eax] value [eax];

#elif defined(__GNUC__) || defined(__INTEL_COMPILER)

/*
 * Make sure gcc doesn't try to be clever and move things around
 * on us. We need to use _exactly_ the address the user gave us,
 * not some alias that contains the same information.
 */
#ifndef __atomic_fool_gcc
struct __gcc_fool { int __fool[100]; };
#define __atomic_fool_gcc(__x) (*(volatile struct __gcc_fool *)__x)
#endif

static __inline int __attribute__((__unused__)) _smp_cmpxchg(volatile unsigned *__dst, unsigned __cmd, unsigned __new) {
	__asm__ __volatile__(
		"lock; cmpxchgl %3, (%2)"
		:"=m" (__atomic_fool_gcc(__dst)), "=a" (__cmd)
		:"d" (__dst), "c" (__new), "1" (__cmd)
		:"memory");
	return __cmd;
}

static __inline int __attribute__((__unused__)) _smp_xchg(volatile unsigned *__dst, unsigned __src) {
	__asm__ __volatile__(
		"xchgl %0,%1"
		:"=r" (__src)
		:"m" (__atomic_fool_gcc(__dst)), "0" (__src)
		:"memory");
	return __src;
}

#elif defined(__MWERKS__)

#define __MWINL inline __declspec(naked)

static __MWINL int
_smp_cmpxchg(volatile unsigned *__dst, unsigned __cmd, unsigned __new) {
	asm {
		push ecx
		push edx
		mov  edx, __dst
		mov  ecx, __new
		mov  eax, __cmd
		lock: cmpxchg  [edx],ecx
		pop edx
		pop ecx
	}
}

static __MWINL int
_smp_xchg(volatile unsigned *__dst, unsigned __src) {
    asm {
    	push edx
    	mov  edx, __dst
    	mov  eax, __src
	xchg [edx],eax
    }
}

#else
#error Compiler not defined
#endif

__END_DECLS

#endif

/* __SRCVERSION("smpxchg.h $Rev: 153052 $"); */
