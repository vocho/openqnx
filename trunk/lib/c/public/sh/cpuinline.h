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
 *  sh/cpuinline.h
 *

 *
 *  All symbols defined through this header file must not pollute the namespace.
 *  i.e. the symbols will start with two underscores or one underscore and
 *  a capital letter.
 */
#ifndef _SH_CPUINLINE_INCLUDED
#define _SH_CPUINLINE_INCLUDED

#define __cpu_membarrier() ({ __asm__ __volatile__ ("nop" : : : "memory"); })

#if defined(__GNUC__)

#define __CPU_ENDIAN_RET16
#define __cpu_endian_ret16(__x)	(__builtin_constant_p(__x) \
								? ((((__x) >> 8) & 0xFF) | (((__x) & 0xFF) << 8)) \
								: ({ \
									_Uint16t __reg; \
									__asm__( \
										"swap.b %0, %0;" \
										: "=r" (__reg) \
										: "0" (__x) \
									); __reg; \
								  }))

#define __CPU_ENDIAN_RET32
#define __cpu_endian_ret32(__x)	(__builtin_constant_p(__x) \
								? ((((__x) >> 24) & 0xFF) | (((__x) >> 8) & 0xFF00) | (((__x) & 0xFF00) << 8) | (((__x) & 0xFF) << 24)) \
								: ({ \
									_Uint32t __reg; \
									__asm__( \
										"swap.b %0, %0;" \
										"swap.w %0, %0;" \
										"swap.b %0, %0;" \
										: "=r" (__reg) \
										: "0" (__x) \
									); __reg; \
								  }))

#define __CPU_ENDIAN_RET64
#define __cpu_endian_ret64(__x)	(__builtin_constant_p(__x) \
								? ((((__x) >> 56) & 0xFF) | (((__x) >> 40) & 0xFF00) | (((__x) >> 24) & 0xFF0000) | (((__x) >>  8) & 0xFF000000) | (((__x) & 0xFF000000) <<  8) | (((__x) & 0xFF0000) << 24) | (((__x) & 0xFF00) << 40) | (((__x) & 0xFF) << 56)) \
								: ({ \
									_Uint64t __value = (__x); \
									_Uint64t *__vp = &(__value); \
									_Uint64t __rv; \
									_Uint64t *__rp = &__rv; \
									_Uint32t __gpr = (__gpr); \
									__asm__( \
										"mov.l @(4,%1),%2;" \
										"swap.b %2, %2;" \
										"swap.w %2, %2;" \
										"swap.b %2, %2;" \
										"mov.l %2, @(0,%0);" \
										"mov.l @(0,%1), %2;" \
										"swap.b %2, %2;" \
										"swap.w %2, %2;" \
										"swap.b %2, %2;" \
										"mov.l %2, @(4,%0);" \
										: \
										: "r" (__rp), "r" (__vp), "r" (__gpr) \
									); __rv; \
								  }))

#endif
#endif

/* __SRCVERSION("cpuinline.h $Rev: 153052 $"); */
