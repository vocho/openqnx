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

#ifndef	__KERCPU_H_
#define	__KERCPU_H_

#include <kernel/cpu_sh.h>

#define REGIP(reg)			((reg)->pc)
#define REGSP(reg)			((reg)->gr[15])
#define REGTYPE(reg)		((*(uint16_t*)((reg)->pc-2)) & 0xff)
#define REGSTATUS(reg)		((reg)->gr[0])

#define SETREGIP(reg,v)		(REGIP(reg) = (uint32_t)(v))
#define SETREGSP(reg,v)		(REGSP(reg) = (uint32_t)(v))
#define SETREGSTATUS(reg,v)	(REGSTATUS(reg) = (uint32_t)(v))

/*
	SH can't restuff the syscall num, since it's in the code seg. itself
	So we make SETREGTYPE() do nothing for now, since there isn't much
	we can do.
*/
#define SETREGTYPE(reg,v)

#define KER_ENTRY_SIZE		2	/* size of kernel entry opcode */

#define KERERR_SKIPAHEAD	6	/* increment IP by this much on error */

/* These constants are defined by the bad_func_handler_table in ker/sh/kernel.s.
 * NOTE that if these constants change, you must also change public/sh/neutrino.h
 * __inline_DebugBreak and __inline_DebugKDOutput
 */
#define SH_KER_TRAP_CMPXCHG		0xff
#define SH_KER_TRAP_PDBREAK		0xfe
#define SH_KER_TRAP_KDOUTPUT	0xfd
#define SH_KER_TRAP_BOUNDARY	0xfd

#define SH_KER_TRAP_CODE(x)		(0xc300 | x)

/*
 * Convert physical address to pointer value and vis-versa when in a
 * physical memory model.
 */
#define PHYS_TO_PTR(phys)	((void *)SH_PHYS_TO_P1(phys))
#define PTR_TO_PHYS(ptr)	((uintptr_t)SH_P1_TO_PHYS(ptr))

#define rd_probe_1(ptr)	({ __attribute__((unused)) uint32_t dummy = *(const volatile uint32_t *)(ptr); })

static inline void
rd_probe_num(const void *addr, int num) {
	int 	tmp;

	asm volatile(	"1:;"
					"mov.l	@%1,%0;"
					"add	#4,%1;"
					"dt		%2;"
					"bf		1b;"
					: "=&r" (tmp), "=&r" (addr), "=&r" (num)
					: "1" (addr), "2" (num) 
					: "cc"
					);
}

static inline void
wr_probe_1(const void *addr) {
	int 	tmp;

	asm volatile( 	"mov.l	@%1,%0;"
					"mov.l	%0,@%1;"
					: "=&r" (tmp)
					: "r" (addr) );
}

static inline void
wr_probe_num(const void *addr, int num) {
	int 	tmp;

	asm volatile(	"1:;"
					"mov.l	@%1,%0;"
					"mov.l	%0,@%1;"
					"add	#4,%1;"
					"dt		%2;"
					"bf		1b;"
					: "=&r" (tmp), "=&r" (addr), "=&r" (num)
					: "1" (addr), "2" (num)
					: "cc");
}

#if defined(VARIANT_smp)
/*
 * For SMP:
 *
 * - inkernel is defined in kernel.s:
 *   . byte 0 holds interrupt nesting counter and inkernel flags
 *   . byte 1 is unused
 *   . byte 2 is unused
 *   . byte 3 holds the cpunum value indicating the cpu currently in kernel
 *
 *   r7_bank holds a pointer to the inkernel variable
 *
 * Note that we need INKERNEL_BITS_SHIFT to be 4 so that the INKERNEL_* bits
 * can be represented as an 8-bit constant for use an immediate operand.
 *
 * - inkernel manipulation must be atomic
 */
#define HAVE_INKERNEL_STORAGE	1
#define INKERNEL_BITS_SHIFT	4

static inline unsigned
_get_inkernel(void)
{
	unsigned	inkp;
	unsigned	ret;

	__asm__ __volatile__(
	"stc	r7_bank, %0;"
	"mov.l	@%0, %1;"
	: "=&r" (inkp), "=&r" (ret)
	);
	return ret;
}

static inline void
_set_inkernel(unsigned x)
{
	unsigned	inkp;

	__asm__ __volatile__(
	"stc	r7_bank, %0;"
	"mov.l	%1, @%0;"
	: "=&r" (inkp)
	: "r" (x)
	);
}

static inline void
_bitset_inkernel(unsigned bits)
{
	__asm__ __volatile__(
	"	stc		r7_bank, r1;"
	"0:	.word	0x0163;"		/* movli.l @r1, r0       */
	"	or		%0, r0;"
	"	.word	0x0173;"		/* movco.l r0, @r1       */
	"	bf		0b;"			/* retry if movco failed */
	:
	: "r" (bits)
	: "r0", "r1", "cc", "memory"
	);
}

