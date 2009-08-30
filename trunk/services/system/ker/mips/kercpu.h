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

#include <kernel/cpu_mips.h>

#define xfer_setjmp(_env)					_setjmp(_env)
#define xfer_longjmp(_env, _ret, _regs)		_longjmp(_env, _ret)

/*
	This makes each argument in a kernel call structure take up 64 bits so
	that it aligns with the register save area layout. 
*/
#define XFILLER( line )		uint32_t	filler##line
#define FILLER( line )		XFILLER( line )

#if defined(__BIGENDIAN__)
	#define KARGSLOT( arg )		FILLER(__LINE__); arg
#elif defined(__LITTLEENDIAN__)
	#define KARGSLOT( arg )		arg; FILLER(__LINE__)
#else
	#error ENDIAN Not defined for system
#endif

#define REGTYPE(rs)		((rs)->regs[MIPS_CREG(MIPS_REG_V0)])
#define REGSTATUS(rs)	((rs)->regs[MIPS_CREG(MIPS_REG_V0)])
#define REGIP(rs)		((rs)->regs[MIPS_CREG(MIPS_REG_EPC)])
#define REGSP(rs)		((rs)->regs[MIPS_CREG(MIPS_REG_SP)])

#define SETREGTYPE(rs,v)	MIPS_REG_SETx64(rs, MIPS_REG_V0, v)
#define SETREGSTATUS(rs,v)	MIPS_REG_SETx64(rs, MIPS_REG_V0, v)
#define SETREGIP(rs,v)		MIPS_REG_SETx64(rs, MIPS_REG_EPC, v)
#define SETREGSP(rs,v)		MIPS_REG_SETx64(rs, MIPS_REG_SP, v)

#define SETKIP_FUNC(thp,v)	cpu_invoke_func(thp, (uintptr_t)v)

#define KER_ENTRY_SIZE		4	/* size of kernel entry opcode */
#define KERERR_SKIPAHEAD	8	/* increment IP by this much on error */

/*
 * Convert physical address to pointer value and vis-versa when in a
 * physical memory model.
 */
#define PHYS_TO_PTR(phys)		((void *)MIPS_PHYS_TO_KSEG0(phys))
#define PTR_TO_PHYS(ptr)		((uintptr_t)MIPS_KSEG0_TO_PHYS(ptr))


#define rd_probe_1(ptr)	({ __attribute__((unused)) uint32_t dummy = *(const volatile uint32_t *)(ptr); })

inline static void
rd_probe_num(const void *loc, int num) {
	ulong_t tmp;

	asm volatile (
		".set noreorder ;"
		"1:	lw %0, 0(%1) ;"
		"	addiu %2,%2,-1 ;"
		"   bne %2,$0,1b ;"
		"	addiu %1,%1,4 ;"
		".set reorder ;"
		: "=&r" (tmp),"=&b" (loc),"=&r" (num)
		: "1" (loc), "2" (num)
		: "memory"
		);
}

inline static void
wr_probe_1(void *loc) {
	ulong_t tmp;

	asm volatile (
		"	lw %0, 0(%1) ;"
		"	sw %0, 0(%1) ;"
		: "=&r" (tmp)
		: "b" (loc)
		: "memory"
		);
}

inline static void
wr_probe_num(void *loc, int num) {
	ulong_t tmp;

	asm volatile (
		".set noreorder ;"
		"1:	lw %0, 0(%1) ;"
		"	sw %0, 0(%1) ;"
		"	addiu %2,%2,-1 ;"
		"   bne %2,$0,1b ;"
		"	addiu %1,%1,4 ;"
		".set reorder ;"
		: "=&r" (tmp),"=&b" (loc), "=&r" (num)
		: "1" (loc), "2" (num)
		: "memory"
		);
}

#define HAVE_ACTIVES_STORAGE	1
#define HAVE_KERSTACK_STORAGE	1
#define HAVE_INKERNEL_STORAGE	1

#define INKERNEL_BITS_SHIFT	8

#if defined(VARIANT_smp)

	#define am_inkernel() \
		({	uintptr_t sp; \
			__asm__ ("move %0, $sp" : "=r" (sp)); \
			((sp >= ker_stack_bot) && (sp < ker_stack_top)); \
		})
	#define bitset_inkernel(bits)	atomic_set(&inkernel, bits)
	#define bitclr_inkernel(bits)	atomic_clr(&inkernel, bits)

#endif /* SMP */

#if !defined(MIPS_CAUSE2SIGMAP_CONST)
#define MIPS_CAUSE2SIGMAP_CONST const
#endif
extern MIPS_CAUSE2SIGMAP_CONST unsigned long __mips_cause2sig_map[];

extern uintptr_t	next_instruction(CPU_REGISTERS *ctx);

/* __SRCVERSION("kercpu.h $Rev: 164523 $"); */
