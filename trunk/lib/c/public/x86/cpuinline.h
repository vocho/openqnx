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
 *  x86/cpuinline.h
 *
 *
 *  All symbols defined through this header file must not pollute the namespace.
 *  i.e. the symbols will start with two underscores or one underscore and
 *  a capital letter.
 */
#ifndef _X86_CPUINLINE_INCLUDED
#define _X86_CPUINLINE_INCLUDED

#if defined(__GNUC__) || defined(__INTEL_COMPILER) 
#define __cpu_membarrier() ({ extern unsigned __cpu_flags; if(__cpu_flags & (1 << 15)) __asm__ __volatile__ ("mfence"); else __asm __volatile__("lock; orb $0,0(%esp)"); })
#elif defined(__WATCOMC__)
extern unsigned __cpu_flags;
extern void	__cpu_membarrier(void);
#pragma aux __cpu_membarrier = \
			"test	dword ptr __cpu_flags,0x8000" \
			"je		skip"	\
			"db 0x0f, 0xae, 0xf0" /* mfence */ \
			"jmp skip2" \
			"skip:"	\
			"lock or byte ptr 0[esp],0" \
			"skip2:"
#else
#error Compiler not defined.
#endif

#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#define __CPU_ENDIAN_RET16
#define __cpu_endian_ret16(__x)	(__builtin_constant_p(__x) \
								? ((((__x) >> 8) & 0xFF) | (((__x) & 0xFF) << 8)) \
								: ({_Uint16t __reg; __asm__("xchgb %b0, %h0" : "=q" (__reg) : "0" (__x)); __reg;}))
#define __CPU_ENDIAN_RET32
#define __cpu_endian_ret32(__x)	(__builtin_constant_p(__x) \
								? ((((__x) >> 24) & 0xFF) | (((__x) >> 8) & 0xFF00) | (((__x) & 0xFF00) << 8) | (((__x) & 0xFF) << 24)) \
								: ({_Uint32t __reg = (__x); __asm__("xchgb %b0, %h0\nrorl $16, %0\nxchgb %b0, %h0" : "=q" (__reg) : "0" (__reg)); __reg;}))
#define __CPU_ENDIAN_RET64
#define __cpu_endian_ret64(__x) (__builtin_constant_p(__x) \
								? ((((__x) >> 56) & 0xFF) | (((__x) >> 40) & 0xFF00) | (((__x) >> 24) & 0xFF0000) | (((__x) >>  8) & 0xFF000000) | (((__x) & 0xFF000000) <<  8) | (((__x) & 0xFF0000) << 24) | (((__x) & 0xFF00) << 40) | (((__x) & 0xFF) << 56)) \
								: ({_Uint64t __reg; __asm__("xchgb %b0, %h0\nrorl $16, %0\nxchgb %b0, %h0\nxchg %%eax, %%edx\nxchgb %b0, %h0\nrorl $16, %0\nxchgb %b0, %h0" : "=A" (__reg) : "0" (__x)); __reg;}))
#endif

#define __CPU_UNALIGNED_RET16
#define __cpu_unaligned_ret16(__p) (*(_Uint16t volatile *)(__p))
#define __CPU_UNALIGNED_RET32
#define __cpu_unaligned_ret32(__p) (*(_Uint32t volatile *)(__p))
#define __CPU_UNALIGNED_RET64
#define __cpu_unaligned_ret64(__p) (*(_Uint64t volatile *)(__p))
#define __CPU_UNALIGNED_PUT16
#define __cpu_unaligned_put16(__p, __x) (*(_Uint16t volatile *)(__p) = (__x))
#define __CPU_UNALIGNED_PUT32
#define __cpu_unaligned_put32(__p, __x) (*(_Uint32t volatile *)(__p) = (__x))
#define __CPU_UNALIGNED_PUT64
#define __cpu_unaligned_put64(__p, __x) (*(_Uint64t volatile *)(__p) = (__x))

#endif

/* __SRCVERSION("cpuinline.h $Rev: 212414 $"); */