static inline void
_bitclr_inkernel(unsigned bits)
{
	__asm__ __volatile__(
	"	stc		r7_bank, r1;"
	"0:	.word	0x0163;"		/* movli.l @r1, r0       */
	"	and		%0, r0;"
	"	.word	0x0173;"		/* movco.l r0, @r1       */
	"	bf		0b;"			/* retry if movco failed */
	:
	: "r" (~bits)
	: "r0", "r1", "cc", "memory"
	);
}

#define	bitset_inkernel(bits)	_bitset_inkernel(bits)
#define	bitclr_inkernel(bits)	_bitclr_inkernel(bits)

/*
 * We cannot use the SR MD bit to tell if we're in the kernel because
 * proc thread run in privileged mode.
 *
 * Instead, we have to determine whether we're on the kernel stack.
 */
#define am_inkernel()									\
({	uintptr_t sp;										\
	__asm__ ("mov r15, %0" : "=&r" (sp));				\
	((sp >= ker_stack_bot) && (sp < ker_stack_top));	\
})

/*
 * We need special code to manipulate the intr_slock lock.
 *
 * The SH interrupt callouts set SR.IMASK directly, which means that a
 * higher priority interrupt can occur during the id callout processing.
 * This will deadlock because intr_slock is already held at this point.
 *
 * To avoid this problem, we implement intr_slock as a recursive spin lock:
 * bits 31-24: contain cpuid
 * bits 23-00: contain lock count
 *
 * Refer to kernel.s for an explanation of why we don't go straight to
 * the movli.l instruction, but instead test the water first.
 */

#define	__slock_lock(l)													\
({																		\
	unsigned	cpuid = RUNCPU;											\
	unsigned	tmp;													\
																		\
	__asm__ __volatile__(												\
	"	mov		%1, r1;"												\
	"2:	mov.l	@r1, r0;"	/* check current value */					\
	"	tst		r0, r0;"												\
	"	bt		0f;"		/* test for zero - safe to proceed */		\
	"	mov		r0, r0;"												\
	"	shlr16	%0;"													\
	"	shlr8	%0;"													\
	"	cmp/eq	%0, %2;"	/* we don't own it -- spin back */			\
	"	bf		2b;"													\
	"0:	.word	0x0163;"	/* movli.l @r1, r0 */						\
	"	tst		r0, r0;"												\
	"	bt		1f;"													\
	"	mov		r0, %0;"	/* extract cpuid field from l->value */		\
	"	shlr16	%0;"													\
	"	shlr8	%0;"													\
	"	cmp/eq	%0, %2;"												\
	"	bf		2b;"													\
	"1:	add		#1, r0;"	/* increment count */						\
	"	mov		%2, %0;"												\
	"	shll16	%0;"													\
	"	shll8	%0;"													\
	"	or		%0, r0;"	/* make sure we set cpuid */				\
	"	.word	0x0173;"	/* movco.l r0, @r1 */						\
	"	bf		2b;"													\
	: "=&r" (tmp)														\
	: "r" (&(l)->value), "r" (cpuid)									\
	: "r0", "r1", "cc", "memory"										\
	);																	\
})

#define __slock_unlock(l)												\
({																		\
	unsigned	tmp;													\
																		\
	__asm__ __volatile__(												\
	"	mov.l	@%1, r0;"												\
	"	add		#-1, r0;"	/* decrement count */						\
	"	mov		r0, %0;"												\
	"	shll8	%0;"		/* count << 8 to remove cpuid field */		\
	"	tst		%0, %0;"												\
	"	bf		0f;"													\
	"	mov		#0, r0;"	/* clear lock */							\
	"0:	mov.l	r0, @%1;"												\
	: "=&r" (tmp)														\
	: "r" (&(l)->value)													\
	: "r0", "cc", "memory"												\
	);																	\
})

#define __slock_intr_lock(l)											\
({																		\
	__inline_InterruptDisable();										\
	__slock_lock(l);													\
})

#define __slock_intr_unlock(l)											\
({																		\
	__slock_unlock(l);													\
	__inline_InterruptEnable();											\
})

#define	CPU_SLOCK_INTR_LOCK(l)		__slock_intr_lock(l)
#define	CPU_SLOCK_INTR_UNLOCK(l)	__slock_intr_unlock(l)
#define	CPU_SLOCK_LOCK(l)			__slock_lock(l)
#define	CPU_SLOCK_UNLOCK(l)			__slock_unlock(l)

#else

/*
 * Non-SMP uses bank1 r7 register to hold inkernel value
 
 */
inline static unsigned
_get_inkernel(void) {
	unsigned ret;

	asm volatile (
	"stc r7_bank,%0"
	: "=r" (ret)
	: /* No inputs */
	);
	return(ret);
}

inline static void
_set_inkernel(unsigned x) {
	asm volatile (
	"ldc %0,r7_bank"
	: /* no output*/
	: "r" (x)
	);
}
#endif	/* VARIANT_smp */

