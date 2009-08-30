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

#ifndef	__KERCPU_H
#define	__KERCPU_H

#include <kernel/cpu_arm.h>

#define xfer_setjmp(_env)					_setjmp(_env)
#define xfer_longjmp(_env, _ret, _regs)		_longjmp(_env, _ret)

#define REGIP(reg)				((reg)->gpr[ARM_REG_PC])
#define REGSP(reg)				((reg)->gpr[ARM_REG_SP])
#define REGTYPE(reg)			((reg)->gpr[ARM_REG_IP])
#define REGSTATUS(reg)			((reg)->gpr[ARM_REG_R0])

#define SETREGIP(reg,v)			(REGIP(reg) = (uint32_t)(v))
#define SETREGSP(reg,v)			(REGSP(reg) = (uint32_t)(v))
#define SETREGSTATUS(reg,v)		(REGSTATUS(reg) = (uint32_t)(v))
#define SETREGTYPE(reg,v)		(REGTYPE(reg) = (uint32_t)(v))

#define KER_ENTRY_SIZE		4	/* size of kernel entry opcode */
#define KERERR_SKIPAHEAD	4	/* increment IP by this much on error */

#define	COUNT_CYCLES(count, cycles)	*(cycles) += (count)

#define CPU_RD_SYSADDR_OK(thp, p, size)	within_syspage((uintptr_t)(p), (size))

#ifndef	VARIANT_v6
/*
 * We need to define WR_PROBE_OPT to force a check for writable data
 * since kernel mode can write to read-only user pages.
 */
#define	WR_PROBE_OPT	WR_PROBE_INT
#endif

extern void	rd_probe_1(const void *loc);
extern void	wr_probe_1(void *loc);
extern void	rd_probe_num(const void *loc, int num);
extern void	wr_probe_num(void *loc, int num);

#if defined(VARIANT_smp)
	#define	HAVE_INKERNEL_STORAGE	1	// inkernel is defined in kernel.S
	#define	INKERNEL_BITS_SHIFT		8

	#define am_inkernel()		arm_am_inkernel()

	static inline unsigned
	arm_am_inkernel()
	{
		unsigned	tmp;

		asm volatile("mrs	%0, cpsr" : "=r"(tmp));
		return ((tmp & ARM_CPSR_MODE_MASK) == ARM_CPSR_MODE_SVC);
	}

	#define bitset_inkernel(bits)	atomic_set(&inkernel, bits)
	#define bitclr_inkernel(bits)	atomic_clr(&inkernel, bits)
#endif

inline static unsigned
arm_intr_disable(void)
{
	unsigned	__old;

	__asm__ __volatile__(
		"	mrs	%0, cpsr"
		: "=r" (__old)
	);
	__asm__ __volatile__(
		"	msr	cpsr, %0"
		:
		: "r" (__old | ARM_CPSR_I | ARM_CPSR_F)
	);
	return __old;
}

inline static void
arm_intr_restore(unsigned __old)
{
	__asm__ __volatile__(
		"	msr	cpsr, %0;"
		:
		: "r" (__old)
	);
}

extern void	arm_abort_fixup(CPU_REGISTERS *__reg);

/*
 * Coprocessor/FPU support code
 */
extern void	(*fpu_ctx_init)(FPU_REGISTERS *__fpu);
extern void	(*fpu_ctx_save)(FPU_REGISTERS *__fpu);
extern void	(*fpu_ctx_restore)(FPU_REGISTERS *__fpu);
extern void	(*fpu_disable)();
extern void	(*fpu_fpemu_prep)(ARM_FPEMU_CONTEXT *__ctx);

extern int	coproc_attach(int __cpnum, int (*__hdl)(void *, unsigned, CPU_REGISTERS *, unsigned *));

#define	ARM_COPROC_EMULATE	0
#define	ARM_COPROC_HANDLED	1
#define	ARM_COPROC_FAULT	2

extern void	vfp_init();

#endif	// __KERCPU_H

/* __SRCVERSION("kercpu.h $Rev: 153052 $"); */
