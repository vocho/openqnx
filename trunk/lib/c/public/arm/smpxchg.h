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
 *  arm/smpxchg.h
 *
 *  Code sequences for SMP atomic exchanging of mutex stuff.
 *

 */

#ifndef __SMPXCHG_H_INCLUDED
#define __SMPXCHG_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

/*
 * _smp_cmpxchg()/_smp_xchg() are not inlined on ARM:
 *	- libc provides versions for use by unprivileged code that use a special
 *    swi trap to emulate the operation.
 *	- libnto provides versions for use by privileged code that directly
 *    emulates the operation.
 */
extern unsigned _smp_cmpxchg(volatile unsigned *__dst, unsigned __cmp, unsigned __src);
extern unsigned _smp_xchg(volatile unsigned *__dst, unsigned __src);

/*
 * _mux_smp_cmpxchg() is used only for mutexes
 */
extern unsigned	_mux_smp_cmpxchg(volatile unsigned *__dst, unsigned __cmp, unsigned __src);

#define __mutex_smp_cmpxchg(d, c, s)	_mux_smp_cmpxchg((d), (c), (s))

__END_DECLS

#endif

/* __SRCVERSION("smpxchg.h $Rev: 153052 $"); */
