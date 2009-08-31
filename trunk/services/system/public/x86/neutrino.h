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
 *  x86/neutrino.h
 *

 */

#ifndef __NEUTRINO_H_INCLUDED
#error x86/neutrino.h should not be included directly.
#endif

#define CLOCKCYCLES_INCR_BIT	0

#if defined(__WATCOMC__)

extern void __inline_InterruptEnable(void);
extern void __inline_InterruptDisable(void);
extern void __inline_InterruptLock(struct intrspin *);
extern void __inline_InterruptUnlock(struct intrspin *);
extern unsigned __inline_InterruptStatus(void);
extern void __inline_DebugBreak(void);
extern void __inline_DebugKDBreak(void);
extern void __inline_DebugKDOutput(const char *__text, _CSTD size_t __len);

/*
 * High speed time calculation
 */

_Uint64t ClockCycles(void);
#pragma aux ClockCycles = \
	".386p" \
	"db 0fh,31h" \
	modify exact nomemory [eax edx];

#pragma aux __inline_InterruptEnable =\
	"sti"\
	"mov eax,eax"\
	parm [ ]\
	modify exact [ ];

#pragma aux __inline_InterruptDisable =\
	"cli"\
	parm [ ]\
	modify exact [ ];

extern unsigned __cpu_flags;
#pragma aux __inline_InterruptUnlock =\
	"test	dword ptr __cpu_flags,0x8000" \
	"je		skip"	\
	"db 0x0f, 0xae, 0xf0" \
	"skip:"	\
	"mov dword ptr [eax],0"\
	"sti"\
	parm [eax]\
	modify exact [ ];

#pragma aux __inline_InterruptLock =\
	"cli"\
	"L1:"\
	"mov	edx,1"\
	"xchg	edx,[eax]"\
	"test	edx,edx"\
	"jz		L2"\
	"byte 0xf3,0x90" \
	"jmp	L1"\
	"L2:"\
	parm [eax]\
	modify exact [edx];

#pragma aux __inline_InterruptStatus =\
	".386"\
	"pushfd"\
	"pop eax"\
	"and eax,0x200"\
	parm nomemory [] modify exact [eax] value [eax]

#pragma aux __inline_DebugBreak = \
	"int 0x03"\
	parm nomemory []\
	modify nomemory exact [];

#pragma aux __inline_DebugKDBreak = \
	"int 0x20"\
	parm nomemory []\
	modify nomemory exact [];

#pragma aux __inline_DebugKDOutput = \
	"int 0x21"\
	parm nomemory [eax] [edx]\
	modify nomemory exact [];

#elif defined(__GNUC__) || defined(__INTEL_COMPILER)
#define ClockCycles()	({ register _Uint64t __cycles; __asm__ __volatile__( \
						"rdtsc" \
						: "=A" (__cycles)); __cycles; })

#define __inline_InterruptDisable() __asm__ __volatile__ ( \
		"cli\n" \
		"\tmovl %%eax,%%eax" \
		: : :"memory")

#define __inline_InterruptEnable() __asm__ __volatile__ ( \
		"sti" \
		: : :"memory")

/*
 * Make sure gcc doesn't try to be clever and move things around
 * on us. We need to use _exactly_ the address the user gave us,
 * not some alias that contains the same information.
 */
#ifndef __atomic_fool_gcc
struct __gcc_fool { int __fool[100]; };
#define __atomic_fool_gcc(__x) (*(volatile struct __gcc_fool *)__x)
#endif

#define __inline_InterruptUnlock(__spin)	({ \
	extern unsigned __cpu_flags; \
	struct intrspin __attribute__((__unused__)) *__check = (__spin); \
	if(__cpu_flags & (1 << 15)) __asm__ __volatile__("mfence");	\
	__asm__ __volatile__ (	\
		"movl $0,%0\n\t"	\
		"sti"	\
		: : "m" (__atomic_fool_gcc(__spin)) :"memory"); })

#define __inline_InterruptLock(__spin)		({ \
	struct intrspin	__attribute__((__unused__)) *__check = (__spin); \
	register int	__tmp; \
	__asm__ __volatile__ ( \
		"cli\n" \
		"0:\n\t" \
		"movl $1,%0\n\t" \
		"xchgl %1,%0\n\t" \
		"testl %0,%0\n\t" \
		"jz 1f\n\t" \
		".byte 0xf3,0x90\n\t" \
		"jmp 0b\n\t" \
		"1:\n\t" \
		: "=&r" (__tmp) : "m" (__atomic_fool_gcc(__spin)) :"memory"); })

#define __inline_InterruptStatus()		({register _Uint32t __flags; __asm__ __volatile__ ( \
						"pushfl\n\t" \
						"pop %0\n\t" \
						: "=r" (__flags) : : "memory"); __flags & 0x200; })

#define __inline_DebugBreak()	((void)({ __asm__ __volatile__ ("int $0x03": :); }))

#define __inline_DebugKDBreak()	((void)({ __asm__ __volatile__ ("int $0x20": :); }))

#define __inline_DebugKDOutput(__text, __len) ((void)({ \
	__asm__ __volatile__ ( \
		"int $0x21" \
		: :"a" (__text), "d" (__len));	}))

#else
#error Compiler not defined.
#endif

/* __SRCVERSION("neutrino.h $Rev: 175130 $"); */
