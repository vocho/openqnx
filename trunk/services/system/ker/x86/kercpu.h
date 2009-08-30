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

#include <kernel/cpu_x86.h>

int  xfer_setjmp(jmp_buf env);
void xfer_longjmp(jmp_buf env, int val, CPU_REGISTERS *regs);

#define REGIP(reg)			((reg)->eip)
#define REGSP(reg)			((reg)->esp)
#define REGSTATUS(reg)		((reg)->eax)
#define REGTYPE(reg)		((reg)->eax)

#define SETREGIP(reg,v)			(REGIP(reg) = (uint32_t)(v))
#define SETREGSP(reg,v)			(REGSP(reg) = (uint32_t)(v))
#define SETREGSTATUS(reg,v)		(REGSTATUS(reg) = (uint32_t)(v))
#define SETREGTYPE(reg,v)       (REGTYPE(reg) = (uint32_t)(v))

#define KER_ENTRY_SIZE		2	/* size of kernel entry opcode */
#define KERERR_SKIPAHEAD	1	/* increment IP by this much on error */

extern unsigned volatile inkernel;

#define COUNT_CYCLES(count, cycles) *(cycles) += (count);

#define CPU_RD_SYSADDR_OK(thp, p, size)	within_syspage((uintptr_t)(p), (size))

void set_trap(unsigned, void (*)());

#include <_pack1.h>
struct segment_info {
	uint16_t	limit;
	uint32_t	base;
};
#include <_packpop.h>

#if defined(__WATCOMC__)

/* Something to force a read of a memory dword */
int rd_probe_1(const void *);
#pragma aux rd_probe_1 = \
			"mov eax,[eax]" \
			parm [eax] \
			modify exact [eax];

int rd_probe_num(const void *ptr, int num);
#pragma aux rd_probe_num = \
			"rep lodsd" \
			parm [esi] [ecx] \
			modify exact [eax ecx esi];

/* Something to force a write of a memory dword */
int wr_probe_1(void *);
#pragma aux wr_probe_1 = \
			"add dword ptr [eax],0" \
			parm [eax] \
			modify exact [eax];

int wr_probe_num(void *ptr, int num);
#pragma aux wr_probe_num = \
			"mov edi,esi" \
			"rep movsd" \
			parm [esi] [ecx] \
			modify exact [eax ecx esi edi];

unsigned _mypriv();
#pragma aux _mypriv =\
			"mov ax,cs"\
			"and eax,3"\
			modify exact nomemory [ eax ];

void smp_set_INKERNEL_LOCK(void);
#pragma aux smp_set_INKERNEL_LOCK = \
			"or byte ptr inkernel+1,04h" \
			modify exact [eax];

void smp_clr_INKERNEL_LOCK(void);
#pragma aux smp_clr_INKERNEL_LOCK = \
			"and byte ptr inkernel+1,0fbh" \
			modify exact [eax];

void smp_set_INKERNEL_SPECRET(void);
#pragma aux smp_set_INKERNEL_SPECRET = \
			"or byte ptr inkernel+1,02h" \
			modify exact [eax];

void smp_clr_INKERNEL_SPECRET(void);
#pragma aux smp_clr_INKERNEL_SPECRET = \
			"and byte ptr inkernel+1,0fdh" \
			modify exact [eax];

/* Map to mempcy for now - update later for Watcom */
#define __inline_xfer_memcpy(dst, src, n)	memcpy(dst, src, n)


#elif (defined(__GNUC__) || defined(__ICC))

#define rd_probe_1(ptr)	({ __attribute__((unused)) uint32_t dummy = *(const volatile uint32_t *)(ptr); })

#define rd_probe_num(ptr, num)	({ \
			unsigned tmp; \
			__asm__ __volatile__( \
			"rep; lodsl" \
			: "=S" (tmp), "=c" (tmp) : "S" ((const void *)(ptr)), "c" (num) : "eax"); })

#define wr_probe_1(ptr)	({ asm volatile( "add $0,(%0)" : : "r" (ptr)); })

#define wr_probe_num(ptr, num)	({ \
			unsigned tmp; \
			__asm__ __volatile__( \
			"rep; movsl" \
			: "=D" (tmp), "=S" (tmp), "=c" (tmp) : "D" ((ptr)), "S" ((ptr)), "c" (num) :  "eax"); })

#define _mypriv()	({ register unsigned priv; __asm__( \
					"movw %%cs,%w0\n\tandl $3,%0": "=r" (priv)); priv; })

#define __inline_xfer_memcpy(dst, src, n) ({ 			\
	int	d0, d1, d2;										\
	__asm__ __volatile__ (								\
		"movl	%%ecx,%%eax\n"							\
		"shr	$2,%%ecx\n"								\
		"repne;	movsl\n"								\
		"movl	%%eax,%%ecx\n"							\
		"andl	$3,%%ecx\n"								\
		"jz		1f\n"									\
		"repne;	movsb\n"								\
		"1: \n"											\
		:"=&c" (d0), "=&D" (d1), "=&S" (d2)				\
		:"0" (n), "1" ((long) dst), "2" ((long) src)	\
		:"eax", "memory");	})

#else

#error Compiler not defined.

#endif

#define am_inkernel()			(_mypriv() == 0)

#if defined(VARIANT_smp)
	#if defined(__WATCOMC__)
		#define bitset_inkernel(bit)	smp_set_##bit()
		#define bitclr_inkernel(bit)	smp_clr_##bit()
	#elif defined(__GNUC__)
		#define bitset_inkernel(bits)	({ __asm__ __volatile__("lock; orl %0,inkernel": : "i" (bits) :"memory"); })
		#define bitclr_inkernel(bits)	({ __asm__ __volatile__("lock; andl %0,inkernel": : "i" (~(bits)) :"memory"); })
	#else
		#error Compiler not defined
	#endif
	#define atomic_set(m, v)		smp_locked_or(m, v)
	#define atomic_clr(m, v)		smp_locked_and(m, ~(v))
	#define HAVE_INKERNEL_STORAGE	1
	#define INKERNEL_BITS_SHIFT	8
#else
	#define atomic_set(m, v)	(*(m) |= (v))
	#define atomic_clr(m, v)	(*(m) &= ~(v))
#endif

//
// This is never written into eflags, it's just in the copy
// of eflags that we save in a thread's register context.
//
#define	SYSENTER_EFLAGS_BIT		_ONEBIT32L( 31 )

#define GOT_HERE(c,l)	(*(volatile char *)(realmode_addr + (0xb8000+(160*(l))) + RUNCPU*2) = (c))

/* __SRCVERSION("kercpu.h $Rev: 201493 $"); */
