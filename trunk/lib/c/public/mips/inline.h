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



/*-
 *  mips/inline.h
 *

 */

#ifndef _MIPS_INLINE_H_INCLUDED
#define _MIPS_INLINE_H_INCLUDED

#ifndef _MIPS_INOUT_INCLUDED
#include <mips/inout.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

/*
 * ldesp()
 *	Set our SP to a given value
 */

#if defined(__GNUC__)

static __inline__ void __attribute__((__unused__))
ldesp(_Uintptrt __nsp) {
	__asm__ __volatile__("move $sp, %0"
		: /* No outputs */
		: "r" (__nsp));
}

#elif defined(__MWERKS__)

asm static /*inline*/ void ldesp(_Uintptrt __nsp) {
	or sp, a0, zero
}

#endif

__END_DECLS

#endif

/* __SRCVERSION("inline.h $Rev: 153052 $"); */