#define DISABLE_FPU() set_sr(get_sr()|SH_SR_FD)

#define get_inkernel()		_get_inkernel()
#define set_inkernel(val)	_set_inkernel(val)

#define xfer_setjmp(_env)					_setjmp(_env)
#define xfer_longjmp(_env, _ret, _regs)		_longjmp(_env, _ret)

extern void	copy_code(uint32_t *exc, void *code_start, void *code_end);
extern void	copy_vm_code(void);
extern void	set_trap(unsigned, void (*)(), unsigned);
extern void	install_traps(void);
extern void restore_fpu_registers(SH_FPU_REGISTERS*);

struct thread_entry;
extern void sh_setup_sigstub(struct thread_entry* thp, uintptr_t new_sp, uintptr_t siginfo);
#define CPU_SETUP_SIGSTUB sh_setup_sigstub


#if defined(VARIANT_smp)
/*
 * For SMP use movli/movco for atomic operation
 */
static inline unsigned __attribute__((__unused__))
_smp_cmpxchg(volatile unsigned *__dst, unsigned __cmp, unsigned __src) {
	unsigned	__res;

	__asm__ __volatile__(
		"	mov		%1, r1	\n"
		"0:	.word	0x0163	\n"	/* movli.l	@r1, r0 */
		"	cmp/eq	r0, %2	\n"
		"	mov		r0, %0	\n"
		"	bf		1f		\n"
		"	mov		%3, r0	\n"
		"	.word	0x0173	\n"	/* movco.l	r0, @r1 */
		"	bf		0b	\n"
		"1:"
		: "=&r" (__res)
		: "r" (__dst), "r" (__cmp), "r" (__src)
		: "r0", "r1", "cc", "memory"
	);
	return __res;
}

static inline unsigned __attribute__((__unused__))
_smp_xchg(volatile unsigned *__dst, unsigned __new) {
	unsigned	__res;

	__asm__ __volatile__(
		"	mov		%1, r1	\n"
		"0:	.word	0x0163	\n"	/* movli.l	@r1, r0 */
		"	mov		r0, %0	\n"
		"	mov		%2, r0	\n"
		"	.word	0x0173	\n"	/* movco.l	r0, @r1 */
		"	bf		0b		\n"
		: "=&r" (__res)
		: "r" (__dst), "r" (__new)
		: "r0", "r1", "cc", "memory"
	);
	return __res;
}
#else
/*
 * For non-SMP disable interrupts for atomic operation
 */
static inline unsigned __attribute__((__unused__))
_smp_cmpxchg(volatile unsigned *__dst, unsigned __cmp, unsigned __src) {
	unsigned	__res;
	unsigned temp;
	temp = get_sr();
	set_sr(temp | SH_SR_IMASK);
	__asm__ __volatile__(
		"	mov.l	@%1,%0;"
		"	cmp/eq	%0,%2;"
		"	bf		1f;"
		"	mov.l	%3,@%1;"
		"1:"
		: "=&r" (__res)
		: "r" (__dst), "r" (__cmp), "r" (__src)
		: "cc", "memory"
	);
	set_sr(temp);
	return(__res);
}


static inline unsigned __attribute__((__unused__))
_smp_xchg(volatile unsigned *__dst, unsigned __new) {
	unsigned	__res;
	unsigned temp;
	temp = get_sr();
	set_sr(temp | SH_SR_IMASK);
	__asm__ __volatile__(
		"	mov.l	@%1,%0;"
		"	mov.l	%2,@%1;"
		: "=&r" (__res)
		: "r" (__dst), "r" (__new)
		: "memory"
	);
	set_sr(temp);

	return(__res);
}
#endif

#define __inline_InterruptEnable() set_sr((get_sr() & ~SH_SR_IMASK) | *__shadow_imask);

static inline void __attribute__((__unused__))
(__inline_InterruptDisable)(void) {
	set_sr(get_sr() | SH_SR_IMASK);
}


#define CLOCKCYCLES_INCR_BIT 32

static inline _Uint64t __attribute__((__unused__))
ClockCycles(void) {
	unsigned __lo, __tmp;
	unsigned __addr_tcor1, __addr_tcnt1;

	__addr_tcor1 = sh_mmr_tmu_base_address + SH_MMR_TMU_TCOR1_OFFSET;
	__addr_tcnt1 = sh_mmr_tmu_base_address + SH_MMR_TMU_TCNT1_OFFSET;

	__asm__ __volatile__(
		"mov.l	@%2,%0;"
		"mov.l	@%3,%1;"
		"sub	%1,%0;"
		: "=&r" (__lo), "=&r" (__tmp)
		: "r" (__addr_tcor1), "r" (__addr_tcnt1)
	);
	return (_Uint64t) __lo << CLOCKCYCLES_INCR_BIT;
}
#endif

/* __SRCVERSION("kercpu.h $Rev: 199716 $"); */
