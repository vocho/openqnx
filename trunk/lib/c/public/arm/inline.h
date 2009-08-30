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
 *  arm/inline.h
 *

 */

#ifndef _ARM_INLINE_H_INCLUDED
#define _ARM_INLINE_H_INCLUDED

/*
 *  some handy pragma's for low-level work: 
 */

#ifndef _ARM_INOUT_INCLUDED
#include <arm/inout.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

static __inline__ void __attribute__((__unused__))
ldesp(unsigned __nsp)
{
	__asm__ __volatile__(
		"mov	sp, %0"
		:
		: "r" (__nsp)
	);
}

/*
 * Read the CPUID register
 */
static __inline__ unsigned
arm_cpuid()
{
	unsigned	id;
	__asm__ __volatile__(
		"mrc	p15, 0, %0, c0, c0, 0"
		: "=r" (id)
		:
	);
	return id;
}

/*
 * Return the ARM architecture version:
 * 0 - ARMv3
 * 1 - ARMv4
 */
static __inline__ int
arm_cpuid_arch()
{
	return (arm_cpuid() >> 16) & 255;
}

__END_DECLS

#endif

/* __SRCVERSION("inline.h $Rev: 153052 $"); */
